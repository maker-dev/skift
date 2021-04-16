#include "Manager.h"
#include "mixer/AudioBuffer.h"

Manager::Manager(Stream *sound_stream) : _sound_stream(sound_stream)
{
    _update_timer = own<Timer>(1000 / 20, [this]() {
        update();
    });
}

void Manager::update()
{
}

AudioBuffer *Manager::get_buffer(struct Client *client, int id)
{
    for (auto buffer : _buffers)
    {
        if (buffer->client() == client && buffer->id() == id)
        {
            return buffer;
        }
    }

    return nullptr;
}