#include <libaudio/AudioEngine.h>
#include <libsystem/Logger.h>
#include <string.h>
#include <libsystem/io/Connection.h>
#include <libsystem/io/Socket.h>
#include <libsystem/utils/Hexdump.h>
#include <libutils/Vector.h>

/* --- IPC ------------------------------------------------------------------ */

void AudioEngine::send_message(const MixerMessage &message)
{
    connection_send(_connection, &message, sizeof(MixerMessage));
}

void AudioEngine::do_message(const MixerMessage &message)
{
    __unused(message);
}

ResultOr<MixerMessage> AudioEngine::wait_for_message(MixerMessageType expected_message)
{
    Vector<MixerMessage> pendings;

    MixerMessage message{};
    connection_receive(_connection, &message, sizeof(MixerMessage));

    if (handle_has_error(_connection))
    {
        return handle_get_error(_connection);
    }

    while (message.type != expected_message)
    {
        pendings.push_back(move(message));
        connection_receive(_connection, &message, sizeof(MixerMessage));

        if (handle_has_error(_connection))
        {
            pendings.foreach ([&](auto &message) {
                do_message(message);
                return Iteration::CONTINUE;
            });

            return handle_get_error(_connection);
        }
    }

    pendings.foreach ([&](auto &message) {
        do_message(message);
        return Iteration::CONTINUE;
    });

    return message;
}

AudioEngine::AudioEngine()
{
    _connection = socket_connect("/Session/audio-mixer.ipc");

    if (handle_has_error(_connection))
    {
        logger_error("Failed to connect to the audio mixer: %s", handle_error_string(_connection));

        _error = handle_get_error(_connection);
        connection_close(_connection);
        _connection = nullptr;

        return;
    }

    EventLoop::initialize();

    _connection_notifier = own<Notifier>(HANDLE(_connection), POLL_READ, [this]() {
        MixerMessage message = {};
        memset((void *)&message, 0xff, sizeof(MixerMessage));
        size_t message_size = connection_receive(_connection, &message, sizeof(MixerMessage));

        if (handle_has_error(_connection))
        {
            logger_error("Connection to the audio-mixer closed %s!", handle_error_string(_connection));
            return;
        }

        if (message_size != sizeof(MixerMessage))
        {
            logger_error("Got a message with an invalid size from audio-mixer %u != %u!", sizeof(MixerMessage), message_size);
            hexdump(&message, message_size);
            return;
        }

        do_message(message);
    });
}

AudioBuffer *AudioEngine::create_buffer(int samplerate, int channels)
{
    auto &buffer = _buffers.emplace_back(own<AudioBuffer>(this, samplerate, channels));

    assert(buffer->id() >= 0);

    MixerMessage message = {
        .type = MIXER_MESSAGE_CREATE_BUFFER,
        .create_buffer = {
            .id = buffer->id(),
            .channels = channels,
            .samplerate = samplerate,
        },
    };

    send_message(message);

    return buffer.naked();
}

void AudioEngine::update_buffer(AudioBuffer *buffer, uint8_t *data, size_t size)
{
    assert(buffer->id() >= 0);
    assert(size <= AUDIO_DATA_MESSAGE_SIZE);

    MixerMessage message = {
        .type = MIXER_MESSAGE_UPDATE_BUFFER,
        .update_buffer = {
            .id = buffer->id(),
            .size = (int)size,
            .audiodata = { 0 }
        },
    };

    for (size_t i = 0; i < size; i++)
    {
        message.update_buffer.audiodata[i] = data[i];
    }

    send_message(message);
}