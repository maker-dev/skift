#include <abi/Syscalls.h>

#include <libutils/ArgParse.h>

int main(int argc, char const *argv[])
{
    ArgParse args;

    args.should_abort_on_failure();
    args.show_help_if_no_option_given();

    args.prologue("Change the power state of the system.");

    args.option('r', "reboot", "Reboot the system.", [&](auto &) {
        hj_system_reboot();
        return ArgParseResult::ShouldFinish;
    });

    args.option('s', "shutdown", "Shutdown the system.", [&](auto &) {
        hj_system_shutdown();
        return ArgParseResult::ShouldFinish;
    });

    return args.eval(argc, argv) == ArgParseResult::Failure ? PROCESS_FAILURE : PROCESS_SUCCESS;
}
