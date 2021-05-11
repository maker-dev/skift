#pragma once

#include <libgraphic/Painter.h>
#include <libgraphic/font/Glyph.h>
#include <libio/Streams.h>
#include <libwidget/Element.h>

struct GlyphElement : public Widget::Element
{
private:
    Graphic::SuperCoolFont::Glyph _glyph;

public:
    GlyphElement(Graphic::SuperCoolFont::Glyph glyph) : _glyph{glyph}
    {
    }

    void paint(Graphic::Painter &painter, const Math::Recti &) override
    {
        painter.clear(Graphic::Colors::WHITE);
        for (auto edge : _glyph.edges.edges())
        {
            IO::logln("Draw line: {} - {}", edge.start() * 10.0f, edge.end() * 10.0f);
            painter.draw_line(edge.start() * 10.0f, edge.end() * 10.0f, Graphic::Colors::BLACK);
        }
    }

    Math::Vec2i size() override
    {
        return {100, 100};
    }
};

WIDGET_BUILDER(GlyphElement, glyph);