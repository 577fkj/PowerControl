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

static void app_list_control_view_on_draw(void *user_data, mui_canvas_t *p_canvas)
{
    mui_canvas_set_font(p_canvas, u8g2_font_t0_13b_tr);
    mui_canvas_draw_utf8(p_canvas, 0, 14, "+");
    mui_canvas_draw_utf8(p_canvas, 0, 35, "*");
    mui_canvas_draw_utf8(p_canvas, 0, 57, "-");
}

static void module_offset_view_on_draw(mui_view_t *p_view, mui_canvas_t *p_canvas)
{
    module_offset_view_t *p_module_offset_view = p_view->user_data;

    mui_canvas_draw_line(p_canvas, 64, 0, 64, 64);
    mui_canvas_set_font(p_canvas, u8g2_font_wqy12_t_gb2312a);

    char txt[10];
    config_t *config = get_config();
    power_protocol_app_t *power_protocol = get_current_power_protocol();
    power_protocol_data_t *power_data = power_protocol->get_data();

    switch (p_module_offset_view->count)
    {
    case 0:
        mui_canvas_draw_utf8(p_canvas, 8, 10, "实际电压");
        sprintf(txt, "%.2fV", p_module_offset_view->offset);
        mui_canvas_draw_utf8(p_canvas, 17, 25, txt);

        mui_canvas_draw_utf8(p_canvas, 14, 40, "偏移量");
        sprintf(txt, "%.4f", config->set_offset_voltage);
        mui_canvas_draw_utf8(p_canvas, 15, 55, txt);

        mui_canvas_draw_utf8(p_canvas, 68, 10, "设置电压");
        sprintf(txt, "%.2fV", config->set_voltage);
        mui_canvas_draw_utf8(p_canvas, 75, 25, txt);

        mui_canvas_draw_utf8(p_canvas, 68, 40, "输出电压");
        sprintf(txt, "%.2fV", power_data->output_voltage);
        mui_canvas_draw_utf8(p_canvas, 75, 55, txt);
        break;

    case 1:
        mui_canvas_draw_utf8(p_canvas, 8, 10, "实际电流");
        sprintf(txt, "%.2fA", p_module_offset_view->offset);
        mui_canvas_draw_utf8(p_canvas, 17, 25, txt);

        mui_canvas_draw_utf8(p_canvas, 14, 40, "偏移量");
        sprintf(txt, "%.4f", config->set_offset_current);
        mui_canvas_draw_utf8(p_canvas, 15, 55, txt);

        mui_canvas_draw_utf8(p_canvas, 68, 10, "设置电流");
        sprintf(txt, "%.2fA", config->set_current);
        mui_canvas_draw_utf8(p_canvas, 75, 25, txt);

        mui_canvas_draw_utf8(p_canvas, 68, 40, "输出电流");
        sprintf(txt, "%.2fA", power_data->output_current);
        mui_canvas_draw_utf8(p_canvas, 75, 55, txt);
        break;

    default:
        break;
    }
}

static void module_offset_view_on_input(mui_view_t *p_view, mui_input_event_t *event)
{
    // TODO: offset
    module_offset_view_t *p_module_offset_view = p_view->user_data;
    app_module_offset_t *p_app_handle = p_module_offset_view->user_data;
    config_t *config = get_config();
    power_protocol_app_t *power_protocol = get_current_power_protocol();
    power_protocol_data_t *power_data = power_protocol->get_data();

    float val = 0.0;
    switch (event->type)
    {
    case INPUT_TYPE_REPEAT:
        switch (event->key)
        {
        case INPUT_KEY_LEFT:
            val += 0.3;
            break;

        case INPUT_KEY_RIGHT:
            val -= 0.3;
            break;
        default:
            break;
        }
        break;

    case INPUT_TYPE_SHORT:
        switch (event->key)
        {
        case INPUT_KEY_LEFT:
            val += 0.1;
            break;

        case INPUT_KEY_CENTER:
            switch (p_module_offset_view->count)
            {
            case 0:
                config->set_offset_voltage = p_module_offset_view->offset / config->set_voltage;
                config->display_offset_voltage = p_module_offset_view->offset / power_data->output_voltage;
                power_protocol->set_online_voltage_current(config->set_voltage, config->set_current);
                break;

            case 1:
                config->set_offset_current = p_module_offset_view->offset / config->set_current;
                config->display_offset_current = p_module_offset_view->offset / power_data->output_current;
                power_protocol->set_online_voltage_current(config->set_voltage, config->set_current);
                break;
            }
            break;

        case INPUT_KEY_RIGHT:
            val -= 0.1;
            break;
        default:
            break;
        }
        break;

    case INPUT_TYPE_LONG:
        switch (event->key)
        {
        case INPUT_KEY_CENTER:
            if (p_module_offset_view->count == 0 && power_protocol->base_voltage_info.max_voltage > 0)
            {
                config->max_output_voltage = power_protocol->base_voltage_info.max_voltage * config->set_offset_voltage;
                config->min_output_voltage = power_protocol->base_voltage_info.min_voltage * config->set_offset_voltage;
                float voltage = 50.0 * config->set_offset_voltage;
                config->set_voltage = voltage;
                power_protocol->set_online_voltage_current(voltage, config->set_current);
            }
            save_config(config);
            mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher, MODULE_OFFSET_LIST_VIEW_ID_MAIN);
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
    if (p_module_offset_view->offset + val > 0)
        p_module_offset_view->offset += val;

    mui_update(mui());
}

static void module_offset_view_on_enter(mui_view_t *p_view)
{
    module_offset_view_t *p_module_offset_view = p_view->user_data;
    p_module_offset_view->old_draw_cb = app_control_bar_get_draw();

    app_control_bar_set_draw(NULL, app_list_control_view_on_draw);
}

static void module_offset_view_on_exit(mui_view_t *p_view)
{
    module_offset_view_t *p_module_offset_view = p_view->user_data;
    app_control_bar_set_draw(NULL, p_module_offset_view->old_draw_cb);
}

module_offset_view_t *module_offset_view_create()
{
    module_offset_view_t *p_module_offset_view = mui_mem_malloc(sizeof(module_offset_view_t));

    create_timer_with_handle(&p_module_offset_view->timer, update_mui, &mui_update, mui());
    esp_timer_start_periodic(p_module_offset_view->timer, 500000);

    mui_view_t *p_view = mui_view_create();
    p_view->user_data = p_module_offset_view;
    p_view->draw_cb = module_offset_view_on_draw;
    p_view->input_cb = module_offset_view_on_input;
    p_view->enter_cb = module_offset_view_on_enter;
    p_view->exit_cb = module_offset_view_on_exit;

    p_module_offset_view->p_view = p_view;

    p_module_offset_view->count = 0;

    return p_module_offset_view;
}

void module_offset_view_free(module_offset_view_t *p_view)
{
    esp_timer_stop(p_view->timer);
    esp_timer_delete(p_view->timer);

    mui_view_free(p_view->p_view);
    mui_mem_free(p_view);
}

mui_view_t *module_offset_view_get_view(module_offset_view_t *p_view) { return p_view->p_view; }
