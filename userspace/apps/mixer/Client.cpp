#include <assert.h>
#include <libsystem/Logger.h>
#include <libsystem/system/Memory.h>
#include <libsystem/utils/Hexdump.h>
#include <libutils/Vector.h>

#include "mixer/AudioBuffer.h"
#include "mixer/Client.h"
#include "mixer/Manager.h"
#include "mixer/Protocol.h"

static Vector<OwnPtr<Client>> _clients;

void Client::handle(const MixerCreateBuffer &create_buffer)
{
    logger_info("MIXER: create buffer");
    if (_manager->get_buffer(this, create_buffer.id))
    {
        logger_error("Duplicated buffer id %d!", create_buffer.id);
        return;
    }

    _manager->add_buffer(new AudioBuffer(create_buffer.id, this, create_buffer.samplerate, create_buffer.channels));
}

void Client::handle(const MixerUpdateBuffer &update_buffer)
{
    logger_info("MIXER: update buffer");
    __unused(update_buffer);
}

void Client::handle(const MixerDeleteBuffer &delete_buffer)
{
    logger_info("MIXER: delete buffer");
    __unused(delete_buffer);
}

void Client::handle_goodbye()
{
    _disconnected = true;

    MixerMessage message = {};
    message.type = MIXER_MESSAGE_ACK;
    send_message(message);
}

void Client::handle_request()
{
    assert(!_disconnected);

    MixerMessage message = {};
    size_t message_size = connection_receive(_connection, &message, sizeof(MixerMessage));

    if (handle_has_error(_connection))
    {
        logger_error("Client handle has error: %s!", handle_error_string(_connection));

        _disconnected = true;
        client_destroy_disconnected();
        return;
    }

    if (message_size != sizeof(MixerMessage))
    {
        logger_error("Got a message with an invalid size from client %u != %u!", sizeof(MixerMessage), message_size);
        hexdump(&message, message_size);

        _disconnected = true;
        client_destroy_disconnected();
        return;
    }

    switch (message.type)
    {
    case MIXER_MESSAGE_CREATE_BUFFER:
        handle(message.create_buffer);
        break;

    case MIXER_MESSAGE_UPDATE_BUFFER:
        handle(message.update_buffer);
        break;

    case MIXER_MESSAGE_DELETE_BUFFER:
        handle(message.delete_buffer);
        break;

    case MIXER_MESSAGE_GOODBYE:
        handle_goodbye();
        break;

    default:
        logger_error("Invalid message for client %08x", this);
        hexdump(&message, message_size);

        _disconnected = true;
        client_destroy_disconnected();

        break;
    }
}

void Client::connect(Connection *connection, Manager *manager)
{
    _clients.push_back(own<Client>(connection, manager));
}

Client::Client(Connection *connection, Manager *manager) : _connection(connection), _manager(manager)
{
    _notifier = own<Notifier>(HANDLE(connection), POLL_READ, [this]() {
        this->handle_request();
    });

    logger_info("Client %08x connected", this);

    this->send_message((MixerMessage){
        .type = MIXER_MESSAGE_GREETINGS,
        .greetings = {
            .samplerate = 44100,
        },
    });
}

Client::~Client()
{
    logger_info("Disconnecting client %08x", this);

    connection_close(_connection);
}

void client_broadcast(MixerMessage message)
{
    _clients.foreach ([&](auto &client) {
        client->send_message(message);
        return Iteration::CONTINUE;
    });
}

Result Client::send_message(MixerMessage message)
{
    if (_disconnected)
    {
        return ERR_STREAM_CLOSED;
    }

    connection_send(_connection, &message, sizeof(MixerMessage));

    if (handle_has_error(_connection))
    {
        logger_error("Failed to send message to %08x: %s", this, handle_error_string(_connection));
        _disconnected = true;
        return handle_get_error(_connection);
    }

    return SUCCESS;
}

void client_destroy_disconnected()
{
    _clients.remove_all_match([](auto &client) { return client->_disconnected; });
}
