#include "app_desktop.h"

void show_toast(mui_toast_view_t *p_toast_view, const char *message)
{
    mui_toast_view_show(p_toast_view, message);
}

void connected_callback(uint8_t *mac, void *user_data)
{
    app_desktop_t *p_app_handle = (app_desktop_t *)user_data;
    show_toast(p_app_handle->p_toast_view, "蓝牙已连接");
}

void disconnected_callback(uint8_t *mac, void *user_data)
{
    app_desktop_t *p_app_handle = (app_desktop_t *)user_data;
    show_toast(p_app_handle->p_toast_view, "蓝牙已断开");
}

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
    p_app_handle->p_toast_view = mui_toast_view_create();

    mui_toast_view_set_user_data(p_app_handle->p_toast_view, p_app_handle);

    p_app_handle->p_view_dispatcher_toast = mui_view_dispatcher_create();
    mui_view_dispatcher_add_view(p_app_handle->p_view_dispatcher_toast, DESKTOP_VIEW_ID_TOAST,
                                 mui_toast_view_get_view(p_app_handle->p_toast_view));
    mui_view_dispatcher_attach(p_app_handle->p_view_dispatcher_toast, MUI_LAYER_TOAST);

    mui_view_dispatcher_add_view(p_app_handle->p_view_dispatcher, DESKTOP_VIEW_ID_MAIN,
                                 mui_list_view_get_view(p_app_handle->p_desktop_view));
    mui_view_dispatcher_attach(p_app_handle->p_view_dispatcher, MUI_LAYER_DESKTOP);
    mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher, DESKTOP_VIEW_ID_MAIN);

    mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher_toast, DESKTOP_VIEW_ID_TOAST);

    set_ble_connected_callback(connected_callback, p_app_handle);
    set_ble_disconnect_callback(disconnected_callback, p_app_handle);
}

void app_desktop_on_kill(mini_app_inst_t *p_app_inst)
{
    app_desktop_t *p_app_handle = p_app_inst->p_handle;

    set_ble_connected_callback(NULL, NULL);
    set_ble_disconnect_callback(NULL, NULL);

    mui_toast_view_free(p_app_handle->p_toast_view);
    mui_view_dispatcher_detach(p_app_handle->p_view_dispatcher_toast, MUI_LAYER_TOAST);
    mui_view_dispatcher_free(p_app_handle->p_view_dispatcher_toast);

    mui_view_dispatcher_detach(p_app_handle->p_view_dispatcher, MUI_LAYER_DESKTOP);
    mui_view_dispatcher_free(p_app_handle->p_view_dispatcher);
    desktop_view_free(p_app_handle->p_desktop_view);

    mui_mem_free(p_app_handle);

    p_app_inst->p_handle = NULL;
}

void app_desktop_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event) {}

const mini_app_t app_desktop_info = {.id = MINI_APP_ID_DESKTOP,
                                     .name = "desktop",
                                     .icon = ICON_HOME,
                                     .deamon = false,
                                     .sys = true,
                                     .hide = true,
                                     .run_cb = app_desktop_on_run,
                                     .kill_cb = app_desktop_on_kill,
                                     .on_event_cb = app_desktop_on_event};
