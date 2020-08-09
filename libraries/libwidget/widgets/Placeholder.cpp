#include <libgraphic/Painter.h>
#include <libsystem/core/CString.h>
#include <libwidget/Window.h>
#include <libwidget/widgets/Placeholder.h>

void placeholder_paint(Placeholder *placeholder, Painter &painter, Rectangle rectangle)
{
    __unused(rectangle);
    painter.draw_rectangle(widget_get_bound(placeholder), COLOR_RED);

    auto alert_icon = Icon::get("alert");

    painter.blit_icon(
        *alert_icon,
        ICON_18PX,
        alert_icon->bound(ICON_18PX).moved(widget_get_bound(placeholder).position() + Vec2i(8, 8)),
        COLOR_RED);

    painter.draw_string(
        *widget_font(),
        placeholder->text,
        widget_get_bound(placeholder).position() + Vec2i(32, 10),
        widget_get_color(placeholder, THEME_FOREGROUND));
}

void placeholder_destroy(Placeholder *placeholder)
{
    free(placeholder->text);
}

static const WidgetClass placeholder_class = {
    .destroy = (WidgetDestroyCallback)placeholder_destroy,
    .paint = (WidgetPaintCallback)placeholder_paint,
};

Placeholder *placeholder_create(Widget *parent, const char *text)
{
    auto placeholder = __create(Placeholder);

    char buffer[256];
    snprintf(buffer, 256, "Cannot create an instance of \"%s\".", text);

    placeholder->text = strdup(buffer);

    widget_initialize(placeholder, &placeholder_class, parent);

    return placeholder;
}
