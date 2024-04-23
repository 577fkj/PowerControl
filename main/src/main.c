/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "main.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "FreeRTOSConfig.h"
#include "freertos/queue.h"

#include "esp_chip_info.h"
#include "esp_flash.h"

#include "driver/gptimer.h"
#include "esp_timer.h"

#include "log.h"
#include "adc.h"
#include "can.h"
#include "ble.h"
#include "ble_service.h"
#include "oled_u8g2.h"
#include "u8g2.h"
#include "utils.h"
#include "pwm.h"

#include "mui_core.h"
#include "mini_app_launcher.h"

#include "esp_task_wdt.h"
#include "bsp_btn.h"
#include "app_config.h"

#include "power_protocol.h"

#define RECEIVEMSG 1
#define SHOW_LOGO_TIME 500

void can_tick()
{
    power_protocol_app_t *power_protocol = get_current_power_protocol();
    power_protocol->tick();
}

void app_main(void)
{
    config_init();
    oled_init();
    start_ani();
    int64_t start = esp_timer_get_time();
    ble_init();
    init_ble_service();
    can_init();
    init_key();
    adc_init();
    init_pwm();

    led_set_level(10);

    // 加载电源协议
    power_protocol_app_t *power_protocol = get_current_power_protocol();
    LOGI("load power protocol %s, tick rate: %lld\n", power_protocol->name, power_protocol->tick_rate);
    power_protocol->init();

    // 启动定时器 以循环方式启动定时器
    create_timer(data, &can_tick, NULL);
    esp_err_t err = esp_timer_start_periodic(data_timer_handle, power_protocol->tick_rate);
    ESP_ERROR_CHECK(err);

    mui_t *p_mui = mui();
    p_mui->u8g2 = u8g2;
    mui_init(p_mui);

    if (esp_timer_get_time() - start < MS2US(SHOW_LOGO_TIME))
    {
        LOGI("start delay = %lld\n", US2MS(MS2US(SHOW_LOGO_TIME) - (esp_timer_get_time() - start)));
        vTaskDelay(US2MS(MS2US(SHOW_LOGO_TIME) - (esp_timer_get_time() - start)));
    }

    mini_app_launcher_t *p_launcher = mini_app_launcher();
    mini_app_launcher_init(p_launcher);
}
