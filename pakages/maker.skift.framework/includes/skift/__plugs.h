#pragma once
#include <skift/generic.h>
// Framework plugs to the syscalls or the kernel.

void __plug_putchar(int c);
int __plug_print(const char *buffer);

int __plug_getchar();
void __plug_read(char * buffer, uint size);

void __plug_assert_failed(const char *expr, const char *file, const char *function, int line);

// Memory allocator plugs
int __plug_memalloc_lock();
int __plug_memalloc_unlock();

void* __plug_memalloc_alloc(uint size);
int __plug_memalloc_free(void* memory, uint size);