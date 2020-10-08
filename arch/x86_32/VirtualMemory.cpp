#include <libsystem/io/Stream.h>
#include <libsystem/thread/Atomic.h>

#include "arch/VirtualMemory.h"
#include "arch/x86_32/Paging.h"

#include "kernel/memory/Memory.h"
#include "kernel/memory/Physical.h"
#include "kernel/system/System.h"

PageDirectory _kernel_page_directory __aligned(ARCH_PAGE_SIZE) = {};
PageTable _kernel_page_tables[256] __aligned(ARCH_PAGE_SIZE) = {};

void arch_virtual_initialize()
{
    // Setup the kernel pagedirectory.
    for (size_t i = 0; i < 256; i++)
    {
        PageDirectoryEntry *entry = &_kernel_page_directory.entries[i];
        entry->User = 0;
        entry->Write = 1;
        entry->Present = 1;
        entry->PageFrameNumber = (size_t)&_kernel_page_tables[i] / ARCH_PAGE_SIZE;
    }
}

void arch_virtual_memory_enable()
{
    paging_enable();
}

void *arch_kernel_address_space()
{
    return &_kernel_page_directory;
}

bool arch_virtual_present(void *address_space, uintptr_t virtual_address)
{
    ASSERT_ATOMIC;

    auto page_directory = reinterpret_cast<PageDirectory *>(address_space);

    int page_directory_index = PAGE_DIRECTORY_INDEX(virtual_address);
    PageDirectoryEntry &page_directory_entry = page_directory->entries[page_directory_index];

    if (!page_directory_entry.Present)
    {
        return false;
    }

    PageTable &page_table = *reinterpret_cast<PageTable *>(page_directory_entry.PageFrameNumber * ARCH_PAGE_SIZE);

    int page_table_index = PAGE_TABLE_INDEX(virtual_address);
    PageTableEntry &page_table_entry = page_table.entries[page_table_index];

    return page_table_entry.Present;
}

uintptr_t arch_virtual_to_physical(void *address_space, uintptr_t virtual_address)
{
    ASSERT_ATOMIC;

    auto page_directory = reinterpret_cast<PageDirectory *>(address_space);

    int page_directory_index = PAGE_DIRECTORY_INDEX(virtual_address);
    PageDirectoryEntry &page_directory_entry = page_directory->entries[page_directory_index];

    if (!page_directory_entry.Present)
    {
        return 0;
    }

    PageTable &page_table = *reinterpret_cast<PageTable *>(page_directory_entry.PageFrameNumber * ARCH_PAGE_SIZE);

    int page_table_index = PAGE_TABLE_INDEX(virtual_address);
    PageTableEntry &page_table_entry = page_table.entries[page_table_index];

    if (!page_table_entry.Present)
    {
        return 0;
    }

    return (page_table_entry.PageFrameNumber * ARCH_PAGE_SIZE) + (virtual_address & 0xfff);
}

Result arch_virtual_map(void *address_space, MemoryRange physical_range, uintptr_t virtual_address, MemoryFlags flags)
{
    ASSERT_ATOMIC;

    auto page_directory = reinterpret_cast<PageDirectory *>(address_space);

    for (size_t i = 0; i < physical_range.size() / ARCH_PAGE_SIZE; i++)
    {
        size_t offset = i * ARCH_PAGE_SIZE;

        int page_directory_index = PAGE_DIRECTORY_INDEX(virtual_address + offset);
        PageDirectoryEntry &page_directory_entry = page_directory->entries[page_directory_index];
        PageTable *page_table = reinterpret_cast<PageTable *>(page_directory_entry.PageFrameNumber * ARCH_PAGE_SIZE);

        if (!page_directory_entry.Present)
        {
            Result alloc_result = memory_alloc_identity(page_directory, MEMORY_CLEAR, (uintptr_t *)&page_table);

            if (alloc_result != SUCCESS)
            {
                return alloc_result;
            }

            page_directory_entry.Present = 1;
            page_directory_entry.Write = 1;
            page_directory_entry.User = 1;
            page_directory_entry.PageFrameNumber = (uint32_t)(page_table) >> 12;
        }

        int page_table_index = PAGE_TABLE_INDEX(virtual_address + offset);
        PageTableEntry &page_table_entry = page_table->entries[page_table_index];

        page_table_entry.Present = 1;
        page_table_entry.Write = 1;
        page_table_entry.User = flags & MEMORY_USER;
        page_table_entry.PageFrameNumber = (physical_range.base() + offset) >> 12;
    }

    paging_invalidate_tlb();

    return SUCCESS;
}

MemoryRange arch_virtual_alloc(void *address_space, MemoryRange physical_range, MemoryFlags flags)
{
    ASSERT_ATOMIC;

    bool is_user_memory = flags & MEMORY_USER;

    uintptr_t virtual_address = 0;
    size_t current_size = 0;

    // we skip the first page to make null deref trigger a page fault
    for (size_t i = (is_user_memory ? 256 : 1) * 1024; i < (is_user_memory ? 1024 : 256) * 1024; i++)
    {
        uintptr_t current_address = i * ARCH_PAGE_SIZE;

        if (!arch_virtual_present(address_space, current_address))
        {
            if (current_size == 0)
            {
                virtual_address = current_address;
            }

            current_size += ARCH_PAGE_SIZE;

            if (current_size == physical_range.size())
            {
                arch_virtual_map(address_space, physical_range, virtual_address, flags);

                return (MemoryRange){virtual_address, current_size};
            }
        }
        else
        {
            current_size = 0;
        }
    }

    system_panic("Out of virtual memory!");
}

void arch_virtual_free(void *address_space, MemoryRange virtual_range)
{
    ASSERT_ATOMIC;

    auto page_directory = reinterpret_cast<PageDirectory *>(address_space);

    for (size_t i = 0; i < virtual_range.size() / ARCH_PAGE_SIZE; i++)
    {
        size_t offset = i * ARCH_PAGE_SIZE;

        size_t page_directory_index = PAGE_DIRECTORY_INDEX(virtual_range.base() + offset);
        PageDirectoryEntry *page_directory_entry = &page_directory->entries[page_directory_index];

        if (!page_directory_entry->Present)
        {
            continue;
        }

        PageTable *page_table = (PageTable *)(page_directory_entry->PageFrameNumber * ARCH_PAGE_SIZE);

        size_t page_table_index = PAGE_TABLE_INDEX(virtual_range.base() + offset);
        PageTableEntry *page_table_entry = &page_table->entries[page_table_index];

        if (page_table_entry->Present)
        {
            page_table_entry->as_uint = 0;
        }
    }
}

void *arch_address_space_create()
{
    AtomicHolder holder;

    PageDirectory *page_directory = nullptr;

    if (memory_alloc(arch_kernel_address_space(), sizeof(PageDirectory), MEMORY_CLEAR, (uintptr_t *)&page_directory) != SUCCESS)
    {
        logger_error("Page directory allocation failed!");

        return nullptr;
    }

    memset(page_directory, 0, sizeof(PageDirectory));

    // Copy first gigs of virtual memory (kernel space);
    for (uint i = 0; i < 256; i++)
    {
        PageDirectoryEntry *page_directory_entry = &page_directory->entries[i];

        page_directory_entry->User = 0;
        page_directory_entry->Write = 1;
        page_directory_entry->Present = 1;
        page_directory_entry->PageFrameNumber = (uint)&_kernel_page_tables[i] / ARCH_PAGE_SIZE;
    }

    return page_directory;
}

void arch_address_space_destroy(void *address_space)
{
    AtomicHolder holder;

    assert(address_space != arch_kernel_address_space());

    auto page_directory = reinterpret_cast<PageDirectory *>(address_space);

    for (size_t i = 256; i < 1024; i++)
    {
        PageDirectoryEntry *page_directory_entry = &page_directory->entries[i];

        if (page_directory_entry->Present)
        {
            PageTable *page_table = (PageTable *)(page_directory_entry->PageFrameNumber * ARCH_PAGE_SIZE);

            for (size_t i = 0; i < 1024; i++)
            {
                PageTableEntry *page_table_entry = &page_table->entries[i];

                if (page_table_entry->Present)
                {
                    uintptr_t physical_address = page_table_entry->PageFrameNumber * ARCH_PAGE_SIZE;

                    MemoryRange physical_range{physical_address, ARCH_PAGE_SIZE};

                    physical_free(physical_range);
                }
            }

            memory_free(arch_kernel_address_space(), (MemoryRange){(uintptr_t)page_table, sizeof(PageTable)});
        }
    }

    memory_free(arch_kernel_address_space(), (MemoryRange){(uintptr_t)page_directory, sizeof(PageDirectory)});
}

void arch_address_space_switch(void *address_space)
{
    AtomicHolder holder;
    paging_load_directory(arch_virtual_to_physical(arch_kernel_address_space(), (uintptr_t)address_space));
}

#define MEMORY_DUMP_REGION_START(__pdir, __addr)                     \
    {                                                                \
        memory_used = true;                                          \
        memory_empty = false;                                        \
        current_physical = arch_virtual_to_physical(__pdir, __addr); \
        printf("\n\t %8x [%08x:", (__addr), current_physical);       \
    }

#define MEMORY_DUMP_REGION_END(__pdir, __addr)                                    \
    {                                                                             \
        memory_used = false;                                                      \
        printf("%08x] %08x", arch_virtual_to_physical(__pdir, __addr), (__addr)); \
    }

void arch_address_space_dump(void *address_space, bool user)
{
    auto page_directory = reinterpret_cast<PageDirectory *>(address_space);

    bool memory_used = false;
    bool memory_empty = true;
    uint current_physical = 0;

    for (int i = user ? 256 : 0; i < 1024; i++)
    {
        PageDirectoryEntry *pde = &page_directory->entries[i];
        if (pde->Present)
        {
            PageTable *ptable = (PageTable *)(pde->PageFrameNumber * ARCH_PAGE_SIZE);

            for (int j = 0; j < 1024; j++)
            {
                PageTableEntry *p = &ptable->entries[j];

                if (p->Present && !memory_used)
                {
                    MEMORY_DUMP_REGION_START(address_space, (i * 1024 + j) * ARCH_PAGE_SIZE);
                }
                else if (!p->Present && memory_used)
                {
                    MEMORY_DUMP_REGION_END(address_space, (i * 1024 + j - 1) * ARCH_PAGE_SIZE);
                }
                else if (p->Present)
                {
                    uint new_physical = arch_virtual_to_physical(address_space, (i * 1024 + j) * ARCH_PAGE_SIZE);

                    if (!(current_physical + ARCH_PAGE_SIZE == new_physical))
                    {
                        printf("%08x | ", current_physical);
                        printf("%08x:", new_physical);
                    }

                    current_physical = new_physical;
                }
            }
        }
        else if (memory_used)
        {
            MEMORY_DUMP_REGION_END(address_space, (i * 1024 - 1) * ARCH_PAGE_SIZE);
        }
    }

    if (memory_empty)
    {
        printf("[empty]");
    }
}
