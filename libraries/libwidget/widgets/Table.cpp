#include <libgraphic/Painter.h>
#include <libwidget/Theme.h>
#include <libwidget/Window.h>
#include <libwidget/widgets/ScrollBar.h>
#include <libwidget/widgets/Table.h>

Rectangle Table::body_bound() const
{
    return content_bound();
}

Rectangle Table::header_bound() const
{
    return body_bound().take_top(TABLE_ROW_HEIGHT);
}

Rectangle Table::list_bound() const
{
    return body_bound().shrinked(Insets(TABLE_ROW_HEIGHT, 0, 0));
}

Rectangle Table::scrollbar_bound() const
{
    return list_bound().take_right(16);
}

Rectangle Table::row_bound(int row) const
{
    return Rectangle(
        list_bound().x(),
        list_bound().y() + row * TABLE_ROW_HEIGHT - _scroll_offset,
        list_bound().width(),
        TABLE_ROW_HEIGHT);
}

Rectangle Table::column_bound(int column) const
{
    int column_count = model_column_count(_model);
    int column_width = list_bound().width() / column_count;

    return Rectangle(
        list_bound().x() + column_width * column,
        list_bound().y(),
        column_width,
        list_bound().height());
}

Rectangle Table::cell_bound(int row, int column) const
{
    return Rectangle(
        row_bound(row).x() + column * column_bound(column).width(),
        row_bound(row).y(),
        column_bound(column).width(),
        row_bound(row).height());
}

int Table::row_at(Vec2i position) const
{
    if (!list_bound().contains(position))
    {
        return -1;
    }

    position = position - list_bound().position();

    int row = (position.y() + _scroll_offset) / TABLE_ROW_HEIGHT;

    if (row < 0 || row >= model_row_count(_model))
    {
        row = -1;
    }

    return row;
}

void Table::paint_cell(Painter &painter, int row, int column)
{
    Rectangle bound = cell_bound(row, column);
    Variant data = model_data(_model, row, column);

    painter.push();
    painter.clip(bound);

    if (data.has_icon())
    {
        painter.blit_icon(
            *data.icon(),
            ICON_18PX,
            Rectangle(bound.x() + 7, bound.y() + 7, 18, 18),
            color(THEME_FOREGROUND));

        painter.draw_string(
            *font(),
            data.as_string(),
            Vec2i(bound.x() + 7 + 18 + 7, bound.y() + 20),
            color(THEME_FOREGROUND));
    }
    else
    {
        painter.draw_string(
            *font(),
            data.as_string(),
            Vec2i(bound.x() + 7, bound.y() + 20),
            color(THEME_FOREGROUND));
    }

    painter.pop();
}

Table::Table(Widget *parent, Model *model) : Widget(parent)
{
    _model = model;
    _selected = -1;
    _scroll_offset = 0;

    _scrollbar = new ScrollBar(this);

    _scrollbar->on(Event::VALUE_CHANGE, [this](auto) {
        _scroll_offset = _scrollbar->value();
        should_repaint();
    });
}

void Table::paint(Painter &painter, Rectangle rectangle)
{
    __unused(rectangle);

    painter.push();
    painter.clip(bound());

    int column_count = model_column_count(_model);
    int column_width = body_bound().width() / column_count;

    if (model_row_count(_model) == 0)
    {
        painter.draw_string_within(*font(), _empty_message.cstring(), list_bound().take_top(TABLE_ROW_HEIGHT), Position::CENTER, color(THEME_FOREGROUND));
    }
    else
    {
        for (int row = MAX(0, _scroll_offset / TABLE_ROW_HEIGHT - 1);
             row < MIN(model_row_count(_model), ((_scroll_offset + list_bound().height()) / TABLE_ROW_HEIGHT) + 1);
             row++)
        {

            if (_selected == row)
            {
                painter.fill_rectangle(row_bound(row), color(THEME_SELECTION));
                painter.draw_rectangle(row_bound(row), color(THEME_SELECTION));
            }
            else if (row % 2)
            {
                painter.fill_rectangle(row_bound(row), color(THEME_FOREGROUND).with_alpha(0.05));
            }

            for (int column = 0; column < column_count; column++)
            {
                paint_cell(painter, row, column);
            }
        }
    }
    painter.blur_rectangle(header_bound(), 8);
    painter.fill_rectangle(header_bound(), color(THEME_BACKGROUND).with_alpha(0.9));

    for (int column = 0; column < column_count; column++)
    {
        Rectangle header_bound_cell = Rectangle(
            header_bound().x() + column * column_width,
            header_bound().y(),
            column_width,
            TABLE_ROW_HEIGHT);

        if (column < column_count - 1)
        {
            painter.fill_rectangle(header_bound_cell.take_right(1), color(THEME_BORDER));
        }

        painter.draw_string(*font(), model_column_name(_model, column), Vec2i(header_bound_cell.x() + 7, header_bound_cell.y() + 20), color(THEME_FOREGROUND));
        painter.draw_string(*font(), model_column_name(_model, column), Vec2i(header_bound_cell.x() + 7 + 1, header_bound_cell.y() + 20), color(THEME_FOREGROUND));
    }

    painter.pop();
}

void Table::event(Event *event)
{
    if (event->type == Event::MOUSE_BUTTON_PRESS)
    {
        _selected = row_at(event->mouse.position);
        should_repaint();
    }
}

void Table::do_layout()
{
    _scrollbar->bound(scrollbar_bound());
    _scrollbar->update(TABLE_ROW_HEIGHT * model_row_count(_model), list_bound().height(), _scroll_offset);
}
