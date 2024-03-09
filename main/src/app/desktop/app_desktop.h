#ifndef APP_DESKTOP_H
#define APP_DESKTOP_H

#include "mini_app_defines.h"
#include "mini_app_registry.h"
#include "mui_include.h"
#include "mini_app_launcher.h"
#include "mui_list_view.h"
#include "desktop_view.h"
#include "mui_icons.h"
#include "mui_toast_view.h"
#include "ble_service.h"

typedef enum
{
    DESKTOP_VIEW_ID_MAIN,
    DESKTOP_VIEW_ID_TOAST
} desktop_view_id_t;

static void app_desktop_on_run(mini_app_inst_t *p_app_inst);
static void app_desktop_on_kill(mini_app_inst_t *p_app_inst);
static void app_desktop_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event);

typedef struct
{
    desktop_view_t *p_desktop_view;
    mui_view_dispatcher_t *p_view_dispatcher;
    mui_toast_view_t *p_toast_view;
    mui_view_dispatcher_t *p_view_dispatcher_toast;
} app_desktop_t;

extern const mini_app_t app_desktop_info;

#endif