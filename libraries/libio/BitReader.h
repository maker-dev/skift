#pragma once

#include <libio/Read.h>
#include <libsystem/Logger.h>
#include <libtest/Assets.h>
#include <libutils/InlineRingBuffer.h>

namespace IO
{
class BitReader
{
private:
    IO::Reader &_reader;
    utils::InlineRingBuffer<uint8_t, 16> _buffer;
    size_t _head = 0;
    size_t _consumed = 0;
    bool _end_of_file;

public:
    inline BitReader(IO::Reader &reader) : _reader(reader) {}

    inline Result hint(size_t num_bits)
    {
        size_t num_bytes = __align_up(_head + num_bits, 8) / 8;

        while (_buffer.used() < num_bytes)
        {
            uint8_t byte;

            if (TRY(_reader.read(&byte, sizeof(byte))) == 0)
            {
                _end_of_file = true;
            }

            _consumed += sizeof(byte);
            _buffer.put(byte);
        }

        return SUCCESS;
    }

    inline size_t consumed()
    {
        return _consumed;
    }

    inline void flush()
    {
        _head = 0;
        _buffer.flush();
    }

    template <class T>
    inline T grab()
    {
        hint(sizeof(T) * 8);

        T value;

        for (size_t i = 0; i < sizeof(T); i++)
        {
            (reinterpret_cast<uint8_t *>(&value))[i] = grab_bits(8);
        }

        return value;
    }

    template <class T>
    inline T grab_aligned()
    {
        flush();
        return grab<T>();
    }

    template <class T>
    inline T peek()
    {
        hint(sizeof(T) * 8);

        T value;

        for (size_t i = 0; i < sizeof(T); i++)
        {
            (reinterpret_cast<uint8_t *>(&value))[i] = peek_bits(i * 8, 8);
        }

        return value;
    }

    inline void skip_bits(size_t num_bits)
    {
        hint(num_bits);

        for (size_t i = 0; i < num_bits; i++)
        {
            grab_bit();
        }
    }

    inline uint8_t grab_bit()
    {
        uint8_t bit = peek_bit(0);

        _head++;
        if (_head == 8)
        {
            _head -= 8;
            _buffer.get(); // grab the byte
        }

        return bit;
    }

    inline uint32_t grab_bits(size_t num_bits)
    {
        hint(num_bits);

        assert_lower_equal(num_bits, 32);

        uint32_t result = 0;

        for (size_t i = 0; i < num_bits; i++)
        {
            uint8_t bit = grab_bit();
            result |= bit << (i);
        }

        return result;
    }

    inline uint8_t peek_bit(size_t index)
    {
        size_t num_bit = _head + index;
        size_t num_byte = num_bit / 8;
        size_t bit_within_the_byte = num_bit - (num_byte * 8);

        uint8_t byte = _buffer.peek(num_byte);

        return ((byte) >> (bit_within_the_byte)) & 1;
    }

    inline uint32_t peek_bits(size_t num_bits)
    {
        return peek_bits(0, num_bits);
    }

    inline uint32_t peek_bits(size_t offset, size_t num_bits)
    {
        hint(offset + num_bits);

        assert_lower_equal(num_bits, 32);

        uint32_t result = 0;

        for (size_t i = 0; i < num_bits; i++)
        {
            uint8_t bit = peek_bit(offset + i);
            result |= bit << (i);
        }

        return result;
    }

    inline uint32_t grab_bits_reverse(size_t num_bits)
    {
        hint(num_bits);

        assert_lower_equal(num_bits, 32);

        uint32_t result = 0;

        for (size_t i = 0; i < num_bits; i++)
        {
            uint8_t bit = grab_bit();
            result |= bit << ((num_bits -1) - i);
        }

        return result;
    }

    inline uint32_t peek_bits_reverse(size_t num_bits)
    {
        hint(num_bits);

        assert_lower_equal(num_bits, 32);

        uint32_t result = 0;

        for (size_t i = 0; i < num_bits; i++)
        {
            uint8_t bit = peek_bit(i);
            result |= bit << ((num_bits - 1) - i);
        }

        return result;
    }
};

} // namespace IO