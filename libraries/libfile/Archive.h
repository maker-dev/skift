#pragma once

#include <libio/Reader.h>
#include <libio/Writer.h>
#include <libsystem/Result.h>
#include <libutils/Path.h>
class Archive : public RefCounted<Archive>
{
public:
    struct Entry
    {
        String name;
        size_t uncompressed_size;
        size_t compressed_size;
        size_t archive_offset;
        unsigned int compression;
    };

protected:
    Vector<Entry> _entries;
    Path _path;
    bool _valid = true;

public:
    static RefPtr<Archive> open(Path path, bool read = true);

    Archive(Path path) : _path(path)
    {
    }

    inline const Vector<Entry> &entries()
    {
        return _entries;
    }

    virtual Result extract(unsigned int entry_index, IO::Writer &writer) = 0;
    virtual Result insert(const char *entry_name, IO::Reader &reader) = 0;

    inline const Path &get_path()
    {
        return _path;
    }

    inline bool valid()
    {
        return _valid;
    }
};