#pragma once
#include <libutils/Vector.h>
#include <libsystem/eventloop/Timer.h>

struct AudioBuffer;
struct Client;
struct Stream;

class Manager
{
public:
    Manager(Stream *sound_stream);
    AudioBuffer *get_buffer(Client *client, int id);
    void add_buffer(AudioBuffer* buffer)
    {
        _buffers.push_back(buffer);
    }
private:
    void update();
    OwnPtr<Timer> _update_timer;
    Stream *_sound_stream;
    Vector<AudioBuffer *> _buffers;
};