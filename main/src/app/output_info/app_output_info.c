#include "app_output_info.h"
#include "mini_app_registry.h"

#include "mui_include.h"

#include "mini_app_launcher.h"
#include "mui_list_view.h"
#include "app_control_bar.h"

#include "huawei_r48xx.h"

#include "driver/gptimer.h"
#include "esp_timer.h"

#include "app_config.h"

typedef enum
{
    OUTPUT_INFO_VIEW_ID_MAIN
} output_info_view_id_t;

static void app_output_info_on_run(mini_app_inst_t *p_app_inst);
static void app_output_info_on_kill(mini_app_inst_t *p_app_inst);
static void app_output_info_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event);

typedef struct
{
    mui_list_view_t *p_list_view;
    mui_view_dispatcher_t *p_view_dispatcher;
    esp_timer_handle_t data_timer_handle; // 定时器句柄
} app_output_info_t;

static void output_info_control_view_on_draw(void *user_data, mui_canvas_t *p_canvas)
{
    mui_canvas_set_font(p_canvas, u8g2_font_siji_t_6x10);
    mui_canvas_draw_glyph(p_canvas, 0, 14, 0xe060);
    mui_canvas_draw_glyph(p_canvas, 0, 35, 0xe069);
    mui_canvas_draw_glyph(p_canvas, 0, 57, 0xe061);
}

static void app_output_info_list_view_on_selected(mui_list_view_event_t event, mui_list_view_t *p_view, mui_list_item_t *p_item)
{
    mini_app_launcher_run(mini_app_launcher(), MINI_APP_ID_APP_LIST);
}

static void app_output_info_on_draw(mui_list_view_t *p_list_view)
{
    ConfigStruct *config = get_config();
    uint16_t force = mui_list_view_get_focus(p_list_view);
    uint32_t offset = p_list_view->scroll_offset;
    mui_list_view_clear_items(p_list_view);
    char buffer[32];
    mui_list_view_add_item(p_list_view, 0x0, "设置电压", NULL);
    sprintf(buffer, "%.2fV", config->set_voltage);
    mui_list_view_add_item(p_list_view, 0x0, buffer, NULL);

    mui_list_view_add_item(p_list_view, 0x0, "输出电压", NULL);
    sprintf(buffer, "%.2fV", power_data.output_voltage);
    mui_list_view_add_item(p_list_view, 0x0, buffer, NULL);

    mui_list_view_add_item(p_list_view, 0x0, "设置电流", NULL);
    sprintf(buffer, "%.2fA", config->set_current);
    mui_list_view_add_item(p_list_view, 0x0, buffer, NULL);

    mui_list_view_add_item(p_list_view, 0x0, "输出电流", NULL);
    sprintf(buffer, "%.2fA", power_data.output_current);
    mui_list_view_add_item(p_list_view, 0x0, buffer, NULL);

    mui_list_view_add_item(p_list_view, 0x0, "输出功率", NULL);
    sprintf(buffer, "%.2fW", power_data.output_power);
    mui_list_view_add_item(p_list_view, 0x0, buffer, NULL);

    mui_list_view_add_item(p_list_view, 0x0, "输出温度", NULL);
    sprintf(buffer, "%.2f°", power_data.output_temp);
    mui_list_view_add_item(p_list_view, 0x0, buffer, NULL);

    mui_list_view_add_item(p_list_view, 0x0, "转换率", NULL);
    sprintf(buffer, "%.2f%%", power_data.efficiency);
    mui_list_view_add_item(p_list_view, 0x0, buffer, NULL);

    mui_list_view_add_item(p_list_view, 0x0, "限流点", NULL);
    sprintf(buffer, "%.2f%%", power_data.current_limit);
    mui_list_view_add_item(p_list_view, 0x0, buffer, NULL);

    mui_list_view_add_item(p_list_view, 0x0, "输出状态", NULL);
    mui_list_view_add_item(p_list_view, 0x0, power_data.output_status ? "开启" : "关闭 & 启动中", NULL);

    mui_list_view_set_focus(p_list_view, force);
    p_list_view->scroll_offset = offset;

    mui_update(mui());
}

void app_output_info_on_run(mini_app_inst_t *p_app_inst)
{
    app_control_bar_set_draw(NULL, output_info_control_view_on_draw);

    app_output_info_t *p_app_handle = mui_mem_malloc(sizeof(app_output_info_t));

    p_app_inst->p_handle = p_app_handle;
    p_app_handle->p_view_dispatcher = mui_view_dispatcher_create();
    p_app_handle->p_list_view = mui_list_view_create();

    app_output_info_on_draw(p_app_handle->p_list_view);

    mui_list_view_set_selected_cb(p_app_handle->p_list_view, app_output_info_list_view_on_selected);

    mui_view_dispatcher_add_view(p_app_handle->p_view_dispatcher, OUTPUT_INFO_VIEW_ID_MAIN,
                                 mui_list_view_get_view(p_app_handle->p_list_view));
    mui_view_dispatcher_attach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher, OUTPUT_INFO_VIEW_ID_MAIN);

    // 定时器结构体初始化
    esp_timer_create_args_t data_timer = {
        .callback = &app_output_info_on_draw, // 定时器回调函数
        .arg = p_app_handle->p_list_view,     // 传递给回调函数的参数
        .name = "update_timer",               // 定时器名称
    };

    /**
     * 创建定时器
     *     返回值为定时器句柄，用于后续对定时器进行其他操作。
     */
    esp_timer_create(&data_timer, &p_app_handle->data_timer_handle);
    // 启动定时器 以循环方式启动定时器
    esp_timer_start_periodic(p_app_handle->data_timer_handle, 500000); // us级定时，500ms
}

void app_output_info_on_kill(mini_app_inst_t *p_app_inst)
{
    app_output_info_t *p_app_handle = p_app_inst->p_handle;

    esp_timer_stop(p_app_handle->data_timer_handle);
    esp_timer_delete(p_app_handle->data_timer_handle);

    mui_view_dispatcher_detach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_free(p_app_handle->p_view_dispatcher);

    mui_mem_free(p_app_handle);

    p_app_inst->p_handle = NULL;
}

void app_output_info_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event) {}

const mini_app_t app_output_info_info = {.id = MINI_APP_ID_OUTPUT_INFO,
                                         .name = "输出信息",
                                         .icon = 0xe1c5,
                                         .deamon = false,
                                         .sys = false,
                                         .hide = false,
                                         .run_cb = app_output_info_on_run,
                                         .kill_cb = app_output_info_on_kill,
                                         .on_event_cb = app_output_info_on_event};
