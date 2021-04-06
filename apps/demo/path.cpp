#include <libgraphic/Bitmap.h>
#include <libgraphic/svg/Rasterizer.h>
#include <libutils/Chrono.h>

#include "demo/Demos.h"

void path_draw(Graphic::Painter &painter, Recti, float time)
{
    painter.clear(Graphic::Colors::BLACK);

    auto t = Trans2f::scale(8) * Trans2f::translation({32, 64});
    Graphic::Rasterizer rast{painter.bitmap()};

    static auto p0 = Graphic::Path::parse("M12,8L10.67,8.09C9.81,7.07 7.4,4.5 5,4.5C5,4.5 3.03,7.46 4.96,11.41C4.41,12.24 4.07,12.67 4,13.66L2.07,13.95L2.28,14.93L4.04,14.67L4.18,15.38L2.61,16.32L3.08,17.21L4.53,16.32C5.68,18.76 8.59,20 12,20C15.41,20 18.32,18.76 19.47,16.32L20.92,17.21L21.39,16.32L19.82,15.38L19.96,14.67L21.72,14.93L21.93,13.95L20,13.66C19.93,12.67 19.59,12.24 19.04,11.41C20.97,7.46 19,4.5 19,4.5C16.6,4.5 14.19,7.07 13.33,8.09L12,8M9,11A1,1 0 0,1 10,12A1,1 0 0,1 9,13A1,1 0 0,1 8,12A1,1 0 0,1 9,11M15,11A1,1 0 0,1 16,12A1,1 0 0,1 15,13A1,1 0 0,1 14,12A1,1 0 0,1 15,11M11,14H13L12.3,15.39C12.5,16.03 13.06,16.5 13.75,16.5A1.5,1.5 0 0,0 15.25,15H15.75A2,2 0 0,1 13.75,17C13,17 12.35,16.59 12,16V16H12C11.65,16.59 11,17 10.25,17A2,2 0 0,1 8.25,15H8.75A1.5,1.5 0 0,0 10.25,16.5C10.94,16.5 11.5,16.03 11.7,15.39L11,14Z");
    static auto image = Graphic::Bitmap::load_from("/Files/Profiles/avocado.png").unwrap();

    auto texture = Graphic::Texture{
        Trans2f::scale(0.5) * Trans2f::rotation(time) * Trans2f::translation({0.5, 0.5}),
        image,
    };

    auto gradient = Graphic::Gradient{
        Trans2f::rotation(PI / 4) * Trans2f::translation({0.5, 0.5}),
        {
            {Graphic::Color::from_rgb_byte(91, 206, 250), 0.2},
            {Graphic::Color::from_rgb_byte(245, 169, 184), 0.2},
            {Graphic::Color::from_rgb_byte(245, 169, 184), 0.4},

            {Graphic::Color::from_rgb_byte(255, 255, 255), 0.4},
            {Graphic::Color::from_rgb_byte(255, 255, 255), 0.6},

            {Graphic::Color::from_rgb_byte(245, 169, 184), 0.6},
            {Graphic::Color::from_rgb_byte(245, 169, 184), 0.8},
            {Graphic::Color::from_rgb_byte(91, 206, 250), 0.8},
        },
        8,
    };

    rast.fill(p0, t, gradient);
    rast.fill(p0, Trans2f::scale(8) * Trans2f::translation({32, 256}), texture);
}