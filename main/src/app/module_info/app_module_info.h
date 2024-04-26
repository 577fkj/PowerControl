#ifndef APP_MODULE_INFO_H
#define APP_MODULE_INFO_H

#include "mini_app_defines.h"
#include "mui_list_view.h"

typedef enum
{
    MODULE_INFO_VIEW_ID_MAIN
} module_info_view_id_t;

static void app_module_info_on_run(mini_app_inst_t *p_app_inst);
static void app_module_info_on_kill(mini_app_inst_t *p_app_inst);
static void app_module_info_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event);

typedef struct
{
    mui_list_view_t *p_list_view;
    mui_view_dispatcher_t *p_view_dispatcher;
} app_module_info_t;

extern const mini_app_t app_module_info_info;
#endif