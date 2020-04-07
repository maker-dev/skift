#pragma once

#include "kernel/memory/MemoryRange.h"

#define MULTIBOOT_BOOTLOADER_NAME_SIZE 64
#define MULTIBOOT_COMMAND_LINE_SIZE 256
#define MULTIBOOT_MEMORY_MAP_SIZE 64
#define MULTIBOOT_MODULES_SIZE 16

typedef enum
{
    MEMORY_MAP_ENTRY_AVAILABLE,
    MEMORY_MAP_ENTRY_RESERVED,
    MEMORY_MAP_ENTRY_ACPI_RECLAIMABLE,
    MEMORY_MAP_ENTRY_NVS,
    MEMORY_MAP_ENTRY_BADRAM,
} MemoryMapEntryType;

typedef struct
{
    MemoryRange range;
    MemoryMapEntryType type;
} MemoryMapEntry;

typedef struct
{
    MemoryRange range;
    char command_line[MULTIBOOT_COMMAND_LINE_SIZE];
} Module;

typedef struct
{
    char bootloader[MULTIBOOT_BOOTLOADER_NAME_SIZE];
    char command_line[MULTIBOOT_COMMAND_LINE_SIZE];

    size_t memory_usable;

    MemoryMapEntry memory_map[MULTIBOOT_MEMORY_MAP_SIZE];
    size_t memory_map_size;

    Module modules[MULTIBOOT_MODULES_SIZE];
    size_t modules_size;
} Multiboot;

void multiboot_assert(uint32_t magic);

int multiboot_version(uint32_t magic);

Multiboot *multiboot_initialize(void *header, uint32_t magic);

bool is_multiboot1(uint32_t magic);

bool is_multiboot2(uint32_t magic);

void multiboot1_parse_header(Multiboot *multiboot, void *header_ptr);

void multiboot2_parse_header(Multiboot *multiboot, void *header_ptr);