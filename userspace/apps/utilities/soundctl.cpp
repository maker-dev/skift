#include <abi/Paths.h>

#include <libsystem/Result.h>
#include <libsystem/cmdline/CMDLine.h>
#include <libsystem/io/Connection.h>
#include <libsystem/io/File.h>
#include <libsystem/io/Socket.h>
#include <libsystem/io_new/Streams.h>

static bool option_list = false;
static bool option_get = false;
static char *option_set = nullptr;

/* --- Command line application initialization -------------------------------*/

static const char *usages[] = {
    "",
    "OPTION...",
    nullptr,
};

static CommandLineOption options[] = {
    COMMANDLINE_OPT_HELP,

    COMMANDLINE_OPT_BOOL("list", 'l', option_list, "List all available samplerates.", COMMANDLINE_NO_CALLBACK),
    COMMANDLINE_OPT_BOOL("get", 'g', option_get, "Get the current samplerate", COMMANDLINE_NO_CALLBACK),
    COMMANDLINE_OPT_STRING("set", 's', option_set, "Set samplerate.", COMMANDLINE_NO_CALLBACK),

    COMMANDLINE_OPT_END,
};

static CommandLine cmdline = CMDLINE(
    usages,
    options,
    "Get or set samplerate",
    "Options can be combined.");

struct SupportedMode
{
    const char *name;
    IOCallSoundStateArgs info;
};

SupportedMode sfxsrmodes[] = {
    {"22500", {22500, 0x0018, 0x0002}},
    {"44100", {44100, 0x0018, 0x0002}},
    {"48000", {48000, 0x0018, 0x0002}},

    {nullptr, {0, 0, 0}},
};

int sfxmode_srlist(Stream *sound_device)
{

    __unused(sound_device);

    for (int i = 0; sfxsrmodes[i].name; i++)
    {
        System::outln("{}\n", sfxsrmodes[i].name);
    }

    return 0;
}

int sfxmode_srget(Stream *sound_device)
{
    IOCallSoundStateArgs sound_state_info;

    if (stream_call(sound_device, IOCALL_SOUND_GET_STATE, &sound_state_info) < 0)
    {
        handle_printf_error(sound_device, "Ioctl to " SOUND_DEVICE_PATH " failed");
        return -1;
    }

    System::outln("Sample Rate: {}\n Volume PCM: {}\n Volume Master: {}\n",
                  sound_state_info.sample_rate,
                  sound_state_info.volume_PCM,
                  sound_state_info.volume_master);

    return 0;
}

IOCallSoundStateArgs *sfxsrmode_by_name(const char *name)
{
    for (int i = 0; sfxsrmodes[i].name; i++)
    {
        if (strcmp(sfxsrmodes[i].name, name) == 0)
        {
            return &sfxsrmodes[i].info;
        }
    }

    return nullptr;
}

int sfxmode_srset(Stream *device, const char *mode_name)
{
    IOCallSoundStateArgs *mode = sfxsrmode_by_name(mode_name);

    if (!mode)
    {
        System::outln("Error: unknown sound samplerate: {}\n", mode_name);
        return -1;
    }
    else
    {

        if (stream_call(device, IOCALL_SOUND_SET_SAMPLERATE, mode) != SUCCESS)
        {
            handle_printf_error(device, "Ioctl to " SOUND_DEVICE_PATH " failed");
            return -1;
        }

        return 0;
    }
}

/* --- Entry point ---------------------------------------------------------- */

int main(int argc, char **argv)
{
    argc = cmdline_parse(&cmdline, argc, argv);

    __cleanup(stream_cleanup) Stream *sound_device = stream_open(SOUND_DEVICE_PATH, OPEN_READ);

    if (handle_has_error(HANDLE(sound_device)))
    {
        handle_printf_error(sound_device, "soundctl: Failed to open " SOUND_DEVICE_PATH);
        return -1;
    }

    if (option_get)
    {
        return sfxmode_srget(sound_device);
    }
    else if (option_list)
    {
        return sfxmode_srlist(sound_device);
    }
    else if (option_set != nullptr)
    {
        return sfxmode_srset(sound_device, option_set);
    }
    else
    {
        return sfxmode_srget(sound_device);
    }
}
