#pragma once
#include "mixer/Protocol.h"
#include <libsystem/eventloop/Notifier.h>
#include <libsystem/io/Connection.h>
#include <libutils/ResultOr.h>
#include <libutils/Vector.h>
#include <libaudio/AudioBuffer.h>

class AudioBuffer;
class AudioEngine
{
public:
    AudioEngine();

    AudioBuffer *create_buffer(int samplerate, int channels);

protected:
    void update_buffer(AudioBuffer *buffer, uint8_t *data, size_t size);

private:
    void send_message(const MixerMessage &);
    void do_message(const MixerMessage &);
    ResultOr<MixerMessage> wait_for_message(MixerMessageType expected_message);

    Result _error;
    Connection *_connection;
    OwnPtr<Notifier> _connection_notifier;
    Vector<OwnPtr<AudioBuffer>> _buffers;
    friend AudioBuffer;
};