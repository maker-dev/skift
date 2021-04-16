#pragma once

#include <abi/Network.h>

struct IOCallTerminalSizeArgs
{
    int width;
    int height;
};

struct IOCallDisplayModeArgs
{
    int width;
    int height;
};

struct IOCallSoundStateArgs
{
    int sample_rate;
    int volume_PCM;
    int volume_master;
};
struct IOCallDisplayBlitArgs
{
    uint32_t *buffer;
    int buffer_width;
    int buffer_height;

    int blit_x;
    int blit_y;
    int blit_width;
    int blit_height;
};

struct IOCallKeyboardSetKeymapArgs
{
    void *keymap;
    size_t size;
};

struct IOCallTextModeStateArgs
{
    int width;
    int height;
    int cursor_x;
    int cursor_y;
};

struct IOCallNetworkSateAgs
{
    MacAddress mac_address;
};

enum IOCall
{
    IOCALL_TERMINAL_GET_SIZE,
    IOCALL_TERMINAL_SET_SIZE,

    IOCALL_DISPLAY_GET_MODE,
    IOCALL_DISPLAY_SET_MODE,
    IOCALL_DISPLAY_BLIT,

    IOCALL_KEYBOARD_SET_KEYMAP,
    IOCALL_KEYBOARD_GET_KEYMAP,

    IOCALL_TEXTMODE_GET_STATE,
    IOCALL_TEXTMODE_SET_STATE,

    IOCALL_NETWORK_GET_STATE,

    IOCALL_SOUND_GET_STATE,
    IOCALL_SOUND_SET_SAMPLERATE,

    __IOCALL_COUNT,
};
