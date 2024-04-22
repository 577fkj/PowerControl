#include "module_offset_view.h"
#include "app_module_offset.h"
#include "app_config.h"
#include "app_control_bar.h"
#include "power_protocol.h"

#include "mini_app_registry.h"
#include "mini_app_launcher.h"

#include "driver/gptimer.h"
#include "esp_timer.h"

#include "utils.h"

uint8_t count = 0;

static void app_list_control_view_on_draw(void *user_data, mui_canvas_t *p_canvas)
{
    mui_canvas_set_font(p_canvas, u8g2_font_siji_t_6x10);
    mui_canvas_draw_glyph(p_canvas, 0, 14, 0xe060);
    if (count == 6)
    {
        mui_canvas_draw_glyph(p_canvas, 0, 35, 0xe069);
    }
    else
    {
        mui_canvas_draw_glyph(p_canvas, 0, 35, 0xe105);
    }
    mui_canvas_draw_glyph(p_canvas, 0, 57, 0xe061);
}

static void module_offset_view_on_draw(mui_view_t *p_view, mui_canvas_t *p_canvas)
{
    mui_canvas_draw_line(p_canvas, 64, 0, 64, 64);
    mui_canvas_set_font(p_canvas, u8g2_font_wqy12_t_gb2312a);
    char txt[10];
    config_t *config = get_config();
    power_protocol_app_t *power_protocol = get_current_power_protocol();
    power_protocol_data_t *power_data = power_protocol->get_data();

    switch (count)
    {
    case 0:
        mui_canvas_draw_utf8(p_canvas, 8, 10, "设置电压");
        mui_canvas_draw_utf8(p_canvas, 19, 23, "偏移");
        sprintf(txt, "%.2f", config->set_offset_voltage);
        mui_canvas_draw_utf8(p_canvas, 17, 35, txt);
        mui_canvas_draw_utf8(p_canvas, 68, 10, "设置电压");
        sprintf(txt, "%.2fV", config->set_voltage);
        mui_canvas_draw_utf8(p_canvas, 75, 25, txt);
        break;

    case 1:
        mui_canvas_draw_utf8(p_canvas, 8, 10, "设置电流");
        mui_canvas_draw_utf8(p_canvas, 19, 23, "偏移");
        sprintf(txt, "%.2f", config->set_offset_current);
        mui_canvas_draw_utf8(p_canvas, 17, 35, txt);
        mui_canvas_draw_utf8(p_canvas, 68, 10, "设置电流");
        sprintf(txt, "%.2fA", config->set_current);
        mui_canvas_draw_utf8(p_canvas, 75, 25, txt);
        break;

    case 2:
        mui_canvas_draw_utf8(p_canvas, 8, 10, "输入电压");
        mui_canvas_draw_utf8(p_canvas, 19, 23, "偏移");
        sprintf(txt, "%.2f", config->offset_voltage_in);
        mui_canvas_draw_utf8(p_canvas, 17, 35, txt);
        mui_canvas_draw_utf8(p_canvas, 68, 10, "输入电压");
        sprintf(txt, "%.2fV", power_data->input_voltage);
        mui_canvas_draw_utf8(p_canvas, 70, 25, txt);
        break;

    case 3:
        mui_canvas_draw_utf8(p_canvas, 8, 10, "输入电流");
        mui_canvas_draw_utf8(p_canvas, 19, 23, "偏移");
        sprintf(txt, "%.2f", config->offset_current_in);
        mui_canvas_draw_utf8(p_canvas, 17, 35, txt);
        mui_canvas_draw_utf8(p_canvas, 68, 10, "输入电流");
        sprintf(txt, "%.2fA", power_data->input_current);
        mui_canvas_draw_utf8(p_canvas, 75, 25, txt);
        break;

    case 4:
        mui_canvas_draw_utf8(p_canvas, 8, 10, "显示电压");
        mui_canvas_draw_utf8(p_canvas, 19, 23, "偏移");
        sprintf(txt, "%.2f", config->offset_voltage);
        mui_canvas_draw_utf8(p_canvas, 17, 35, txt);
        mui_canvas_draw_utf8(p_canvas, 68, 10, "显示电压");
        LOGI("%.2fV\n", power_data->output_voltage);
        sprintf(txt, "%.2fV", power_data->output_voltage);
        mui_canvas_draw_utf8(p_canvas, 75, 25, txt);
        break;

    case 5:
        mui_canvas_draw_utf8(p_canvas, 8, 10, "显示电流");
        mui_canvas_draw_utf8(p_canvas, 19, 23, "偏移");
        sprintf(txt, "%.2f", config->offset_current);
        mui_canvas_draw_utf8(p_canvas, 17, 35, txt);
        mui_canvas_draw_utf8(p_canvas, 80, 10, "显示电流");
        sprintf(txt, "%.2fA", power_data->output_current);
        mui_canvas_draw_utf8(p_canvas, 75, 25, txt);
        break;

    case 6:
        mui_canvas_draw_utf8(p_canvas, 8, 10, "其他偏移");
        sprintf(txt, "%.2f", config->other_offset);
        mui_canvas_draw_utf8(p_canvas, 17, 35, txt);
        break;

    default:
        break;
    }
}

static void module_offset_view_on_input(mui_view_t *p_view, mui_input_event_t *event)
{
    // TODO: offset
    // float val = 0.0;
    // switch (event->type)
    // {
    // case INPUT_TYPE_REPEAT:
    // case INPUT_TYPE_SHORT:
    //     switch (event->key)
    //     {
    //     case INPUT_KEY_LEFT:
    //         val += 0.1;
    //         break;
    //     case INPUT_KEY_CENTER:
    //         count++;
    //         break;
    //     case INPUT_KEY_RIGHT:
    //         val -= 0.1;
    //         break;
    //     default:
    //         break;
    //     }
    //     break;

    // case INPUT_TYPE_LONG:
    //     break;

    // default:
    //     break;
    // }

    // ConfigStruct *config = get_config();
    // switch (count)
    // {
    // case 0:
    //     config->set_offset_voltage += val;
    //     set_voltage(config->set_voltage, false, true);
    //     break;
    // case 1:
    //     config->set_offset_current += val;
    //     set_current(config->set_current, false, true);
    //     break;
    // case 2:
    //     config->offset_voltage_in += val;
    //     break;
    // case 3:
    //     config->offset_current_in += val;
    //     break;
    // case 4:
    //     config->offset_voltage += val;
    //     break;
    // case 5:
    //     config->offset_current += val;
    //     break;
    // case 6:
    //     config->other_offset += val;
    //     break;
    // case 7:
    //     save_config(config);
    //     mini_app_launcher_run(mini_app_launcher(), MINI_APP_ID_APP_LIST);
    //     break;
    // }
    // send_get_data();
    // mui_update(mui());
}

static void module_offset_view_on_enter(mui_view_t *p_view) {}

static void module_offset_view_on_exit(mui_view_t *p_view) {}

module_offset_view_t *module_offset_view_create()
{
    module_offset_view_t *p_module_offset_view = mui_mem_malloc(sizeof(module_offset_view_t));

    count = 0;

    create_timer_with_handle(&p_module_offset_view->timer, update_mui, &mui_update, mui());
    esp_timer_start_periodic(p_module_offset_view->timer, 500000);

    mui_view_t *p_view = mui_view_create();
    p_view->user_data = p_module_offset_view;
    p_view->draw_cb = module_offset_view_on_draw;
    p_view->input_cb = module_offset_view_on_input;
    p_view->enter_cb = module_offset_view_on_enter;
    p_view->exit_cb = module_offset_view_on_exit;

    p_module_offset_view->p_view = p_view;

    app_control_bar_set_draw(NULL, app_list_control_view_on_draw);

    return p_module_offset_view;
}

void module_offset_view_free(module_offset_view_t *p_view)
{
    app_control_bar_set_draw(NULL, NULL);

    esp_timer_stop(p_view->timer);
    esp_timer_delete(p_view->timer);

    mui_view_free(p_view->p_view);
    mui_mem_free(p_view);
}

mui_view_t *module_offset_view_get_view(module_offset_view_t *p_view) { return p_view->p_view; }
