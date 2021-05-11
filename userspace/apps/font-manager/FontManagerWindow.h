#include <font-manager/widgets/GlyphElement.h>
#include <libfilepicker/FilePicker.h>
#include <libgraphic/font/FontFace.h>
#include <libgraphic/font/Parser.h>
#include <libgraphic/rast/Rasterizer.h>
#include <libio/File.h>
#include <libio/Streams.h>
#include <libwidget/Components.h>
#include <libwidget/Layouts.h>
#include <libwidget/Window.h>

using namespace Widget;

struct FontManagerWindow : public Window
{
private:
    RefPtr<Graphic::SuperCoolFont::FontFace> _font_face;
    Graphic::SuperCoolFont::Glyph _current_glyph;
    Codepoint _current_codepoint = 'A';

public:
    FontManagerWindow(String path) : Window(WINDOW_RESIZABLE)
    {
        size(Math::Vec2i(700, 500));
        load_font(path);
    }

    Result load_font(String path)
    {
        if (!path.null_or_empty())
        {
            IO::logln("Opening font from {}", path);
            IO::File file{path, OPEN_READ};
            _font_face = TRY(Graphic::SuperCoolFont::load(file));
            update_glyph();
        }

        return Result::SUCCESS;
    }

    void update_glyph()
    {
        auto opt_glyph = _font_face->glyph(_current_codepoint);
        if (opt_glyph.present())
        {
            _current_glyph = _font_face->glyph(_current_codepoint).unwrap();
        }
        should_rebuild();
    }

    RefPtr<Element> build() override
    {
        // clang-format off

        return vflow({
            titlebar(Graphic::Icon::get("font-format"), "Font Manager"),
            fill(
                glyph(_current_glyph))
                });

        // clang-format on
    }
};
