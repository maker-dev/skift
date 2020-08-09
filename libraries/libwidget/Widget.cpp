#include <libgraphic/Font.h>
#include <libgraphic/Painter.h>
#include <libsystem/Assert.h>
#include <libsystem/Logger.h>
#include <libsystem/io/Stream.h>
#include <libsystem/math/MinMax.h>
#include <libwidget/Application.h>
#include <libwidget/Event.h>
#include <libwidget/Theme.h>
#include <libwidget/Widget.h>
#include <libwidget/Window.h>

/* --- Enable/ Disable state -------------------------------------------- */

bool Widget::enabled() { return _enabled; }

bool Widget::disabled() { return !_enabled; }

void Widget::enable()
{
    if (disabled())
    {
        _enabled = true;
        should_repaint();
    }
}

void Widget::disable()
{
    if (enabled())
    {
        _enabled = false;
        should_repaint();
    }
}

void Widget::disable_if(bool condition)
{
    if (condition)
        disable();
    else
        enable();
}

void Widget::enable_if(bool condition)
{
    if (condition)
        enable();
    else
        disable();
}

/* --- Focus state ---------------------------------------------------------- */

bool Widget::focused()
{
    return window && window->focused_widget == this;
}

void Widget::focus()
{
    if (window)
        window_set_focused_widget(window, this);
}

/* --- Paint ---------------------------------------------------------------- */

void Widget::should_repaint()
{
    if (window)
    {
        window_schedule_update(window, bound);
    }
}

void Widget::should_repaint(Rectangle rectangle)
{
    if (window)
    {
        window_schedule_update(window, rectangle);
    }
}

/* --- Events ----------------------------------------------------------------*/

void Widget::on(EventType event_type, EventHandler handler)
{
    assert(event_type < EventType::__COUNT);
    handlers[event_type] = move(handler);
}

static RefPtr<Font> _widget_font = nullptr;
RefPtr<Font> widget_font()
{
    if (_widget_font == nullptr)
    {
        _widget_font = Font::create("sans").take_value();
    }

    return _widget_font;
}

void widget_initialize(
    Widget *widget,
    const WidgetClass *klass,
    Widget *parent)
{
    assert(widget != nullptr);
    assert(klass != nullptr);

    widget->_enabled = true;
    widget->klass = klass;
    widget->childs = list_create();
    widget->bound = Rectangle(32, 32);

    if (parent != nullptr)
    {
        widget->window = parent->window;
        widget_add_child(parent, widget);
    }
}

void widget_destroy(Widget *widget)
{
    if (widget->klass->destroy)
    {
        widget->klass->destroy(widget);
    }

    Widget *child = (Widget *)list_peek(widget->childs);
    while (child)
    {
        widget_destroy(child);
        child = (Widget *)list_peek(widget->childs);
    }

    list_destroy(widget->childs);

    if (widget->parent)
    {
        widget_remove_child(widget->parent, widget);
    }

    if (widget->window)
    {
        window_widget_removed(widget->window, widget);
    }

    free(widget);
}

void widget_invalidate_layout(Widget *widget)
{
    if (widget->window)
    {
        window_schedule_layout(widget->window);
    }
}

void widget_add_child(Widget *widget, Widget *child)
{
    assert(child->parent == nullptr);

    child->parent = widget;
    child->window = widget->window;
    list_pushback(widget->childs, child);

    widget_invalidate_layout(widget);
}

void widget_remove_child(Widget *widget, Widget *child)
{
    assert(child->parent == widget);

    child->parent = nullptr;
    child->window = nullptr;
    list_remove(widget->childs, child);

    widget_invalidate_layout(widget);
}

void widget_clear_childs(Widget *widget)
{
    Widget *child = (Widget *)list_peek(widget->childs);
    while (child)
    {
        widget_destroy(child);
        child = (Widget *)list_peek(widget->childs);
    }
}

void widget_event(Widget *widget, Event *event)
{
    if (widget->klass->event)
    {
        widget->klass->event(widget, event);
    }

    if (!event->accepted && widget->handlers[event->type])
    {
        event->accepted = true;
        widget->handlers[event->type](event);
    }

    if (!event->accepted && widget->parent)
    {
        widget_event(widget->parent, event);
    }
}

void widget_paint(Widget *widget, Painter &painter, Rectangle rectangle)
{
    if (widget_get_bound(widget).width() == 0 ||
        widget_get_bound(widget).height() == 0)
        return;

    painter.push();
    painter.clip(widget_get_bound(widget));

    if (application_is_debbuging_layout())
        painter.fill_insets(widget_get_bound(widget), widget->insets, ALPHA(COLOR_MAGENTA, 0.25));

    painter.push();
    if (widget->klass->paint)
    {
        widget->klass->paint(widget, painter, rectangle);
    }
    painter.pop();

    list_foreach(Widget, child, widget->childs)
    {
        if (rectangle.colide_with(child->bound))
        {
            widget_paint(child, painter, rectangle);
        }
    }

    if (application_is_debbuging_layout())
        painter.draw_rectangle(widget_get_bound(widget), ALPHA(COLOR_CYAN, 0.25));

    painter.pop();
}

Vec2i widget_compute_size(Widget *widget)
{
    int width = 0;
    int height = 0;

    if (widget->klass->size)
    {
        Vec2i size = widget->klass->size(widget);
        width = size.x();
        height = size.y();
    }
    else if (widget->layout.type == LAYOUT_STACK)
    {

        width = widget->min_width;
        height = widget->min_height;

        list_foreach(Widget, child, widget->childs)
        {
            Vec2i child_size = widget_compute_size(child);

            width = MAX(width, child_size.x());
            height = MAX(height, child_size.y());
        }
    }
    else
    {
        list_foreach(Widget, child, widget->childs)
        {
            Vec2i child_size = widget_compute_size(child);

            switch (widget->layout.type)
            {
            case LAYOUT_HFLOW:
            case LAYOUT_HGRID:
                width += child_size.x();
                height = MAX(height, child_size.y());
                break;

            case LAYOUT_VFLOW:
            case LAYOUT_VGRID:
                width = MAX(width, child_size.x());
                height += child_size.y();
                break;

            default:
                width = MAX(width, child_size.x());
                height = MAX(height, child_size.y());
                break;
            }
        }

        if (widget->layout.type == LAYOUT_HFLOW || widget->layout.type == LAYOUT_HGRID)
        {
            width += widget->layout.spacing.x() * (widget->childs->count() - 1);
        }

        if (widget->layout.type == LAYOUT_VFLOW || widget->layout.type == LAYOUT_VGRID)
        {
            height += widget->layout.spacing.y() * (widget->childs->count() - 1);
        }
    }

    width += widget->insets.left();
    width += widget->insets.right();

    height += widget->insets.top();
    height += widget->insets.bottom();

    if (widget->max_width)
    {
        width = MIN(width, widget->max_width);
    }

    if (widget->max_height)
    {
        height = MIN(height, widget->max_height);
    }

    if (widget->min_width)
    {
        width = MAX(width, widget->min_width);
    }

    if (widget->min_height)
    {
        height = MAX(height, widget->min_height);
    }

    return Vec2i(width, height);
}

static void widget_do_vhgrid_layout(Widget *widget, Layout layout, Dimension dim)
{
    Dimension ivdim = dimension_invert_xy(dim);

    int current = widget_get_content_bound(widget).position().component(dim);
    int spacing = layout.spacing.component(dim);
    int size = widget_get_content_bound(widget).size().component(dim);
    int used_space_without_spacing = size - (spacing * (widget->childs->count() - 1));
    int child_size = used_space_without_spacing / widget->childs->count();
    int used_space_with_spacing = child_size * widget->childs->count() + (spacing * (widget->childs->count() - 1));
    int correction_space = size - used_space_with_spacing;

    list_foreach(Widget, child, widget->childs)
    {
        int current_child_size = MAX(1, child_size);

        if (correction_space > 0)
        {
            current_child_size++;
            correction_space--;
        }

        if (dim == Dimension::X)
        {
            child->bound = Rectangle(
                current,
                widget_get_content_bound(widget).position().component(ivdim),
                current_child_size,
                widget_get_content_bound(widget).size().component(ivdim));
        }
        else
        {
            child->bound = Rectangle(
                widget_get_content_bound(widget).position().component(ivdim),
                current,
                widget_get_content_bound(widget).size().component(ivdim),
                current_child_size);
        }

        current += current_child_size + spacing;
    }
}

void widget_layout(Widget *widget)
{
    if (widget->klass->layout)
    {
        widget->klass->layout(widget);
        return;
    }

    if (widget->childs->count() == 0)
        return;

    Layout layout = widget->layout;

    switch (layout.type)
    {
    case LAYOUT_STACK:
        list_foreach(Widget, child, widget->childs)
        {
            child->bound = widget_get_content_bound(widget);
        }
        break;
    case LAYOUT_GRID:
    {
        int originX = widget_get_content_bound(widget).x();
        int originY = widget_get_content_bound(widget).y();

        int child_width = (widget_get_content_bound(widget).width() - (layout.spacing.x() * (layout.hcell - 1))) / layout.hcell;
        int child_height = (widget_get_content_bound(widget).height() - (layout.spacing.y() * (layout.vcell - 1))) / layout.vcell;

        int index = 0;
        list_foreach(Widget, child, widget->childs)
        {
            int x = index % layout.hcell;
            int y = index / layout.hcell;

            child->bound = Rectangle(
                originX + x * (child_width + layout.spacing.x()),
                originY + y * (child_height + layout.spacing.y()),
                child_width,
                child_height);

            index++;
        }
    }
    break;

    case LAYOUT_HGRID:
        widget_do_vhgrid_layout(widget, layout, Dimension::X);
        break;

    case LAYOUT_VGRID:
        widget_do_vhgrid_layout(widget, layout, Dimension::Y);
        break;

    case LAYOUT_HFLOW:
    {
        int fixed_child_count = 0;
        int fixed_child_total_width = 0;

        int fill_child_count = 0;

        list_foreach(Widget, child, widget->childs)
        {
            if (child->layout_attributes & LAYOUT_FILL)
            {
                fill_child_count++;
            }
            else
            {
                fixed_child_count++;
                fixed_child_total_width += widget_compute_size(child).x();
            }
        }

        int usable_space =
            widget_get_content_bound(widget).width() -
            layout.spacing.x() * (widget->childs->count() - 1);

        int fill_child_total_width = MAX(0, usable_space - fixed_child_total_width);

        int fill_child_width = (fill_child_total_width) / MAX(1, fill_child_count);

        int current = widget_get_content_bound(widget).x();

        list_foreach(Widget, child, widget->childs)
        {
            if (child->layout_attributes & LAYOUT_FILL)
            {
                child->bound = Rectangle(
                    current,
                    widget_get_content_bound(widget).y(),
                    fill_child_width,
                    widget_get_content_bound(widget).height());

                current += fill_child_width + layout.spacing.x();
            }
            else
            {
                child->bound = Rectangle(
                    current,
                    widget_get_content_bound(widget).y(),
                    widget_compute_size(child).x(),
                    widget_get_content_bound(widget).height());

                current += widget_compute_size(child).x() + layout.spacing.x();
            }
        }
    }
    break;

    case LAYOUT_VFLOW:
    {
        int fixed_child_count = 0;
        int fixed_child_total_height = 0;

        int fill_child_count = 0;

        list_foreach(Widget, child, widget->childs)
        {
            if (child->layout_attributes & LAYOUT_FILL)
            {
                fill_child_count++;
            }
            else
            {
                fixed_child_count++;
                fixed_child_total_height += widget_compute_size(child).y();
            }
        }

        int usable_space =
            widget_get_content_bound(widget).height() -
            layout.spacing.y() * (widget->childs->count() - 1);

        int fill_child_total_height = MAX(0, usable_space - fixed_child_total_height);

        int fill_child_height = (fill_child_total_height) / MAX(1, fill_child_count);

        int current = widget_get_content_bound(widget).y();

        list_foreach(Widget, child, widget->childs)
        {
            if (child->layout_attributes & LAYOUT_FILL)
            {
                child->bound = Rectangle(
                    widget_get_content_bound(widget).x(),
                    current,
                    widget_get_content_bound(widget).width(),
                    fill_child_height);

                current += fill_child_height + layout.spacing.y();
            }
            else
            {
                child->bound = Rectangle(
                    widget_get_content_bound(widget).x(),
                    current,
                    widget_get_content_bound(widget).width(),
                    widget_compute_size(child).y());

                current += widget_compute_size(child).y() + layout.spacing.y();
            }
        }
    }
    break;

    default:
        break;
    }

    list_foreach(Widget, child, widget->childs)
    {
        widget_layout(child);
    }
}

void widget_set_focus(Widget *widget)
{
    if (widget->window)
    {
        window_set_focused_widget(widget->window, widget);
    }
}

Rectangle widget_get_bound(Widget *widget)
{
    return widget->bound;
}

Rectangle widget_get_content_bound(Widget *widget)
{
    return widget_get_bound(widget).shrinked(widget->insets);
}

Widget *widget_get_child_at(Widget *parent, Vec2i position)
{
    if (parent->layout_attributes & LAYOUT_GREEDY)
    {
        return parent;
    }

    list_foreach(Widget, child, parent->childs)
    {
        if (widget_get_bound(child).containe(position))
        {
            return widget_get_child_at(child, position);
        }
    }

    return parent;
}

Color widget_get_color(Widget *widget, ThemeColorRole role)
{
    if (!widget->enabled() || (widget->parent && !widget->parent->enabled()))
    {
        if (role == THEME_FOREGROUND)
        {
            role = THEME_FOREGROUND_INACTIVE;
        }

        if (role == THEME_SELECTION)
        {
            role = THEME_SELECTION_INACTIVE;
        }

        if (role == THEME_ACCENT)
        {
            role = THEME_ACCENT_INACTIVE;
        }
    }

    if (widget->colors[role].overwritten)
    {
        return widget->colors[role].color;
    }

    return window_get_color(widget->window, role);
}

void widget_set_color(Widget *widget, ThemeColorRole role, Color color)
{
    widget->colors[role].overwritten = true;
    widget->colors[role].color = color;

    widget->should_repaint();
}
