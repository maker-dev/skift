#include <libsystem/Assert.h>
#include <libsystem/Logger.h>
#include <libsystem/utils/Vector.h>

#include "paint/PaintDocument.h"
#include "paint/PaintTool.h"

void pencil_tool_mouse_event(PaintTool *tool, PaintDocument *document, Event event)
{
    __unused(tool);

    if (event.type == EVENT_MOUSE_MOVE || event.type == EVENT_MOUSE_BUTTON_PRESS)
    {
        Vec2i from = event.mouse.old_position;
        Vec2i to = event.mouse.position;

        if (event.mouse.buttons & MOUSE_BUTTON_LEFT)
        {
            painter_draw_line(document->painter, from, to, document->primary_color);
            document->dirty = true;
        }
        else if (event.mouse.buttons & MOUSE_BUTTON_RIGHT)
        {
            painter_draw_line(document->painter, from, to, document->secondary_color);
            document->dirty = true;
        }
    }
}

PaintTool *pencil_tool_create(void)
{
    PaintTool *tool = __create(PaintTool);

    tool->on_mouse_event = pencil_tool_mouse_event;

    return tool;
}

void brush_tool_mouse_event(PaintTool *tool, PaintDocument *document, Event event)
{
    __unused(tool);

    if (event.type == EVENT_MOUSE_MOVE || event.type == EVENT_MOUSE_BUTTON_PRESS)
    {
        if (event.mouse.buttons & MOUSE_BUTTON_LEFT)
        {
            painter_fill_rectangle(
                document->painter,
                RECTANGLE(
                    event.mouse.position.x - 16,
                    event.mouse.position.y - 16,
                    32,
                    32),
                document->primary_color);

            document->dirty = true;
        }
        else if (event.mouse.buttons & MOUSE_BUTTON_RIGHT)
        {
            painter_fill_rectangle(
                document->painter,
                RECTANGLE(
                    event.mouse.position.x - 16,
                    event.mouse.position.y - 16,
                    32,
                    32),
                document->secondary_color);

            document->dirty = true;
        }
    }
}

PaintTool *brush_tool_create(void)
{
    PaintTool *tool = __create(PaintTool);

    tool->on_mouse_event = brush_tool_mouse_event;

    return tool;
}

void eraser_tool_mouse_event(PaintTool *tool, PaintDocument *document, Event event)
{
    __unused(tool);

    if (event.type == EVENT_MOUSE_MOVE || event.type == EVENT_MOUSE_BUTTON_PRESS)
    {
        if (event.mouse.buttons & MOUSE_BUTTON_LEFT)
        {
            painter_clear_rectangle(
                document->painter,
                RECTANGLE(
                    event.mouse.position.x - 16,
                    event.mouse.position.y - 16,
                    32,
                    32),
                COLOR_RGBA(0, 0, 0, 0));

            document->dirty = true;
        }
        else if (event.mouse.buttons & MOUSE_BUTTON_RIGHT)
        {
            painter_clear_rectangle(
                document->painter,
                RECTANGLE(
                    event.mouse.position.x - 16,
                    event.mouse.position.y - 16,
                    32,
                    32),
                document->secondary_color);

            document->dirty = true;
        }
    }
}

PaintTool *eraser_tool_create(void)
{
    PaintTool *tool = __create(PaintTool);

    tool->on_mouse_event = eraser_tool_mouse_event;

    return tool;
}

static void flood_fill(Bitmap *bitmap, Vec2i position, Color target, Color fill)
{

    if (!rectangle_containe_point(bitmap_bound(bitmap), position))
    {
        return;
    }

    if (color_equals(bitmap_get_pixel(bitmap, position), fill))
    {
        return;
    }

    if (!color_equals(bitmap_get_pixel(bitmap, position), target))
    {
        return;
    }

    Vector *queue = vector_create(sizeof(Vec2i), 256);
    vector_pushback(queue, &position);

    while (!vector_empty(queue))
    {
        Vec2i current = {};
        vector_popback(queue, &current);

        if (!color_equals(bitmap_get_pixel(bitmap, current), target))
        {
            continue;
        }

        bitmap_set_pixel(bitmap, current, fill);

        if (current.x != 0)
        {
            Vec2i new_position = vec2i_add(current, vec2i(-1, 0));
            vector_pushback(queue, &new_position);
        }

        if (current.x != bitmap_bound(bitmap).width - 1)
        {
            Vec2i new_position = vec2i_add(current, vec2i(1, 0));
            vector_pushback(queue, &new_position);
        }

        if (current.y != 0)
        {
            Vec2i new_position = vec2i_add(current, vec2i(0, -1));
            vector_pushback(queue, &new_position);
        }

        if (current.y != bitmap_bound(bitmap).height - 1)
        {
            Vec2i new_position = vec2i_add(current, vec2i(0, 1));
            vector_pushback(queue, &new_position);
        }
    }

    vector_destroy(queue);
}

void fill_tool_mouse_event(PaintTool *tool, PaintDocument *document, Event event)
{
    __unused(tool);

    if (event.type == EVENT_MOUSE_BUTTON_PRESS)
    {
        Color fill_color = COLOR_MAGENTA;

        if (event.mouse.buttons & MOUSE_BUTTON_LEFT)
        {
            fill_color = document->primary_color;
        }
        else if (event.mouse.buttons & MOUSE_BUTTON_RIGHT)
        {
            fill_color = document->secondary_color;
        }
        else
        {
            return;
        }

        Color target_color = bitmap_get_pixel(document->bitmap, event.mouse.position);

        flood_fill(document->bitmap, event.mouse.position, target_color, fill_color);

        document->dirty = true;
    }
}

PaintTool *fill_tool_create(void)
{
    PaintTool *tool = __create(PaintTool);

    tool->on_mouse_event = fill_tool_mouse_event;

    return tool;
}

void picker_tool_mouse_event(PaintTool *tool, PaintDocument *document, Event event)
{
    __unused(tool);

    if (event.type == EVENT_MOUSE_BUTTON_PRESS)
    {
        if (event.mouse.buttons & MOUSE_BUTTON_LEFT)
        {
            document->primary_color = bitmap_get_pixel(document->bitmap, event.mouse.position);
        }
        else if (event.mouse.buttons & MOUSE_BUTTON_RIGHT)
        {
            document->secondary_color = bitmap_get_pixel(document->bitmap, event.mouse.position);
        }
    }
}

PaintTool *picker_tool_create(void)
{
    PaintTool *tool = __create(PaintTool);

    tool->on_mouse_event = picker_tool_mouse_event;

    return tool;
}
