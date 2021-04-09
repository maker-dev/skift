#include <libgraphic/Painter.h>
#include <libgraphic/svg/Rasterizer.h>
#include <libsystem/Logger.h>

namespace Graphic
{

Rasterizer::Rasterizer(RefPtr<Bitmap> bitmap) : _bitmap{bitmap}
{
    _scanline.resize(_bitmap->width());
}

void Rasterizer::clear()
{
    _edges.clear();
}

void Rasterizer::set_clip(Recti c)
{
    _clip = c;
}

Recti Rasterizer::get_clip()
{
    Recti c = _bitmap->bound().clipped_with(_edges.bound());

    if (_clip.present())
    {
        c = c.clipped_with(_clip.unwrap());
    }

    return c;
}

void Rasterizer::flatten(const Path &path, const Mat3x2f &transform)
{
    for (auto &subpath : path.subpaths())
    {
        for (size_t i = 0; i < subpath.length(); i++)
        {
            auto curve = subpath.curves(i);
            curve = transform.apply(curve);
            _edges.append(curve);
        }
    }
}

void Rasterizer::rasterize(Paint &paint)
{
    auto line = [&](float start, float end) {
        for (float x = start; x < end; x += 1.0f / 4)
        {
            if ((int)x < (int)_scanline.count())
            {
                _scanline[(int)x] += 1;
            }
        }
    };

    auto scanline = [&](int start, int end, float y) {
        _actives_edges.clear();

        for (auto &edge : _edges.edges())
        {
            if (y >= edge.min_y() && y < edge.max_y())
            {
                _actives_edges.push_back(edge);
            }
        }

        _actives_edges.sort([&](Edgef &a, Edgef &b) { return a.intersection_y(y).x() - b.intersection_y(y).x(); });

        bool odd_even = true;

        for (size_t i = 0; i + 1 < _actives_edges.count(); i++)
        {
            auto &a = _actives_edges[i];
            auto &b = _actives_edges[i + 1];

            float seg_start = MAX(a.intersection_y(y).x(), start);
            float seg_end = MIN(b.intersection_y(y).x(), end);

            if (odd_even)
            {
                line(seg_start, seg_end);
            }

            odd_even = !odd_even;
        }
    };

    auto bound = get_clip();

    for (int y = bound.top(); y < bound.bottom(); y++)
    {
        for (int i = bound.left(); i < bound.right(); i++)
        {
            _scanline[i] = 0;
        }

        for (float yy = (y - 0.5f); yy < (y + 0.5f); yy += 1.0f / 4)
        {
            scanline(bound.left(), bound.right(), yy + 0.5f);
        }

        for (int i = bound.left(); i < bound.right(); i++)
        {
            Vec2f p = {
                (i - _edges.bound().left()) / (float)_edges.bound().width(),
                (y - _edges.bound().top()) / (float)_edges.bound().height(),
            };

            auto color = sample(paint, p);
            auto alpha = clamp((_scanline[i] / 16.0f), 0, 1);

            if (alpha >= 0.003f)
            {
                _bitmap->blend_pixel_no_check({i, y}, color.with_alpha(alpha));
            }
        }
    }
}

void FLATTEN Rasterizer::fill(Path &path, const Mat3x2f &transform, Paint paint)
{
    clear();
    flatten(path, transform);
    rasterize(paint);
}

} // namespace Graphic
