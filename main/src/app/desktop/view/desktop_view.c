#include "desktop_view.h"
#include "app_desktop.h"
#include "mui_include.h"
#include <stdbool.h>

#include "driver/gptimer.h"
#include "esp_timer.h"

#include "app_control_bar.h"
#include "app_config.h"

#include "mini_app_registry.h"
#include "mini_app_launcher.h"

#include "utils.h"
#include "log.h"
#include "adc.h"

#include "power_protocol.h"

bool time_tick = false;
bool set_tick = false;
static int timer = 0;

static void desktop_control_view_on_draw(void *user_data, mui_canvas_t *p_canvas)
{
    if (((desktop_view_t *)user_data)->set_event == SET_NONE)
    {
        mui_canvas_set_font(p_canvas, u8g2_font_t0_13b_tr); /*字库选择*/
        mui_canvas_draw_utf8(p_canvas, 2, 14, "V");

        mui_canvas_set_font(p_canvas, u8g2_font_t0_13b_tr);
        mui_canvas_draw_utf8(p_canvas, 2, 35, "A");

        mui_canvas_set_font(p_canvas, u8g2_font_siji_t_6x10);
        mui_canvas_draw_glyph(p_canvas, 0, 57, 0xe005);
    }
    else
    {
        mui_canvas_set_font(p_canvas, u8g2_font_siji_t_6x10);
        mui_canvas_draw_glyph(p_canvas, 0, 14, 0xe060);
        mui_canvas_draw_glyph(p_canvas, 0, 35, 0xe105);
        mui_canvas_draw_glyph(p_canvas, 0, 57, 0xe061);
    }
}

static void desktop_view_on_draw(mui_view_t *p_view, mui_canvas_t *p_canvas)
{
    config_t *config = get_config();
    power_protocol_app_t *power_protocol = get_current_power_protocol();
    power_protocol_data_t *power_data = power_protocol->get_data();

    desktop_view_t *p_desktop_view = p_view->user_data;
    char buffer[64];

    mui_canvas_set_font(p_canvas, u8g2_font_inr16_mn); /*字库选择*/
    sprintf(buffer, "%.2f", power_data->output_voltage);
    mui_canvas_draw_utf8_right(p_canvas, 65, 18, buffer);
    // mui_canvas_set_font(p_canvas, u8g2_font_t0_18b_tf);
    // mui_canvas_draw_utf8(p_canvas, 68, 18, "V");

    mui_canvas_set_font(p_canvas, u8g2_font_inr16_mn);
    sprintf(buffer, "%.2f", power_data->output_current);
    mui_canvas_draw_utf8_right(p_canvas, 65, 38, buffer);
    // mui_canvas_set_font(p_canvas, u8g2_font_t0_18b_tf);
    // mui_canvas_draw_utf8(p_canvas, 68, 38, "A");

    mui_canvas_draw_line(p_canvas, 65, 0, 65, 40);
    mui_canvas_draw_line(p_canvas, 0, 40, 115, 40);

    mui_canvas_set_font(p_canvas, u8g2_font_wqy12_t_gb2312a);
    mui_canvas_draw_utf8_right(p_canvas, 115, 10, "炸飞老铁");
    mui_canvas_draw_utf8_right(p_canvas, 115, 22, "丢雷楼谋");

    mui_canvas_draw_line(p_canvas, 65, 25, 115, 25);

    mui_canvas_set_font(p_canvas, u8g2_font_6x13_tf);
    // sprintf(buffer, , power_data.amp_hour / 3600);

    sprintf(buffer, time_tick ? "%02d:%02d:%02d" : "%02d %02d %02d", timer / 3600, timer / 60, timer % 60);
    mui_canvas_draw_utf8_right(p_canvas, 115, 38, buffer);

    if (p_desktop_view->set_event != SET_CURRENT)
    {
        mui_canvas_set_font(p_canvas, u8g2_font_6x13_tf);
        mui_canvas_draw_utf8(p_canvas, 3, 52, "Set");
        sprintf(buffer, config->set_voltage >= 100 ? "%05.1fV" : "%05.2fV", config->set_voltage);
        mui_canvas_draw_utf8_right(p_canvas, 60, 52, buffer);
    }
    else
    {
        if (set_tick)
        {
            mui_canvas_set_font(p_canvas, u8g2_font_siji_t_6x10);
            mui_canvas_draw_glyph(p_canvas, (p_desktop_view->set_pos >= 3 ? 27 : 21) + (p_desktop_view->set_pos * 6), 52, 0xe061);
        }
    }

    if (p_desktop_view->set_event != SET_VOLTAGE)
    {
        mui_canvas_set_font(p_canvas, u8g2_font_6x13_tf);
        mui_canvas_draw_utf8(p_canvas, 3, 63, "Set");
        sprintf(buffer, "%04.1fA", config->set_current);
        mui_canvas_draw_utf8_right(p_canvas, 60, 63, buffer);
    }
    else
    {
        if (set_tick)
        {
            mui_canvas_set_font(p_canvas, u8g2_font_siji_t_6x10);
            mui_canvas_draw_glyph(p_canvas, (p_desktop_view->set_pos >= (config->set_voltage >= 100 ? 3 : 2) ? 27 : 21) + (p_desktop_view->set_pos * 6), 63, 0xe060);
        }
    }

    mui_canvas_draw_line(p_canvas, 62, 40, 62, 64);

    mui_canvas_set_font(p_canvas, u8g2_font_6x13_tf);

    float ah = power_data->amp_hours / 3600;
    sprintf(buffer, ah < 100 ? "%06.3fAH" : "%06.2fAH", ah);
    mui_canvas_draw_utf8_right(p_canvas, 113, 52, buffer);

    sprintf(buffer, "%04dW", (int)power_data->output_power);
    mui_canvas_draw_utf8(p_canvas, 65, 63, buffer);

    sprintf(buffer, "%.f°", get_temp());
    mui_canvas_draw_utf8_right(p_canvas, 115, 63, buffer);
}

void adjustValue(float *value, int pos, bool increase)
{
    float increment = increase ? 0.01 : -0.01;

    switch (pos)
    {
    case 0:
        *value += increment * 1000;
        break;
    case 1:
        *value += increment * 100;
        break;
    case 2:
        *value += increment * 10;
        break;
    case 3:
        *value += increment;
        break;
    default:
        break;
    }
}

static void desktop_view_on_input(mui_view_t *p_view, mui_input_event_t *event)
{
    config_t *config = get_config();
    power_protocol_app_t *power_protocol = get_current_power_protocol();
    desktop_view_t *p_desktop_view = p_view->user_data;
    switch (event->type)
    {
    // case INPUT_TYPE_REPEAT:
    case INPUT_TYPE_SHORT:
        switch (event->key)
        {
        case INPUT_KEY_LEFT:
            if (p_desktop_view->set_event == SET_VOLTAGE)
            {
                if (config->set_voltage < config->max_output_voltage)
                {
                    adjustValue(&config->set_voltage, p_desktop_view->set_pos, true);
                    power_protocol->set_voltage(config->set_voltage, false, true);
                }
            }
            else if (p_desktop_view->set_event == SET_CURRENT)
            {
                if (config->set_current < config->max_output_current)
                {
                    adjustValue(&config->set_current, p_desktop_view->set_pos - 1, true);
                    power_protocol->set_current(config->set_current, false, true);
                }
            }
            break;
        case INPUT_KEY_RIGHT:
            if (p_desktop_view->set_event == SET_VOLTAGE)
            {
                if (config->min_output_voltage < config->set_voltage)
                {
                    adjustValue(&config->set_voltage, p_desktop_view->set_pos, false);
                    power_protocol->set_voltage(config->set_voltage, false, true);
                }
            }
            else if (p_desktop_view->set_event == SET_CURRENT)
            {
                if (config->min_output_current < config->set_current)
                {
                    adjustValue(&config->set_current, p_desktop_view->set_pos - 1, false);
                    power_protocol->set_current(config->set_current, false, true);
                }
            }
            else
            {
                mini_app_launcher_run(mini_app_launcher(), MINI_APP_ID_APP_LIST);
            }
            break;
        case INPUT_KEY_CENTER:
        {
            if (p_desktop_view->set_event != SET_NONE)
            {
                if (p_desktop_view->set_pos >= 3)
                {
                    p_desktop_view->set_event = SET_NONE;
                    p_desktop_view->set_pos = 0;
                    save_config(config);
                }
                else
                {
                    p_desktop_view->set_pos++;
                }
            }
        }
        break;
        default:
            break;
        }
        break;

    case INPUT_TYPE_LONG:
        if (p_desktop_view->set_event == SET_NONE)
        {
            switch (event->key)
            {
            case INPUT_KEY_LEFT:
                p_desktop_view->set_event = SET_VOLTAGE;
                p_desktop_view->set_pos = 0;
                break;
            case INPUT_KEY_CENTER:
                p_desktop_view->set_event = SET_CURRENT;
                p_desktop_view->set_pos = 1;
                break;

            default:
                break;
            }
        }

    default:
        break;
    }
    mui_update(mui());
}

static void desktop_view_on_enter(mui_view_t *p_view) {}

static void desktop_view_on_exit(mui_view_t *p_view) {}

void desktop_tick_timer_callback(void *arg)
{
    power_protocol_app_t *power_protocol = get_current_power_protocol();
    power_protocol_data_t *power_data = power_protocol->get_data();
    set_tick = !set_tick;
    if (power_data->output_current > 0.1)
    {
        time_tick = !time_tick;
    }
    else
    {
        time_tick = true;
    }
    if (!time_tick)
    {
        timer++;
    }
    mui_update(mui());
}

desktop_view_t *desktop_view_create()
{
    desktop_view_t *p_desktop_view = mui_mem_malloc(sizeof(desktop_view_t));

    mui_view_t *p_view = mui_view_create();
    p_view->user_data = p_desktop_view;
    p_view->draw_cb = desktop_view_on_draw;
    p_view->input_cb = desktop_view_on_input;
    p_view->enter_cb = desktop_view_on_enter;
    p_view->exit_cb = desktop_view_on_exit;

    p_desktop_view->p_view = p_view;
    p_desktop_view->set_event = SET_NONE;

    app_control_bar_set_draw(p_desktop_view, desktop_control_view_on_draw);

    LOGI("create desktop view\r\n");

    create_timer_with_handle(&p_desktop_view->data_timer_handle, desktop_data, &desktop_tick_timer_callback, NULL);
    esp_err_t err = esp_timer_start_periodic(p_desktop_view->data_timer_handle, 500000); // us级定时，500ms
    ESP_ERROR_CHECK(err);

    return p_desktop_view;
}

void desktop_view_free(desktop_view_t *p_view)
{
    LOGI("free desktop view\r\n");
    app_control_bar_set_draw(NULL, NULL);
    mui_view_free(p_view->p_view);
    esp_timer_stop(p_view->data_timer_handle);
    esp_timer_delete(p_view->data_timer_handle);
    mui_mem_free(p_view);
}
mui_view_t *desktop_view_get_view(desktop_view_t *p_view) { return p_view->p_view; }
