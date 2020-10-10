#pragma once

// this header list all "plugs" function between the library and the syscalls or the kernel

#include <abi/Filesystem.h>
#include <abi/Handle.h>
#include <abi/IOCall.h>
#include <abi/Launchpad.h>
#include <abi/System.h>

#include <libsystem/Time.h>
#include <libsystem/thread/Lock.h>

extern "C" void __plug_init();

extern "C" void __plug_fini(int exit_code);

void __no_return __plug_assert_failed(const char *expr, const char *file, const char *function, int line);

void __plug_lock_assert_failed(Lock *lock, const char *file, const char *function, int line);

/* --- Logger --------------------------------------------------------------- */

void __plug_logger_lock();

void __plug_logger_unlock();

void __no_return __plug_logger_fatal();

/* --- Memory allocator ----------------------------------------------------- */

int __plug_memalloc_lock();

int __plug_memalloc_unlock();

void *__plug_memalloc_alloc(size_t size);

void __plug_memalloc_free(void *address, size_t size);

/* --- File system ---------------------------------------------------------- */

Result __plug_filesystem_link(const char *oldpath, const char *newpath);

Result __plug_filesystem_unlink(const char *path);

Result __plug_filesystem_mkdir(const char *path);

Result __plug_filesystem_rmdir(const char *path);

/* --- System --------------------------------------------------------------- */

void __plug_system_get_info(SystemInfo *info);

void __plug_system_get_status(SystemStatus *status);

TimeStamp __plug_system_get_time();

uint __plug_system_get_ticks();

/* --- Processes ------------------------------------------------------------ */

int __plug_process_this();

Result __plug_process_launch(Launchpad *launchpad, int *pid);

void __no_return __plug_process_exit(int code);

Result __plug_process_cancel(int pid);

Result __plug_process_get_directory(char *buffer, uint size);

Result __plug_process_set_directory(const char *directory);

Result __plug_process_sleep(int time);

Result __plug_process_wait(int pid, int *exit_value);

/* --- I/O ------------------------------------------------------------------ */

void __plug_handle_open(Handle *handle, const char *path, OpenFlag flags);

void __plug_handle_close(Handle *handle);

Result __plug_handle_select(
    HandleSet *handles,
    int *selected,
    SelectEvent *selected_events,
    Timeout timeout);

size_t __plug_handle_read(Handle *handle, void *buffer, size_t size);

size_t __plug_handle_write(Handle *handle, const void *buffer, size_t size);

Result __plug_handle_call(Handle *handle, IOCall request, void *args);

int __plug_handle_seek(Handle *handle, int offset, Whence whence);

int __plug_handle_tell(Handle *handle, Whence whence);

int __plug_handle_stat(Handle *handle, FileState *stat);

void __plug_handle_connect(Handle *handle, const char *path);

void __plug_handle_accept(Handle *handle, Handle *connection_handle);

Result __plug_create_pipe(int *reader_handle, int *writer_handle);

Result __plug_create_term(int *master_handle, int *slave_handle);
