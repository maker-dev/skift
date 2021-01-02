#include <abi/Syscalls.h>

#include <libwidget/Application.h>
#include <libwidget/Markup.h>
#include <libwidget/Screen.h>
#include <libwidget/Widgets.h>
#include <libwidget/widgets/PaginationDots.h>

int main(int argc, char **argv)
{
    Application::initialize(argc, argv);

    Window *window = new Window(WINDOW_BORDERLESS | WINDOW_ALWAYS_FOCUSED | WINDOW_TRANSPARENT);

    window->title("Onboarding");
    window->type(WINDOW_TYPE_PANEL);
    window->bound(Screen::bound());
    window->opacity(0);
    window->show();
    window->root()->layout(STACK());

    auto background = new Panel(window->root());

    background->layout(STACK());
    background->color(THEME_MIDDLEGROUND, Colors::BLACK.with_alpha(0.5));

    auto dialog = new Panel(background);

    dialog->max_width(400);
    dialog->max_height(400);
    dialog->layout(VFLOW(0));
    dialog->border_radius(6);

    auto illustration = new Panel(dialog);

    illustration->max_height(160);
    illustration->min_height(160);
    illustration->border_radius(6);
    illustration->color(THEME_MIDDLEGROUND, Colors::WHITE);

    auto image = new Image(illustration, Bitmap::placeholder());

    auto content = new Container(dialog);
    content->flags(Widget::FILL);
    content->insets(16);

    auto dots_container = new Container(dialog);
    dots_container->insets(16);

    auto dots = new PaginationDots(dots_container, 5);

    auto navigation = new Container(dialog);

    navigation->layout(HFLOW(4));
    navigation->insets(8);

    auto skipall_button = new Button(navigation, BUTTON_TEXT, "Skip All");

    new Spacer(navigation);

    auto back_button = new Button(navigation, BUTTON_OUTLINE, "Previous");

    auto next_button = new Button(navigation, BUTTON_FILLED, "Next");

    int current_page = 0;

    auto set_current_page = [&](int index) {
        current_page = index;

        skipall_button->enable_if(current_page < 4);
        back_button->enable_if(current_page > 0);
        dots->index(index);

        char image_path[PATH_LENGTH];
        snprintf(image_path, PATH_LENGTH, "/Applications/onboarding/illustration%d.png", index);

        char content_path[PATH_LENGTH];
        snprintf(content_path, PATH_LENGTH, "/Applications/onboarding/content%d.markup", index);

        content->clear_children();
        image->change_bitmap(Bitmap::load_from(image_path).value());
        widget_create_from_file(content, content_path);
    };

    set_current_page(0);

    skipall_button->on(Event::ACTION, [](auto) {
        Application::exit(PROCESS_SUCCESS);
    });

    back_button->on(Event::ACTION, [&](auto) {
        if (current_page > 0)
        {
            set_current_page(current_page - 1);
        }
    });

    next_button->on(Event::ACTION, [&](auto) {
        if (current_page == 4)
        {
            Application::exit(PROCESS_SUCCESS);
        }
        else
        {
            set_current_page(current_page + 1);
        }
    });

    window->on(Event::KEYBOARD_KEY_PRESS, [&](Event* event){
        if(event->keyboard.key == KEYBOARD_KEY_ESC){
            Application::exit(PROCESS_SUCCESS);
        }else if(event->keyboard.key == KEYBOARD_KEY_RIGHT){
            if (current_page != 4)
            {
                set_current_page(current_page + 1);
            }
        }else if(event->keyboard.key == KEYBOARD_KEY_LEFT){
            if (current_page != 0)
            {
                set_current_page(current_page - 1);
            }
        }
    });

    return Application::run();
}
