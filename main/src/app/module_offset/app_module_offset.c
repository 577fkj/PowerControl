#include "app_module_offset.h"
#include "mini_app_registry.h"

#include "mui_include.h"

#include "module_offset_view.h"

#include "mini_app_launcher.h"
#include "mini_app_registry.h"

typedef enum
{
    MODULE_OFFSET_VIEW_ID_MAIN
} module_offset_view_id_t;

static void app_module_offset_on_run(mini_app_inst_t *p_app_inst);
static void app_module_offset_on_kill(mini_app_inst_t *p_app_inst);
static void app_module_offset_on_event(mini_app_inst_t *p_app_inst,
                                       mini_app_event_t *p_event);

typedef struct
{
    module_offset_view_t *p_module_offset_view;
    mui_view_dispatcher_t *p_view_dispatcher;
} app_module_offset_t;

void app_module_offset_on_run(mini_app_inst_t *p_app_inst)
{

    app_module_offset_t *p_app_handle = mui_mem_malloc(sizeof(app_module_offset_t));

    p_app_inst->p_handle = p_app_handle;
    p_app_handle->p_view_dispatcher = mui_view_dispatcher_create();
    p_app_handle->p_module_offset_view = module_offset_view_create();

    mui_view_dispatcher_add_view(
        p_app_handle->p_view_dispatcher, MODULE_OFFSET_VIEW_ID_MAIN,
        module_offset_view_get_view(p_app_handle->p_module_offset_view));
    mui_view_dispatcher_attach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher,
                                       MODULE_OFFSET_VIEW_ID_MAIN);
}

void app_module_offset_on_kill(mini_app_inst_t *p_app_inst)
{
    app_module_offset_t *p_app_handle = p_app_inst->p_handle;

    mui_view_dispatcher_detach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_free(p_app_handle->p_view_dispatcher);
    module_offset_view_free(p_app_handle->p_module_offset_view);

    mui_mem_free(p_app_handle);

    p_app_inst->p_handle = NULL;
}

void app_module_offset_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event) {}

const mini_app_t app_module_offset_info = {.id = MINI_APP_ID_MODULE_OFFSET,
                                           .name = "校准参数",
                                           .icon = 0xe1b0,
                                           .deamon = false,
                                           .sys = false,
                                           .hide = false,
                                           .run_cb = app_module_offset_on_run,
                                           .kill_cb = app_module_offset_on_kill,
                                           .on_event_cb = app_module_offset_on_event};
