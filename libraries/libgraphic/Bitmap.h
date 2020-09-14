#pragma once

#include <libgraphic/Color.h>
#include <libgraphic/Shape.h>
#include <libsystem/Result.h>
#include <libsystem/math/Math.h>

#include <libutils/RefPtr.h>
#include <libutils/ResultOr.h>

enum BitmapStorage
{
    BITMAP_SHARED,
    BITMAP_STATIC,
    BITMAP_MALLOC,
};

enum BitmapFiltering
{
    BITMAP_FILTERING_NEAREST,
    BITMAP_FILTERING_LINEAR,
};

class Bitmap : public RefCounted<Bitmap>
{
private:
    int _handle;
    BitmapStorage _storage;
    int _width;
    int _height;
    BitmapFiltering _filtering;
    Color *_pixels;

    __noncopyable(Bitmap);
    __nonmovable(Bitmap);

public:
    Bitmap(int handle, BitmapStorage storage, int width, int height, Color *pixels)
        : _handle(handle),
          _storage(storage),
          _width(width),
          _height(height),
          _filtering(BITMAP_FILTERING_LINEAR),
          _pixels(pixels)
    {
    }

    ~Bitmap();

    Color *pixels() { return _pixels; }

    int handle() const { return _handle; }
    int width() const { return _width; }
    int height() const { return _height; }
    Vec2i size() const { return Vec2i(_width, _height); }
    Rectangle bound() const { return Rectangle(_width, _height); }

    void filtering(BitmapFiltering filtering) { _filtering = filtering; }

    static ResultOr<RefPtr<Bitmap>> create_shared(int width, int height);

    static ResultOr<RefPtr<Bitmap>> create_shared_from_handle(int handle, Vec2i width_and_height);

    static RefPtr<Bitmap> create_static(int width, int height, Color *pixels);

    static ResultOr<RefPtr<Bitmap>> load_from(const char *path);

    static RefPtr<Bitmap> load_from_or_placeholder(const char *path);

    Result save_to(const char *path);

    void set_pixel(Vec2i position, Color color)
    {
        if (bound().contains(position))
            _pixels[(int)(position.x() + position.y() * width())] = color;
    }

    void set_pixel_no_check(Vec2i position, Color color)
    {
        _pixels[(int)(position.x() + position.y() * width())] = color;
    }

    void blend_pixel(Vec2i position, Color color)
    {
        Color background = get_pixel(position);
        set_pixel(position, Color::blend(color, background));
    }

    void blend_pixel_no_check(Vec2i position, Color color)
    {
        Color background = get_pixel_no_check(position);
        set_pixel_no_check(position, Color::blend(color, background));
    }

    Color get_pixel(Vec2i position)
    {
        return _pixels[clamp(position.y(), 0, height() - 1) * width() + clamp(position.x(), 0, width() - 1)];
    }

    Color get_pixel_no_check(Vec2i position)
    {
        return _pixels[position.y() * width() + position.x()];
    }

    __flatten Color sample(Vec2f position)
    {
        return sample(bound(), position);
    }

    __flatten Color sample(Rectangle source, Vec2f position)
    {
        Vec2i sample_position = source.position() + (source.size() * position);

        if (_filtering == BITMAP_FILTERING_NEAREST)
        {
            return get_pixel(source.position() + sample_position);
        }

        Color c00 = get_pixel(sample_position + Vec2i(0, 0));
        Color c10 = get_pixel(sample_position + Vec2i(1, 0));
        Color c01 = get_pixel(sample_position + Vec2i(0, 1));
        Color c11 = get_pixel(sample_position + Vec2i(1, 1));

        float xx = source.width() * position.x();
        float yy = source.height() * position.y();

        return Color::blerp(c00, c10, c01, c11, xx - (int)xx, yy - (int)yy);
    }

    __flatten void copy_from(Bitmap &source, Rectangle region)
    {
        region = region.clipped_with(source.bound());
        region = region.clipped_with(bound());

        if (region.is_empty())
            return;

        for (int y = region.y(); y < region.y() + region.height(); y++)
        {
            for (int x = region.x(); x < region.x() + region.width(); x++)
            {
                set_pixel_no_check(Vec2i(x, y), source.get_pixel_no_check(Vec2i(x, y)));
            }
        }
    }

    void clear(Color color)
    {
        for (int i = 0; i < width() * height(); i++)
        {
            pixels()[i] = color;
        }
    }
};
