#include "mixer/AudioBuffer.h"

AudioBuffer::AudioBuffer(
    int id,
    struct Client *client,
    int samplerate,
    int channels) : _id(id), _client(client), _samplerate(samplerate), _channels(channels)
{
}