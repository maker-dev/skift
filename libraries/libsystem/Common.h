#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <libsystem/Macros.h>
#include <libsystem/core/Allocator.h>
#include <libutils/Iteration.h>
#include <libutils/Move.h>

typedef unsigned int uint;

typedef struct
{
    const char *file;
    const char *function;
    int line;
} __SOURCE_LOCATION__;

#define SOURCE_LOCATION \
    ((__SOURCE_LOCATION__){__FILE__, __FUNCTION__, __LINE__})
