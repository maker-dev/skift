#pragma once

#define AUDIO_DATA_MESSAGE_SIZE 0x4000

enum MixerMessageType
{
    MIXER_MESSAGE_INVALID,
    MIXER_MESSAGE_ACK,
    MIXER_MESSAGE_GREETINGS,
    MIXER_MESSAGE_GOODBYE,

    MIXER_MESSAGE_CREATE_BUFFER,
    MIXER_MESSAGE_UPDATE_BUFFER,
    MIXER_MESSAGE_DELETE_BUFFER,

    // TODO replace this with sources
    MIXER_MESSAGE_PLAY_BUFFER,
};

struct MixerGreetings
{
    int samplerate;
};

struct MixerCreateBuffer
{
    int id;
    int channels;
    int samplerate;
};

struct MixerUpdateBuffer
{
    int id;
    int size;
    char audiodata[AUDIO_DATA_MESSAGE_SIZE];
};

struct MixerDeleteBuffer
{
    int id;
};

struct MixerPlayBuffer
{
    int id;
}; 

struct MixerMessage
{
    MixerMessageType type;
    union
    {
        MixerGreetings greetings;
        MixerCreateBuffer create_buffer;
        MixerUpdateBuffer update_buffer;
        MixerDeleteBuffer delete_buffer;
    };
};
