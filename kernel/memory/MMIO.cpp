#include <libsystem/Logger.h>
#include <libsystem/thread/Atomic.h>

#include "arch/VirtualMemory.h"

#include "kernel/memory/MMIO.h"
#include "kernel/memory/Physical.h"

MMIORange::MMIORange()
{
}

MMIORange::MMIORange(size_t size)
{
    size = PAGE_ALIGN_UP(size);

    AtomicHolder holder;

    _own_physical_range = true;
    _physical_range = {physical_alloc(size)};
    _virtual_range = {arch_virtual_alloc(arch_kernel_address_space(), _physical_range, MEMORY_NONE)};
}

MMIORange::MMIORange(MemoryRange range)
{
    AtomicHolder holder;

    _physical_range = {range};
    _virtual_range = {arch_virtual_alloc(arch_kernel_address_space(), _physical_range, MEMORY_NONE)};

    logger_info("Created MMIO region %08x-%08x mapped to %08x-%08x",
                range.base(), range.end(), _virtual_range.base(), _virtual_range.end());
}

MMIORange::~MMIORange()
{
    if (empty())
        return;

    AtomicHolder holder;

    arch_virtual_free(arch_kernel_address_space(), _virtual_range);

    if (!_own_physical_range)
        return;

    physical_free(_physical_range);
}
