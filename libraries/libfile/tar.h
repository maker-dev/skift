#pragma once

#include <libsystem/Common.h>

struct TARBlock
{
    char name[100];
    char typeflag;
    char linkname[100];
    u32 size;
    char *data;
} ;

bool tar_read(void *tarfile, TARBlock *block, uint index);
