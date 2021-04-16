#pragma once

#include <libsystem/eventloop/Notifier.h>
#include <libsystem/io/Connection.h>

#include "mixer/Protocol.h"

class Manager;
struct Client
{
    OwnPtr<Notifier> _notifier = nullptr;
    Connection *_connection = nullptr;
    Manager *_manager = nullptr;
    bool _disconnected = false;

    static void connect(Connection *connection, Manager *manager);

    Client(Connection *connection, Manager *manager);

    ~Client();

    Result send_message(MixerMessage message);

    void handle(const MixerCreateBuffer &create_buffer);

    void handle(const MixerUpdateBuffer &update_buffer);

    void handle(const MixerDeleteBuffer &delete_buffer);

    void handle_goodbye();

    void handle_request();
};

void client_broadcast(MixerMessage message);

void client_destroy_disconnected();
