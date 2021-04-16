#include <libaudio/AudioBuffer.h>
#include <libaudio/AudioEngine.h>

AudioBuffer::AudioBuffer(AudioEngine *engine, int samplerate, int channels) : _engine(engine), _samplerate(samplerate), _channels(channels)
{
    static int buffer_id_counter = 0;
    _id = buffer_id_counter++;
}

void AudioBuffer::update(uint8_t *data, size_t size)
{
    _engine->update_buffer(this, data, size);
}