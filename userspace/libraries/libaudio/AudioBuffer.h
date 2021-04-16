#pragma once
#include <libsystem/Common.h>

class AudioEngine;
class AudioBuffer
{
private:
    int _id = 0;
    AudioEngine* _engine;
    int _samplerate;
    int _channels;
public:
    AudioBuffer(AudioEngine* engine, int samplerate, int channels);
    
    inline int id() const
    {
        return _id;
    }

    void update(uint8_t* data, size_t size);
};