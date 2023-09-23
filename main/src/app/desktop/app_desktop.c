#include "app_desktop.h"
#include "mini_app_registry.h"

#include "mui_include.h"

#include "mini_app_launcher.h"
#include "mui_list_view.h"

#include "desktop_view.h"

typedef enum
{
    DESKTOP_VIEW_ID_MAIN
} desktop_view_id_t;

static void app_desktop_on_run(mini_app_inst_t *p_app_inst);
static void app_desktop_on_kill(mini_app_inst_t *p_app_inst);
static void app_desktop_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event);

typedef struct
{
    desktop_view_t *p_desktop_view;
    mui_view_dispatcher_t *p_view_dispatcher;
} app_desktop_t;

static void app_desktop_list_view_on_selected(mui_list_view_event_t event, mui_list_view_t *p_view, mui_list_item_t *p_item)
{
    mini_app_launcher_run(mini_app_launcher(), (uint32_t)p_item->user_data);
}

void app_desktop_on_run(mini_app_inst_t *p_app_inst)
{

    app_desktop_t *p_app_handle = mui_mem_malloc(sizeof(app_desktop_t));

    p_app_inst->p_handle = p_app_handle;
    p_app_handle->p_view_dispatcher = mui_view_dispatcher_create();
    p_app_handle->p_desktop_view = desktop_view_create();

    mui_view_dispatcher_add_view(p_app_handle->p_view_dispatcher, DESKTOP_VIEW_ID_MAIN,
                                 mui_list_view_get_view(p_app_handle->p_desktop_view));
    mui_view_dispatcher_attach(p_app_handle->p_view_dispatcher, MUI_LAYER_DESKTOP);
    mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher, DESKTOP_VIEW_ID_MAIN);
}

void app_desktop_on_kill(mini_app_inst_t *p_app_inst)
{
    app_desktop_t *p_app_handle = p_app_inst->p_handle;

    mui_view_dispatcher_detach(p_app_handle->p_view_dispatcher, MUI_LAYER_DESKTOP);
    mui_view_dispatcher_free(p_app_handle->p_view_dispatcher);
    desktop_view_free(p_app_handle->p_desktop_view);

    mui_mem_free(p_app_handle);

    p_app_inst->p_handle = NULL;
}

void app_desktop_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event) {}

const mini_app_t app_desktop_info = {.id = MINI_APP_ID_DESKTOP,
                                     .name = "desktop",
                                     .icon = 0xe1f0,
                                     .deamon = false,
                                     .sys = true,
                                     .hide = true,
                                     .run_cb = app_desktop_on_run,
                                     .kill_cb = app_desktop_on_kill,
                                     .on_event_cb = app_desktop_on_event};
