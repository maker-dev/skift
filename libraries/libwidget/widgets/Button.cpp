#include <libgraphic/Painter.h>
#include <libsystem/Logger.h>
#include <libsystem/core/CString.h>

#include <libwidget/Widgets.h>
#include <libwidget/Window.h>

void button_paint(Button *widget, Painter &painter, Rectangle rectangle)
{
    __unused(rectangle);

    if (widget->enabled())
    {
        if (widget->style == BUTTON_OUTLINE)
        {
            painter.draw_rounded_rectangle(widget_get_bound(widget), 4, 1, widget_get_color(widget, THEME_BORDER));
        }
        else if (widget->style == BUTTON_FILLED)
        {
            painter.fill_rounded_rectangle(widget_get_bound(widget), 4, widget_get_color(widget, THEME_ACCENT));
        }

        if (widget->state == BUTTON_OVER)
        {
            painter.fill_rounded_rectangle(widget_get_bound(widget), 4, ALPHA(widget_get_color(widget, THEME_FOREGROUND), 0.1));
        }

        if (widget->state == BUTTON_PRESS)
        {
            painter.fill_rounded_rectangle(widget_get_bound(widget), 4, ALPHA(widget_get_color(widget, THEME_FOREGROUND), 0.2));
        }
    }
}

void button_event(Button *widget, Event *event)
{
    if (event->type == Event::MOUSE_ENTER)
    {
        widget->state = BUTTON_OVER;
        widget->should_repaint();
        event->accepted = true;
    }
    else if (event->type == Event::MOUSE_LEAVE)
    {
        widget->state = BUTTON_IDLE;
        widget->should_repaint();
        event->accepted = true;
    }
    else if (event->type == Event::MOUSE_BUTTON_PRESS)
    {
        widget->state = BUTTON_PRESS;
        widget->should_repaint();

        event->accepted = true;
    }
    else if (event->type == Event::MOUSE_BUTTON_RELEASE)
    {
        widget->state = BUTTON_OVER;
        widget->should_repaint();

        event->accepted = true;

        Event action_event = {};

        action_event.type = Event::ACTION;

        widget_event(widget, &action_event);
    }
}

static const WidgetClass button_class = {
    .paint = (WidgetPaintCallback)button_paint,
    .event = (WidgetEventCallback)button_event,
};

Button *button_create(Widget *parent, ButtonStyle style)
{
    auto widget = __create(Button);

    widget->style = style;
    widget->state = BUTTON_IDLE;

    widget->layout = HFLOW(0);
    widget->insets = Insets(0, 16);
    widget->min_height = 32;
    widget->layout_attributes |= LAYOUT_GREEDY;

    widget_initialize(widget, &button_class, parent);

    return widget;
}

Button *button_create_with_icon(Widget *parent, ButtonStyle style, RefPtr<Icon> icon)
{
    auto button = button_create(parent, style);

    button->layout = STACK();
    button->insets = Insets(4, 4);

    icon_panel_create(button, icon)->layout_attributes = LAYOUT_FILL;

    return button;
}

Button *button_create_with_text(Widget *parent, ButtonStyle style, const char *text)
{
    auto button = button_create(parent, style);

    button->insets = Insets(0, 0, 8, 8);
    button->min_width = 64;

    label_create(button, text)->layout_attributes = LAYOUT_FILL;

    return button;
}

Button *button_create_with_icon_and_text(Widget *parent, ButtonStyle style, RefPtr<Icon> icon, const char *text)
{
    auto button = button_create(parent, style);
    button->insets = Insets(0, 0, 6, 10);
    button->min_width = 64;

    Widget *button_icon = icon_panel_create(button, icon);
    button_icon->insets = Insets(0, 0, 0, 4);

    label_create(button, text);

    return button;
}
