#pragma once

#include <libsystem/Common.h>
#include <libsystem/Result.h>
#include <libsystem/io/File.h>

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

    Archive(Path path) : _path(path)
    {
    }

    inline const Vector<Entry> &entries()
    {
        return _entries;
    }

    virtual Result extract(unsigned int entry_index, const char *dest_path) = 0;
    virtual Result insert(const char *entry_name, const char *src_dir) = 0;

    inline const Path &get_path()
    {
        return _path;
    }

    inline bool valid()
    {
        return _valid;
    }

protected:
    Vector<Entry> _entries;
    Path _path;
    bool _valid = true;
};