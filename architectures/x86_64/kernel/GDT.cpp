#include "architectures/x86_64/kernel/GDT.h"

static TSS64 tss = {
    .reserved = 0,
    .rsp0 = 0,
    .rsp1 = 0,
    .rsp2 = 0,
    .reserved0 = 0,
    .ist1 = 0,
    .ist2 = 0,
    .ist3 = 0,
    .ist4 = 0,
    .ist5 = 0,
    .ist6 = 0,
    .ist7 = 0,
    .reserved1 = 0,
    .reserved2 = 0,
    .reserved3 = 0,
    .iopb_offset = 0,
};

static GDT64 gdt = {};

static GDTDescriptor64 gdt_descriptor = {
    .size = sizeof(GDT64),
    .offset = (uint64_t)&gdt,
};

static void gdt_set_descriptor(GDTEntry64 *gdt_descriptors,
                               uint8_t flags, uint8_t gran)
{
    gdt_descriptors->flags = flags;
    gdt_descriptors->granularity = gran;
    gdt_descriptors->limit0_15 = 0xFFFF;
}
void gdt_initialize()
{
    gdt.entries[0] = {0, 0, 0, 0}; // null descriptor
    gdt_set_descriptor(&gdt.entries[1], GDT_PRESENT | GDT_SEGMENT | GDT_EXECUTABLE, 0x2 );
    gdt_set_descriptor(&gdt.entries[2], GDT_PRESENT | GDT_SEGMENT | GDT_READWRITE, 0 );
    gdt_set_descriptor(&gdt.entries[3], GDT_PRESENT | GDT_SEGMENT | GDT_EXECUTABLE | GDT_USER, 0x2 );
    gdt_set_descriptor(&gdt.entries[4], GDT_PRESENT | GDT_SEGMENT | GDT_READWRITE | GDT_USER, 0 );

    gdt_flush((uint64_t)&gdt_descriptor);
}

void set_kernel_stack(uint64_t stack)
{
    tss.rsp0 = stack;
}
