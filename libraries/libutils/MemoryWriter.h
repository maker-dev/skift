#pragma once
#include <libsystem/io/Stream.h>
#include <libutils/Path.h>
#include <libutils/Writer.h>

class MemoryWriter final : public Writer
{
public:
    MemoryWriter(size_t reserve = 0);

    virtual size_t length() override;
    virtual size_t position() override;

    virtual void flush() override;
    virtual void write(const void *buffer, size_t size) override;

    const Vector<uint8_t> &data()
    {
        return _data;
    }

private:
    Vector<uint8_t> _data;
};