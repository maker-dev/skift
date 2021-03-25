#include <libgraphic/Bitmap.h>
#include <libgraphic/png/PngReader.h>
#include <libio/Copy.h>
#include <libio/File.h>
#include <libio/MemoryReader.h>
#include <libsystem/system/Memory.h>

static Color _placeholder_buffer[] = {
    Colors::MAGENTA,
    Colors::BLACK,
    Colors::BLACK,
    Colors::MAGENTA,
};

ResultOr<RefPtr<Bitmap>> Bitmap::create_shared(int width, int height)
{
    Color *pixels = nullptr;
    TRY(memory_alloc(width * height * sizeof(Color), reinterpret_cast<uintptr_t *>(&pixels)));

    int handle = -1;
    memory_get_handle(reinterpret_cast<uintptr_t>(pixels), &handle);

    auto bitmap = make<Bitmap>(handle, BITMAP_SHARED, width, height, pixels);
    bitmap->clear(Colors::BLACK);

    return bitmap;
}

ResultOr<RefPtr<Bitmap>> Bitmap::create_shared_from_handle(int handle, Vec2i width_and_height)
{
    Color *pixels = nullptr;
    size_t size = 0;

    TRY(memory_include(handle, reinterpret_cast<uintptr_t *>(&pixels), &size));

    if (size < width_and_height.x() * width_and_height.y() * sizeof(Color))
    {
        memory_free(reinterpret_cast<uintptr_t>(pixels));
        return ERR_BAD_IMAGE_FILE_FORMAT;
    }

    memory_get_handle(reinterpret_cast<uintptr_t>(pixels), &handle);

    return make<Bitmap>(handle, BITMAP_SHARED, width_and_height.x(), width_and_height.y(), pixels);
}

RefPtr<Bitmap> Bitmap::create_static(int width, int height, Color *pixels)
{
    return make<Bitmap>(-1, BITMAP_STATIC, width, height, pixels);
}

RefPtr<Bitmap> Bitmap::placeholder()
{
    return create_static(2, 2, _placeholder_buffer);
}

ResultOr<RefPtr<Bitmap>> Bitmap::load_from(String path)
{
    IO::File file{path, OPEN_READ};

    if (!file.exist())
    {
        return ERR_NO_SUCH_FILE_OR_DIRECTORY;
    }

    Graphic::PngReader png_reader{file};

    if (!png_reader.valid())
    {
        return ERR_BAD_IMAGE_FILE_FORMAT;
    }

    unsigned int decoded_width = png_reader.width();
    unsigned int decoded_height = png_reader.height();
    const Color *decoded_data = png_reader.pixels().raw_storage();

    auto bitmap_or_result = Bitmap::create_shared(decoded_width, decoded_height);

    if (bitmap_or_result.success())
    {
        auto bitmap = bitmap_or_result.take_value();
        memcpy(bitmap->pixels(), decoded_data, sizeof(Color) * decoded_width * decoded_height);
        return bitmap;
    }
    else
    {
        return bitmap_or_result;
    }
}

RefPtr<Bitmap> Bitmap::load_from_or_placeholder(String path)
{
    auto result = load_from(path);

    if (!result.success())
    {
        return placeholder();
    }

    return result.take_value();
}

Result Bitmap::save_to(String path)
{
    __unused(path);
    return Result::ERR_OPERATION_NOT_SUPPORTED;

    // void *outbuffer __cleanup_malloc = nullptr;

    // IO::File file{path, OPEN_READ};

    // if (!file.exist())
    // {
    //     return ERR_NO_SUCH_FILE_OR_DIRECTORY;
    // }

    // return IO::write_all(file, {outbuffer, outbuffer_size});
}

Bitmap::~Bitmap()
{
    if (_storage == BITMAP_SHARED)
    {
        memory_free(reinterpret_cast<uintptr_t>(_pixels));
    }
}
