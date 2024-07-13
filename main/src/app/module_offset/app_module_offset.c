#include "app_module_offset.h"
#include "mini_app_registry.h"

#include "mui_include.h"

#include "module_offset_view.h"

#include "mini_app_launcher.h"
#include "mini_app_registry.h"
#include "mui_list_view.h"
#include "mui_msg_box.h"
#include "app_control_bar.h"
#include "mui_icons.h"

#include "app_config.h"
#include "power_protocol.h"

static void module_offset_control_view_on_draw(void *user_data, mui_canvas_t *p_canvas)
{
    mui_canvas_set_font(p_canvas, u8g2_font_siji_t_6x10);
    mui_canvas_draw_glyph(p_canvas, 0, 14, 0xe060);
    mui_canvas_draw_glyph(p_canvas, 0, 35, 0xe105);
    mui_canvas_draw_glyph(p_canvas, 0, 57, 0xe061);
}

static void module_offset_msg_box_cb(mui_msg_box_event_t event, mui_msg_box_t *p_msg_box)
{
    module_offset_data_t *data = mui_msg_box_get_user_data(p_msg_box);
    app_module_offset_t *p_app_handle = data->user_data;

    if (event == MUI_MSG_BOX_EVENT_SELECT_CENTER || event == MUI_MSG_BOX_EVENT_SELECT_LEFT)
    {
        mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher, MODULE_OFFSET_LIST_VIEW_ID_MAIN);
        mui_msg_box_set_user_data(p_app_handle->p_msg_box, p_app_handle);
        mui_mem_free(data);
        return;
    }

    config_t *conf = get_config();
    power_protocol_app_t *power_protocol = get_current_power_protocol();
    power_protocol_data_t *power_data = power_protocol->get_data();
    switch (data->selection)
    {
    case 1:
        conf->set_offset_voltage = 1.0;
        conf->display_offset_voltage = 1.0;
        if (power_protocol->base_voltage_info.base_voltage > 0)
        {
            conf->set_voltage = power_protocol->base_voltage_info.base_voltage;
            power_protocol->set_online_voltage_current(power_protocol->base_voltage_info.base_voltage, conf->set_current);
        }

        p_app_handle->p_module_offset_view->count = 0;
        p_app_handle->p_module_offset_view->offset = power_data->output_voltage;
        break;

    case 2:
        conf->set_offset_current = 0.0;
        conf->display_offset_current = 0.0;

        p_app_handle->p_module_offset_view->count = 1;
        p_app_handle->p_module_offset_view->offset = power_data->output_current;
        break;
    }

    save_config(conf);

    mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher, MODULE_OFFSET_VIEW_ID_MAIN);
    mui_msg_box_set_user_data(p_app_handle->p_msg_box, p_app_handle);
    mui_mem_free(data);
}

static void module_offset_list_view_on_selected(mui_list_view_event_t event, mui_list_view_t *p_view, mui_list_item_t *p_item)
{
    app_module_offset_t *p_app_handle = p_view->user_data;

    if (p_item->user_data == 3)
    {
        mini_app_launcher_run(mini_app_launcher(), MINI_APP_ID_APP_LIST);
        return;
    }

    config_t *conf = get_config();
    power_protocol_app_t *power_protocol = get_current_power_protocol();
    power_protocol_data_t *power_data = power_protocol->get_data();

    mui_msg_box_set_header(p_app_handle->p_msg_box, "提示");
    mui_msg_box_set_event_cb(p_app_handle->p_msg_box, module_offset_msg_box_cb);

    module_offset_data_t *data = mui_mem_malloc(sizeof(module_offset_data_t));
    data->selection = p_item->user_data;
    data->user_data = p_app_handle;
    mui_msg_box_set_user_data(p_app_handle->p_msg_box, data);

    switch ((uint32_t)p_item->user_data)
    {
    case 1:
        if (power_data->output_voltage < 10 || power_data->output_current > 0)
        {
            mui_msg_box_set_message(p_app_handle->p_msg_box, "电压校正时输出必须大于10V且输出电流\n为0A");
            mui_msg_box_set_btn_focus(p_app_handle->p_msg_box, 1);
            mui_msg_box_set_btn_text(p_app_handle->p_msg_box, NULL, "确定", NULL);
        }
        else
        {
            mui_msg_box_set_message(p_app_handle->p_msg_box, "确定要校准电压吗？");
            mui_msg_box_set_btn_focus(p_app_handle->p_msg_box, 0);
            mui_msg_box_set_btn_text(p_app_handle->p_msg_box, "返回", NULL, "确定");
        }
        mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher, MODULE_OFFSET_VIEW_ID_MSG_BOX);
        break;
    case 2:
        if (power_data->output_current < 5 || conf->set_voltage - power_data->output_voltage < 0.2)
        {
            mui_msg_box_set_message(p_app_handle->p_msg_box, "电流校正时输出必须大于5A且处于恒流状态");
            mui_msg_box_set_btn_focus(p_app_handle->p_msg_box, 1);
            mui_msg_box_set_btn_text(p_app_handle->p_msg_box, NULL, "确定", NULL);
        }
        else
        {
            mui_msg_box_set_message(p_app_handle->p_msg_box, "确定要校准电流吗？");
            mui_msg_box_set_btn_focus(p_app_handle->p_msg_box, 0);
            mui_msg_box_set_btn_text(p_app_handle->p_msg_box, "返回", NULL, "确定");
        }
        mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher, MODULE_OFFSET_VIEW_ID_MSG_BOX);
        break;
    }
}

void app_module_offset_on_run(mini_app_inst_t *p_app_inst)
{
    app_control_bar_set_draw(NULL, module_offset_control_view_on_draw);

    app_module_offset_t *p_app_handle = mui_mem_malloc(sizeof(app_module_offset_t));

    p_app_inst->p_handle = p_app_handle;
    p_app_handle->p_view_dispatcher = mui_view_dispatcher_create();

    // offset view
    p_app_handle->p_module_offset_view = module_offset_view_create();
    p_app_handle->p_module_offset_view->user_data = p_app_handle;

    // list view
    p_app_handle->p_list_view = mui_list_view_create();
    mui_list_view_set_user_data(p_app_handle->p_list_view, p_app_handle);

    // msg box
    p_app_handle->p_msg_box = mui_msg_box_create();
    mui_msg_box_set_user_data(p_app_handle->p_msg_box, p_app_handle);

    mui_list_view_add_item(p_app_handle->p_list_view, 0xe040, "校准电压", 1);
    mui_list_view_add_item(p_app_handle->p_list_view, 0xe040, "校准电流", 2);

    mui_list_view_add_item(p_app_handle->p_list_view, ICON_HOME, "返回主页", 3);
    mui_list_view_set_selected_cb(p_app_handle->p_list_view, module_offset_list_view_on_selected);

    // add list view
    mui_view_dispatcher_add_view(
        p_app_handle->p_view_dispatcher, MODULE_OFFSET_LIST_VIEW_ID_MAIN,
        mui_list_view_get_view(p_app_handle->p_list_view));

    // add offset view
    mui_view_dispatcher_add_view(
        p_app_handle->p_view_dispatcher, MODULE_OFFSET_VIEW_ID_MAIN,
        module_offset_view_get_view(p_app_handle->p_module_offset_view));

    // add msg box
    mui_view_dispatcher_add_view(
        p_app_handle->p_view_dispatcher, MODULE_OFFSET_VIEW_ID_MSG_BOX,
        mui_msg_box_get_view(p_app_handle->p_msg_box));

    mui_view_dispatcher_attach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher,
                                       MODULE_OFFSET_LIST_VIEW_ID_MAIN);
}

void app_module_offset_on_kill(mini_app_inst_t *p_app_inst)
{
    app_module_offset_t *p_app_handle = p_app_inst->p_handle;

    mui_view_dispatcher_detach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_free(p_app_handle->p_view_dispatcher);
    module_offset_view_free(p_app_handle->p_module_offset_view);
    mui_list_view_free(p_app_handle->p_list_view);
    mui_msg_box_free(p_app_handle->p_msg_box);

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
