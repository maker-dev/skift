#pragma once

#include <libgraphic/font/Glyph.h>
#include <libio/Reader.h>
#include <libutils/unicode/Codepoint.h>

namespace Graphic::SuperCoolFont
{
enum FontStyle
{
    FONT_STYLE_REGULAR  = (1 << 0),
    FONT_STYLE_BOLD     = (1 << 1),
    FONT_STYLE_ITALIC   = (1 << 2)
};

class FontFace : public RefCounted<FontFace>
{
public:
    virtual String family() = 0;
    virtual FontStyle style() = 0;
    virtual Optional<Glyph> glyph(Codepoint c) = 0;
};
} // namespace Graphic::SuperCoolFont