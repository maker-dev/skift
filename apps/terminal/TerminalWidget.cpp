#include <libgraphic/Painter.h>
#include <libsystem/Logger.h>
#include <libsystem/process/Launchpad.h>
#include <libutils/Assert.h>
#include <libwidget/Event.h>
#include <libwidget/Window.h>

#include "terminal/Common.h"
#include "terminal/TerminalWidget.h"

#define TERMINAL_IO_BUFFER_SIZE 4096

TerminalWidget::TerminalWidget(Component *parent) : Component(parent)
{
    _terminal = own<Terminal::Terminal>(80, 24);

    _terminal_device = IO::Terminal::create().unwrap();

    _server_notifier = own<Notifier>(_terminal_device.server, POLL_READ, [this]() {
        handle_read();
    });

    _cursor_blink_timer = own<Timer>(250, [this]() {
        blink();

        int cx = _terminal->cursor().x;
        int cy = _terminal->cursor().y;

        should_repaint(cell_bound(cx, cy).offset(bound().position()));
    });

    _cursor_blink_timer->start();

    Launchpad *shell_launchpad = launchpad_create("shell", "/Applications/shell/shell");
    launchpad_handle(shell_launchpad, _terminal_device.client, 0);
    launchpad_handle(shell_launchpad, _terminal_device.client, 1);
    launchpad_handle(shell_launchpad, _terminal_device.client, 2);
    launchpad_launch(shell_launchpad, nullptr);
}

void TerminalWidget::paint(Graphic::Painter &painter, const Recti &dirty)
{
    for (int y = 0; y < _terminal->height(); y++)
    {
        for (int x = 0; x < _terminal->width(); x++)
        {
            Terminal::Cell cell = _terminal->cell_at(x, y);
            render_cell(painter, x, y, cell);
            _terminal->cell_undirty(x, y);
        }
    }

    int cx = _terminal->cursor().x;
    int cy = _terminal->cursor().y;

    if (cell_bound(cx, cy).colide_with(dirty))
    {
        Terminal::Cell cell = _terminal->cell_at(cx, cy);

        if (window()->focused())
        {
            if (_cursor_blink)
            {
                render_cell(
                    painter,
                    cx,
                    cy,
                    cell.codepoint,
                    Terminal::BACKGROUND,
                    Terminal::FOREGROUND,
                    Terminal::Attributes::defaults());
            }
            else
            {
                render_cell(painter, cx, cy, cell);
            }
        }
        else
        {
            render_cell(painter, cx, cy, cell);
            painter.draw_rectangle(cell_bound(cx, cy), color(Widget::THEME_ANSI_CURSOR));
        }
    }
}

void TerminalWidget::event(Widget::Event *event)
{
    auto send_sequence = [&](auto sequence) {
        _terminal_device.server.write(sequence, strlen(sequence));
        event->accepted = true;
    };

    if (event->type == Widget::Event::MOUSE_SCROLL)
    {
        int lines = event->mouse.scroll > 0 ? 1 : -1;
        _terminal->scroll(lines);
        should_repaint();
    }
    else if (event->type == Widget::Event::KEYBOARD_KEY_TYPED)
    {
        switch (event->keyboard.key)
        {
        case KEYBOARD_KEY_DELETE:
            send_sequence("\e[3~");
            break;

        case KEYBOARD_KEY_END:
            send_sequence("\e[4~");
            break;

        case KEYBOARD_KEY_F1:
            send_sequence("\e[11~");
            break;

        case KEYBOARD_KEY_F2:
            send_sequence("\e[12~");
            break;

        case KEYBOARD_KEY_F3:
            send_sequence("\e[13~");
            break;

        case KEYBOARD_KEY_F4:
            send_sequence("\e[14~");
            break;

        case KEYBOARD_KEY_F5:
            send_sequence("\e[15~");
            break;

        case KEYBOARD_KEY_F6:
            send_sequence("\e[17~");
            break;

        case KEYBOARD_KEY_F7:
            send_sequence("\e[18~");
            break;

        case KEYBOARD_KEY_F8:
            send_sequence("\e[19~");
            break;

        case KEYBOARD_KEY_F9:
            send_sequence("\e[20~");
            break;

        case KEYBOARD_KEY_F10:
            send_sequence("\e[21~");
            break;

        case KEYBOARD_KEY_F11:
            send_sequence("\e[23~");
            break;

        case KEYBOARD_KEY_F12:
            send_sequence("\e[24~");
            break;

        case KEYBOARD_KEY_HOME:
            send_sequence("\e[1~");
            break;

        case KEYBOARD_KEY_INSERT:
            send_sequence("\e[2~");
            break;

        case KEYBOARD_KEY_PGUP:
            send_sequence("\e[5~");
            break;

        case KEYBOARD_KEY_PGDOWN:
            send_sequence("\e[6~");
            break;

        case KEYBOARD_KEY_UP:
            send_sequence("\e[A");
            break;

        case KEYBOARD_KEY_DOWN:
            send_sequence("\e[B");
            break;

        case KEYBOARD_KEY_RIGHT:
            send_sequence("\e[C");
            break;

        case KEYBOARD_KEY_LEFT:
            send_sequence("\e[D");
            break;

        default:
            if (event->keyboard.codepoint != 0)
            {
                uint8_t buffer[5];
                int size = codepoint_to_utf8(event->keyboard.codepoint, buffer);
                _terminal_device.server.write(buffer, size);
                event->accepted = true;
            }
            break;
        }
    }
}

void TerminalWidget::do_layout()
{
    int width = bound().width() / cell_size().x();
    int height = bound().height() / cell_size().y();

    width = MAX(width, 8);
    height = MAX(height, 8);

    if (_terminal->width() != width || _terminal->height() != height)
    {
        _terminal->resize(width, height);

        IOCallTerminalSizeArgs args = {width, height};
        _terminal_device.server.handle()->call(IOCALL_TERMINAL_SET_SIZE, &args);
    }
}

void TerminalWidget::handle_read()
{
    char buffer[TERMINAL_IO_BUFFER_SIZE];
    auto read_result = _terminal_device.server.read(buffer, TERMINAL_IO_BUFFER_SIZE);

    if (!read_result.success())
    {
        logger_error("Terminal: read from server failed: %s", read_result.description());
        return;
    }

    _terminal->write(buffer, read_result.unwrap());
    should_repaint();
}