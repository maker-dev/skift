
#include <libsystem/Result.h>

#include "kernel/node/Handle.h"
#include "kernel/node/Pipe.h"

FsPipe::FsPipe() : FsNode(FILE_TYPE_PIPE)
{
}

bool FsPipe::can_read(FsHandle *handle)
{
    __unused(handle);

    // FIXME: make this atomic or something...
    return !_buffer.empty() || !writers;
}

bool FsPipe::can_write(FsHandle *handle)
{
    __unused(handle);

    // FIXME: make this atomic or something...
    return !_buffer.full() || !readers;
}

ResultOr<size_t> FsPipe::read(FsHandle &handle, void *buffer, size_t size)
{
    __unused(handle);

    if (!writers)
    {
        return ERR_STREAM_CLOSED;
    }

    return _buffer.read((char *)buffer, size);
}

ResultOr<size_t> FsPipe::write(FsHandle &handle, const void *buffer, size_t size)
{
    __unused(handle);

    if (!readers)
    {
        return ERR_STREAM_CLOSED;
    }

    return _buffer.write((const char *)buffer, size);
}
