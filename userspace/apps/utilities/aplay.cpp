
#include <libaudio/AudioEngine.h>
#include <libsystem/io/File.h>
#include <libsystem/io/Filesystem.h>
#include <libsystem/io/Stream.h>
#include <libutils/ArgParse.h>
#include <libutils/Path.h>

int main(int argc, char const *argv[])
{
    ArgParse args;

    args.show_help_if_no_operand_given();
    args.should_abort_on_failure();

    args.usage("FILES...");
    args.usage("OPTION... FILES...");

    args.eval(argc, argv);

    AudioEngine audio_engine;

    // Play all files that were passed
    args.argv().foreach ([&](auto &path) {
        stream_format(out_stream, "%s: Play '%s'\n", argv[0], path.cstring());

        File file{path};
        if (!file.exist())
        {
            stream_format(err_stream, "%s: File does not exist '%s'\n", argv[0], path.cstring());
            process_exit(PROCESS_FAILURE);
            return Iteration::STOP;
        }

        auto buffer = audio_engine.create_buffer(44100, 2);
        __unused(buffer);
        
        return Iteration::CONTINUE;
    });


    return PROCESS_SUCCESS;
}
