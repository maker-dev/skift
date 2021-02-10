#pragma once
#include <libsystem/Common.h>
#include <libc/skift/Time.h>
#include <assert.h>

/* --- Memory allocator ----------------------------------------------------- */

void __plug_memory_lock();

void __plug_memory_unlock();

void *__plug_memory_alloc(size_t size);

void __plug_memory_free(void *address, size_t size);

/* --- Assert ---------------------------------------------------------------- */

void __no_return __plug_assert_failed(const char *expr, const char *file, const char *function, int line);

/* --- Time ------------------------------------------------------------------ */

TimeStamp __plug_system_get_time();