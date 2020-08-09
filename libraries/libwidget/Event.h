#pragma once

#include <abi/Keyboard.h>
#include <libgraphic/Shape.h>
#include <libsystem/unicode/Codepoint.h>
#include <libutils/Callback.h>

#define MOUSE_NO_BUTTON (0)
#define MOUSE_BUTTON_LEFT (1 << 1)
#define MOUSE_BUTTON_RIGHT (1 << 2)
#define MOUSE_BUTTON_MIDDLE (1 << 3)

typedef unsigned int MouseButton;

struct MouseEvent
{
    Vec2i position;
    Vec2i old_position;

    MouseButton button;
    MouseButton buttons;
};

struct KeyboardEvent
{
    Key key;
    Codepoint codepoint;
};

struct Event
{
    enum Type
    {
        VALUE_CHANGE,
        ACTION,

        GOT_FOCUS,
        LOST_FOCUS,

        WINDOW_CLOSING,

        MOUSE_MOVE,
        MOUSE_ENTER,
        MOUSE_LEAVE,

        MOUSE_BUTTON_PRESS,
        MOUSE_BUTTON_RELEASE,
        MOUSE_DOUBLE_CLICK,

        KEYBOARD_KEY_PRESS,
        KEYBOARD_KEY_RELEASE,
        KEYBOARD_KEY_TYPED,

        __COUNT,
    };

    Type type;
    bool accepted;

    MouseEvent mouse;
    KeyboardEvent keyboard;
};

using EventType = Event::Type;
using EventHandler = Callback<void(Event *)>;

#define is_mouse_event(__event)                                   \
    (((Event *)(__event))->type == Event::MOUSE_MOVE ||           \
     ((Event *)(__event))->type == Event::MOUSE_ENTER ||          \
     ((Event *)(__event))->type == Event::MOUSE_LEAVE ||          \
     ((Event *)(__event))->type == Event::MOUSE_BUTTON_PRESS ||   \
     ((Event *)(__event))->type == Event::MOUSE_BUTTON_RELEASE || \
     ((Event *)(__event))->type == Event::MOUSE_DOUBLE_CLICK)
