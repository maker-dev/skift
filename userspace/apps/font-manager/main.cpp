#include <libwidget/Application.h>

#include "font-manager/FontManagerWindow.h"

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : "";
    auto window = new FontManagerWindow(path);
    window->show();

    return Application::the().run();
}
