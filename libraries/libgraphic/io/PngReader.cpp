#include <libcompression/Inflate.h>
#include <libgraphic/io/PngCommon.h>
#include <libgraphic/io/PngReader.h>
#include <libio/Read.h>
#include <libsystem/Logger.h>
#include <libutils/Array.h>

graphic::PngReader::PngReader(IO::Reader &reader) : _reader(reader)
{
    read();
    logger_trace("Image dims: %u %u", _width, _height);
}

Result graphic::PngReader::read()
{
    Array<uint8_t, 8> signature;
    _reader.read(signature.raw_storage(), sizeof(signature));

    // Doesn't matter if the read above can't read all bytes, this will fail anyways
    if (signature != Array<uint8_t, 8>{137, 80, 78, 71, 13, 10, 26, 10})
    {
        logger_error("Invalid PNG signature!");
        return Result::ERR_INVALID_DATA;
    }

    bool end = false;
    while (!end)
    {
        auto chunk_length = TRY(IO::read<be_uint32_t>(_reader));
        auto chunk_signature = TRY(IO::read<be_uint32_t>(_reader));

        switch (chunk_signature())
        {
        case Png::ImageHeader::SIG:
        {
            auto image_header = TRY(IO::read<Png::ImageHeader>(_reader));
            _width = image_header.width();
            _height = image_header.height();
            logger_trace("Compression method: %u", image_header.compression_method);
        }
        break;

        case Png::Gamma::SIG:
        {
            TRY(IO::read<Png::Gamma>(_reader));
        }
        break;

        case Png::Chroma::SIG:
        {
            TRY(IO::read<Png::Chroma>(_reader));
        }
        break;

        case Png::BackgroundColor::SIG:
        {
            char buf[6];
            _reader.read(buf, chunk_length());
        }
        break;

        case Png::Time::SIG:
        {
            TRY(IO::read<Png::Time>(_reader));
        }
        break;

        case Png::ImageData::SIG:
        {
            Inflate inflate;

            Vector<uint8_t> data(chunk_length());
            TRY(_reader.read(data.raw_storage(), chunk_length()));
        }
        break;

        case Png::TextualData::SIG:
        {
            Vector<uint8_t> data(chunk_length());
            TRY(_reader.read(data.raw_storage(), chunk_length()));
        }
        break;

        case Png::ImageEnd::SIG:
        {
            end = true;
        }
        break;

        default:
        {
            logger_error("Invalid PNG chunk: %u", chunk_signature());
            Vector<uint8_t> data(chunk_length());
            TRY(_reader.read(data.raw_storage(), chunk_length()));
        }
        break;
        }

        auto crc = TRY(IO::read<be_uint32_t>(_reader));
        __unused(crc);
    }

    return Result::SUCCESS;
}
