
#include <libjson/Json.h>
#include <libsystem/Logger.h>
#include <libsystem/Result.h>
#include <libsystem/core/CString.h>
#include <libsystem/math/MinMax.h>
#include <libsystem/thread/Atomic.h>

#include "kernel/filesystem/Filesystem.h"
#include "kernel/node/Handle.h"
#include "kernel/node/ProcessInfo.h"
#include "kernel/scheduling/Scheduler.h"
#include "kernel/tasking/Task-Memory.h"

static Iteration serialize_task(JsonValue *destination, Task *task)
{
    if (task->id == 0)
        return Iteration::CONTINUE;

    JsonValue *task_object = json_create_object();

    json_object_put(task_object, "id", json_create_integer(task->id));
    json_object_put(task_object, "name", json_create_string(task->name));
    json_object_put(task_object, "state", json_create_string(task_state_string(task->state)));
    json_object_put(task_object, "directory", json_create_string_adopt(path_as_string(task->directory)));
    json_object_put(task_object, "cpu", json_create_integer(scheduler_get_usage(task->id)));
    json_object_put(task_object, "ram", json_create_integer(task_memory_usage(task)));
    json_object_put(task_object, "user", json_create_boolean(task->user));

    json_array_append(destination, task_object);

    return Iteration::CONTINUE;
}

static Result process_info_open(FsProcessInfo *node, FsHandle *handle)
{
    __unused(node);

    JsonValue *destination = json_create_array();

    task_iterate(destination, (TaskIterateCallback)serialize_task);

    handle->attached = json_stringify(destination);
    handle->attached_size = strlen((const char *)handle->attached);

    json_destroy(destination);

    return SUCCESS;
}

static void process_info_close(FsProcessInfo *node, FsHandle *handle)
{
    __unused(node);

    if (handle->attached)
    {
        free(handle->attached);
    }
}

static Result process_info_read(FsProcessInfo *node, FsHandle *handle, void *buffer, size_t size, size_t *read)
{
    __unused(node);

    if (handle->offset <= handle->attached_size)
    {
        *read = MIN(handle->attached_size - handle->offset, size);
        memcpy(buffer, (char *)handle->attached + handle->offset, *read);
    }

    return SUCCESS;
}

static size_t process_info_size(FsProcessInfo *node, FsHandle *handle)
{
    __unused(node);

    if (handle == nullptr)
    {
        return 0;
    }
    else
    {
        return handle->attached_size;
    }
}

static FsNode *process_info_create()
{
    FsProcessInfo *info = __create(FsProcessInfo);

    fsnode_init(info, FILE_TYPE_DEVICE);

    info->open = (FsNodeOpenCallback)process_info_open;
    info->close = (FsNodeCloseCallback)process_info_close;
    info->read = (FsNodeReadCallback)process_info_read;
    info->size = (FsNodeSizeCallback)process_info_size;

    return (FsNode *)info;
}

void process_info_initialize()
{
    FsNode *info_device = process_info_create();

    Path *info_device_path = path_create("/System/processes");
    filesystem_link_and_take_ref(info_device_path, info_device);
    path_destroy(info_device_path);
}
