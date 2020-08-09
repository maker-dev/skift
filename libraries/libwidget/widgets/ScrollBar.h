#pragma once

#include <libwidget/Widget.h>

struct ScrollBar : public Widget
{
    Vec2i mouse_origine;

    int track;
    int thumb;
    int value;
};

ScrollBar *scrollbar_create(Widget *parent);
