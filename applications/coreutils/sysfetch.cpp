#include <libsystem/io/Path.h>
#include <libsystem/io/Stream.h>
#include <libsystem/system/System.h>

int main(int argc, char **argv)
{
    __unused(argc);
    __unused(argv);

    // FIXME: - get the uptime from the kernel.
    //        - get user and the machine name from the system

    SystemInfo info = system_get_info();
    SystemStatus status = system_get_status();

    printf("\e[1;94m");
    printf("       ___      \n");
    printf("      /\\  \\     \n");
    printf("     /::\\  \\    \n");
    printf("    /:/\\ \\  \\   \n");
    printf("   _\\:\\~\\ \\  \\  \n");
    printf("  /\\ \\:\\ \\ \\__\\ \n");
    printf("  \\:\\ \\:\\ \\/__/ \n");
    printf("   \\:\\ \\:\\__\\   \n");
    printf("    \\:\\/:/  /   \n");
    printf("     \\::/  /    \n");
    printf("      \\/__/     \n");
    printf("\e[m");

    printf("\e[11A");
    printf("\e[16C user@%s\n", info.machine);
    printf("\e[16C ------------\n");
    printf("\e[16C OS: %sOS\n", info.system_name);
    printf("\e[16C KERNEL: %s\n", info.kernel_name);
    printf("\e[16C VERSION: %s\n", info.kernel_release);
    printf("\e[16C BUILD: %s\n", info.kernel_build);
    printf("\e[16C UPTIME: ");

    ElapsedTime seconds = status.uptime;

    if (seconds / 86400 > 0)
    {
        printf("%d day%s, ", seconds / 86400, (seconds / 86400) == 1 ? "" : "s");
        seconds %= 86400;
    }

    if (seconds / 3600 > 0)
    {
        printf("%d hour%s, ", seconds / 3600, (seconds / 3600) == 1 ? "" : "s");
        seconds %= 3600;
    }

    if (seconds / 60 > 0)
    {
        printf("%d minute%s, ", seconds / 60, (seconds / 60) == 1 ? "" : "s");
        seconds %= 60;
    }

    printf("%d second%s\n", seconds, seconds == 1 ? "" : "s");

    printf("\e[16C SHELL: /Applications/shell\n");
    printf("\e[16C TERMINAL: /Applications/terminal\n");
    printf("\e[16C COMPOSITOR: /Applications/compositor\n");
    printf("\e[16C MEMORY: \e[m%dMib / %dMib\n", status.used_ram / (1024 * 1024), status.total_ram / (1024 * 1024));

    printf("\n");
    printf("\e[16C \e[40m  \e[41m  \e[42m  \e[43m  \e[44m  \e[45m  \e[46m  \e[47m  \e[m\n");
    printf("\n");

    return 0;
}
