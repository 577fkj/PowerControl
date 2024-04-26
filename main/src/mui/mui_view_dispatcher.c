#include "mui_view_dispatcher.h"
#include "mui_core.h"

static void mui_view_dispatcher_set_curent_view(mui_view_dispatcher_t *p_dispatcher,
                                                mui_view_t *p_view)
{
    if (p_dispatcher->p_active_view)
    {
        mui_view_exit(p_dispatcher->p_active_view);
    }
    p_dispatcher->p_active_view = p_view;
    if (p_dispatcher->p_active_view)
    {
        mui_view_enter(p_dispatcher->p_active_view);
        mui_view_port_enable_set(p_dispatcher->p_view_port, true);
        mui_view_port_update(p_dispatcher->p_view_port);
    }
    else
    {
        mui_view_port_enable_set(p_dispatcher->p_view_port, false);
    }
}

static void mui_view_dispatcher_on_draw(mui_view_port_t *p_vp, mui_canvas_t *p_canvas)
{
    mui_view_dispatcher_t *p_dispatcher = p_vp->user_data;
    if (p_dispatcher->p_active_view)
    {
        p_dispatcher->p_active_view->draw_cb(p_dispatcher->p_active_view, p_canvas);
    }
}

static void mui_view_dispatcher_on_input(mui_view_port_t *p_vp,
                                         mui_input_event_t *p_event)
{
    mui_view_dispatcher_t *p_dispatcher = p_vp->user_data;
    if (p_dispatcher->p_active_view)
    {
        p_dispatcher->p_active_view->input_cb(p_dispatcher->p_active_view, p_event);
    }
}

mui_view_dispatcher_t *mui_view_dispatcher_create()
{
    mui_view_dispatcher_t *p_dsp = mui_mem_malloc(sizeof(mui_view_dispatcher_t));
    mui_view_dict_init(p_dsp->views);
    p_dsp->p_view_port = mui_view_port_create();
    p_dsp->p_view_port->draw_cb = mui_view_dispatcher_on_draw;
    p_dsp->p_view_port->input_cb = mui_view_dispatcher_on_input;
    p_dsp->p_view_port->user_data = p_dsp;
    p_dsp->p_active_view = NULL;

    return p_dsp;
}
void mui_view_dispatcher_free(mui_view_dispatcher_t *p_dispatcher)
{
    mui_view_dict_clear(p_dispatcher->views);
    mui_view_port_free(p_dispatcher->p_view_port);

    mui_mem_free(p_dispatcher);
}
void mui_view_dispatcher_add_view(mui_view_dispatcher_t *p_dispatcher, uint32_t view_id,
                                  mui_view_t *p_view)
{
    mui_view_dict_set_at(p_dispatcher->views, view_id, p_view);
}
void mui_view_dispatcher_remove_view(mui_view_dispatcher_t *p_dispatcher,
                                     uint32_t view_id)
{
    mui_view_t *p_view = *mui_view_dict_get(p_dispatcher->views, view_id);
    if (p_view == NULL)
    {
        return;
    }

    if (p_dispatcher->p_active_view == p_view)
    {
        mui_view_dispatcher_set_curent_view(p_dispatcher, NULL);
    }
}
void mui_view_dispatcher_attach(mui_view_dispatcher_t *p_dispatcher, mui_layer_t layer)
{
    mui_add_view_port(mui(), p_dispatcher->p_view_port, layer);
}
void mui_view_dispatcher_detach(mui_view_dispatcher_t *p_dispatcher, mui_layer_t layer)
{
    mui_view_port_enable_set(p_dispatcher->p_view_port, false);
    mui_remove_view_port(mui(), p_dispatcher->p_view_port, layer);
}
void mui_view_dispatcher_switch_to_view(mui_view_dispatcher_t *p_dispatcher,
                                        uint32_t view_id)
{
    if (view_id == VIEW_NONE)
    {
        mui_view_dispatcher_set_curent_view(p_dispatcher, NULL);
    }
    else
    {
        mui_view_t *p_view = *mui_view_dict_get(p_dispatcher->views, view_id);
        if (p_view)
        {
            mui_view_dispatcher_set_curent_view(p_dispatcher, p_view);
        }
    }
}