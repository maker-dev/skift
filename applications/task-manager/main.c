#include <libsystem/System.h>
#include <libsystem/eventloop/Timer.h>
#include <libwidget/Application.h>
#include <libwidget/Widgets.h>

#include "task-manager/TaskModel.h"

typedef struct
{
    Window window;

    /// --- Graphs --- ///
    Widget *ram_graph;
    Timer *ram_timer;
    Widget *cpu_graph;
    Timer *cpu_timer;

    /// --- Table view --- //
    Widget *table;
    TaskModel *table_model;
    Timer *table_timer;
} TaskManagerWindow;

void widget_ram_update(Graph *widget)
{
    SystemStatus status = system_get_status();

    graph_record(widget, status.used_ram / (double)status.total_ram);
}

void widget_cpu_update(Graph *widget)
{
    SystemStatus status = system_get_status();

    graph_record(widget, status.cpu_usage / 100.0);
}

void widget_table_update(TaskManagerWindow *window)
{
    model_update((Model *)window->table_model);
    widget_update(window->table);
    widget_layout(window->table);
}

void task_manager_window_destroy(TaskManagerWindow *window)
{
    model_destroy((Model *)window->table_model);
}

int main(int argc, char **argv)
{
    application_initialize(argc, argv);

    TaskManagerWindow *window = __create(TaskManagerWindow);
    window_initialize((Window *)window, "memory", "Task Manager", 700, 500, WINDOW_RESIZABLE);
    window_root((Window *)window)->layout = VFLOW(0);

    /// --- Toolbar --- ///
    Widget *toolbar = toolbar_create(window_root((Window *)window));
    button_create_with_icon_and_text(toolbar, BUTTON_FILLED, "plus", "New task");
    button_create_with_icon_and_text(toolbar, BUTTON_TEXT, "close", "Cancel task");

    /// --- Table view --- //
    window->table_model = task_model_create();

    window->table = table_create(window_root((Window *)window), (Model *)window->table_model);
    window->table->layout_attributes = LAYOUT_FILL;
    window->table_timer = timer_create(window, 1000, (TimerCallback)widget_table_update);
    timer_start(window->table_timer);

    /// --- Graphs --- ///
    Widget *graphs_container = panel_create(window_root((Window *)window));
    graphs_container->layout = HFLOW(0);
    graphs_container->max_height = 96;

    window->cpu_graph = graph_create(graphs_container, 256, COLOR_SEAGREEN);
    window->cpu_graph->layout = VFLOW(8);
    window->cpu_graph->insets = INSETS(8);
    window->cpu_graph->layout_attributes = LAYOUT_FILL;

    Widget *cpu_icon_and_text = container_create(window->cpu_graph);
    cpu_icon_and_text->layout = HFLOW(4);
    icon_create(cpu_icon_and_text, "memory");
    label_create(cpu_icon_and_text, "Processor");

    window->cpu_timer = timer_create(window->cpu_graph, 100, (TimerCallback)widget_cpu_update);
    timer_start(window->cpu_timer);

    separator_create(graphs_container);

    window->ram_graph = graph_create(graphs_container, 256, COLOR_ROYALBLUE);
    window->ram_graph->layout = VFLOW(8);
    window->ram_graph->insets = INSETS(8);
    window->ram_graph->layout_attributes = LAYOUT_FILL;

    Widget *ram_icon_and_text = container_create(window->ram_graph);
    ram_icon_and_text->layout = HFLOW(4);
    icon_create(ram_icon_and_text, "chip");
    label_create(ram_icon_and_text, "Memory");

    window->ram_timer = timer_create(window->ram_graph, 500, (TimerCallback)widget_ram_update);
    timer_start(window->ram_timer);

    window_show((Window *)window);

    return application_run();
}