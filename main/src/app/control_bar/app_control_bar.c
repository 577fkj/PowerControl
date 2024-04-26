#include "app_control_bar.h"
#include "mini_app_registry.h"

#include "mui_include.h"

#include "control_bar_view.h"

#include "mini_app_launcher.h"
#include "mini_app_registry.h"

#include "mui_icons.h"

typedef enum
{
    CONTROL_BAR_VIEW_ID_MAIN
} control_bar_view_id_t;

static void app_control_bar_on_run(mini_app_inst_t *p_app_inst);
static void app_control_bar_on_kill(mini_app_inst_t *p_app_inst);
static void app_control_bar_on_event(mini_app_inst_t *p_app_inst,
                                     mini_app_event_t *p_event);

typedef struct
{
    control_bar_view_t *p_control_bar_view;
    mui_view_dispatcher_t *p_view_dispatcher;
} app_control_bar_t;

void app_control_bar_on_run(mini_app_inst_t *p_app_inst)
{

    app_control_bar_t *p_app_handle = mui_mem_malloc(sizeof(app_control_bar_t));

    p_app_inst->p_handle = p_app_handle;
    p_app_handle->p_view_dispatcher = mui_view_dispatcher_create();
    p_app_handle->p_control_bar_view = control_bar_view_create();

    mui_view_dispatcher_add_view(
        p_app_handle->p_view_dispatcher, CONTROL_BAR_VIEW_ID_MAIN,
        control_bar_view_get_view(p_app_handle->p_control_bar_view));
    mui_view_dispatcher_attach(p_app_handle->p_view_dispatcher, MUI_LAYER_CONTROL_BAR);
    mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher,
                                       CONTROL_BAR_VIEW_ID_MAIN);
}

void app_control_bar_on_kill(mini_app_inst_t *p_app_inst)
{
    app_control_bar_t *p_app_handle = p_app_inst->p_handle;

    mui_view_dispatcher_detach(p_app_handle->p_view_dispatcher, MUI_LAYER_CONTROL_BAR);
    mui_view_dispatcher_free(p_app_handle->p_view_dispatcher);
    control_bar_view_free(p_app_handle->p_control_bar_view);

    mui_mem_free(p_app_handle);

    p_app_inst->p_handle = NULL;
}

void app_control_bar_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event) {}

void app_control_bar_set_draw(void *user_data, control_bar_draw_cb_t draw)
{
    app_control_bar_t *app = mini_app_launcher_get_app_handle(mini_app_launcher(), MINI_APP_ID_CONTROL_BAR);
    app->p_control_bar_view->draw_cb = draw;
    app->p_control_bar_view->user_data = user_data;
    mui_update(mui());
}

control_bar_draw_cb_t app_control_bar_get_draw()
{
    app_control_bar_t *app = mini_app_launcher_get_app_handle(mini_app_launcher(), MINI_APP_ID_CONTROL_BAR);
    return app->p_control_bar_view->draw_cb;
}

void *app_control_bar_get_user_data()
{
    app_control_bar_t *app = mini_app_launcher_get_app_handle(mini_app_launcher(), MINI_APP_ID_CONTROL_BAR);
    return app->p_control_bar_view->user_data;
}

const mini_app_t app_control_bar_info = {.id = MINI_APP_ID_CONTROL_BAR,
                                         .name = "control bar",
                                         .icon = ICON_HOME,
                                         .deamon = true,
                                         .sys = true,
                                         .hide = true,
                                         .run_cb = app_control_bar_on_run,
                                         .kill_cb = app_control_bar_on_kill,
                                         .on_event_cb = app_control_bar_on_event};
