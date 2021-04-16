#include <abi/Paths.h>

#include <assert.h>
#include <libsystem/Logger.h>
#include <libsystem/eventloop/EventLoop.h>
#include <libsystem/eventloop/Notifier.h>
#include <libsystem/eventloop/Timer.h>
#include <libsystem/io/Socket.h>
#include <libsystem/io/Stream.h>
#include <libsystem/process/Launchpad.h>
#include <libsystem/process/Process.h>

#include "Client.h"
#include "Manager.h"

Vector<Client *> client_list;
Stream *device;

bool acquire_lock()
{
    Stream *socket_stream = stream_open("/Session/audio-mixer.lock", OPEN_READ);
    if (!handle_has_error(socket_stream))
    {
        stream_close(socket_stream);
        return false;
    }
    else
    {
        Stream *lock_stream = stream_open("/Session/audio-mixer.lock", OPEN_READ | OPEN_CREATE);
        stream_close(lock_stream);
        return true;
    }
}

int main(int argc, char const *argv[])
{
    __unused(argc);
    __unused(argv);

    if (!acquire_lock())
    {
        stream_format(err_stream, "Audio mixer is already running.\n");
        return PROCESS_FAILURE;
    }

    EventLoop::initialize();

    Stream *sound_stream = stream_open(SOUND_DEVICE_PATH, OPEN_WRITE | OPEN_CREATE);

    Socket *socket = socket_open("/Session/audio-mixer.ipc", OPEN_CREATE);

    Manager manager(sound_stream);

    auto socker_notifier = own<Notifier>(HANDLE(socket), POLL_ACCEPT, [&]() {
        Connection *incoming_connection = socket_accept(socket);

        Client::connect(incoming_connection, &manager);

        client_destroy_disconnected();
    });

    return EventLoop::run();
}
