#include "control_bar_view.h"
#include "app_control_bar.h"

static void control_bar_view_on_draw(mui_view_t *p_view, mui_canvas_t *p_canvas)
{
    control_bar_view_t *view = p_view->user_data;
    if (view->draw_cb)
    {
        view->draw_cb(view->user_data, p_canvas);
    }
    mui_canvas_draw_line(p_canvas, 12, 0, 12, 64);
}

static void control_bar_view_on_input(mui_view_t *p_view, mui_input_event_t *event) {}

static void control_bar_view_on_enter(mui_view_t *p_view) {}

static void control_bar_view_on_exit(mui_view_t *p_view) {}

control_bar_view_t *control_bar_view_create()
{
    control_bar_view_t *p_control_bar_view = mui_mem_malloc(sizeof(control_bar_view_t));

    mui_view_t *p_view = mui_view_create();
    p_view->user_data = p_control_bar_view;
    p_view->draw_cb = control_bar_view_on_draw;
    p_view->input_cb = control_bar_view_on_input;
    p_view->enter_cb = control_bar_view_on_enter;
    p_view->exit_cb = control_bar_view_on_exit;

    p_control_bar_view->p_view = p_view;

    return p_control_bar_view;
}
void control_bar_view_free(control_bar_view_t *p_view)
{
    mui_view_free(p_view->p_view);
    mui_mem_free(p_view);
}
mui_view_t *control_bar_view_get_view(control_bar_view_t *p_view) { return p_view->p_view; }
