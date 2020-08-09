#include <libsystem/core/CString.h>
#include <libwidget/Menu.h>
#include <libwidget/Widgets.h>
#include <libwidget/Window.h>

struct Menu
{
    RefPtr<Icon> icon;
    char *text;
    List *items;
};

Menu *menu_create(Menu *parent, RefPtr<Icon> icon, const char *text)
{
    Menu *menu = __create(Menu);

    menu->icon = icon;
    menu->text = strdup(text);
    menu->items = list_create();

    if (parent)
    {
        list_pushback(parent->items, menu);
    }

    return menu;
}

void menu_destroy(Menu *menu)
{
    list_destroy_with_callback(menu->items, (ListDestroyElementCallback)menu_destroy);
    free(menu->text);
    free(menu);
}

void menu_show(Menu *menu)
{
    Window *window = window_create(WINDOW_POP_OVER | WINDOW_BORDERLESS);

    window_set_size(window, Vec2i(128, menu->items->count() * 32));

    window_root(window)->layout = VGRID(4);

    list_foreach(Menu, item, menu->items)
    {
        button_create_with_icon_and_text(window_root(window), BUTTON_TEXT, item->icon, item->text);
    }

    window_show(window);
}
