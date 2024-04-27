#include "app_module_info.h"
#include "mini_app_registry.h"

#include "mui_include.h"

#include "mini_app_launcher.h"
#include "mui_list_view.h"
#include "app_control_bar.h"

#include "power_protocol.h"

static void module_info_control_view_on_draw(void *user_data, mui_canvas_t *p_canvas)
{
    mui_canvas_set_font(p_canvas, u8g2_font_siji_t_6x10);
    mui_canvas_draw_glyph(p_canvas, 0, 14, 0xe060);
    mui_canvas_draw_glyph(p_canvas, 0, 35, 0xe069);
    mui_canvas_draw_glyph(p_canvas, 0, 57, 0xe061);
}

static void app_module_info_list_view_on_selected(mui_list_view_event_t event, mui_list_view_t *p_view, mui_list_item_t *p_item)
{
    mini_app_launcher_run(mini_app_launcher(), MINI_APP_ID_APP_LIST);
}

void app_module_info_on_run(mini_app_inst_t *p_app_inst)
{
    app_control_bar_set_draw(NULL, module_info_control_view_on_draw);

    app_module_info_t *p_app_handle = mui_mem_malloc(sizeof(app_module_info_t));

    p_app_inst->p_handle = p_app_handle;
    p_app_handle->p_view_dispatcher = mui_view_dispatcher_create();
    p_app_handle->p_list_view = mui_list_view_create();

    mui_list_view_set_selected_cb(p_app_handle->p_list_view, app_module_info_list_view_on_selected);

    mui_view_dispatcher_add_view(p_app_handle->p_view_dispatcher, MODULE_INFO_VIEW_ID_MAIN,
                                 mui_list_view_get_view(p_app_handle->p_list_view));
    mui_view_dispatcher_attach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher, MODULE_INFO_VIEW_ID_MAIN);

    power_protocol_app_t *power_protocol = get_current_power_protocol();
    if (power_protocol->draw_module_info)
    {
        power_protocol->draw_module_info(p_app_handle->p_list_view);
    }
    else
    {
        mui_list_view_add_item(p_app_handle->p_list_view, NULL, "此协议不支持此功能", NULL);
        mui_list_view_add_item(p_app_handle->p_list_view, NULL, "协议名称", NULL);
        mui_list_view_add_item(p_app_handle->p_list_view, NULL, power_protocol->name, NULL);
    }
}

void app_module_info_on_kill(mini_app_inst_t *p_app_inst)
{
    app_module_info_t *p_app_handle = p_app_inst->p_handle;

    mui_view_dispatcher_detach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_free(p_app_handle->p_view_dispatcher);

    mui_mem_free(p_app_handle);

    p_app_inst->p_handle = NULL;
}

void app_module_info_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event) {}

const mini_app_t app_module_info_info = {.id = MINI_APP_ID_MODULE_INFO,
                                         .name = "电源信息",
                                         .icon = 0xe1c5,
                                         .deamon = false,
                                         .sys = false,
                                         .hide = false,
                                         .run_cb = app_module_info_on_run,
                                         .kill_cb = app_module_info_on_kill,
                                         .on_event_cb = app_module_info_on_event};
