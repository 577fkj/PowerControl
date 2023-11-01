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

#include "huawei_r48xx.h"
#include "can.h"
#include "ble.h"
#include "oled_u8g2.h"
#include "u8g2.h"

#include "mui_core.h"
#include "mini_app_launcher.h"

#include "esp_task_wdt.h"
#include "bsp_btn.h"
#include "app_config.h"

#define RECEIVEMSG 1

int tick_count = 0;

void r48xx_tick()
{
    send_get_data();
    if (tick_count == 10)
    {
        ConfigStruct *config = get_config();
        set_voltage(config->set_voltage, false, true);
        set_current(config->set_current, false, true);
        tick_count = 0;
    }
    else
    {
        tick_count++;
    }
}

void app_main(void)
{
    // Initialize NVS
    // esp_err_t err = nvs_flash_init();
    // if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    // {
    //     // NVS partition was truncated and needs to be erased
    //     // Retry nvs_flash_init
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     err = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(err);
    config_init();
    oled_init();
    start_ani();
    int64_t start = esp_timer_get_time();
    ble_init();
    can_init();
    init_key();

    gpio_reset_pin(LED_GPIO_PIN);
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT); // 输出
    gpio_set_level(LED_GPIO_PIN, 1);

    // 定时器结构体初始化
    esp_timer_create_args_t data_timer = {
        .callback = &r48xx_tick, // 定时器回调函数
        .arg = NULL,             // 传递给回调函数的参数
        .name = "data_timer",    // 定时器名称
    };

    esp_timer_handle_t data_timer_handle = 0; // 定时器句柄

    /**
     * 创建定时器
     *     返回值为定时器句柄，用于后续对定时器进行其他操作。
     */
    esp_err_t err = esp_timer_create(&data_timer, &data_timer_handle);
    // 启动定时器 以循环方式启动定时器
    err = esp_timer_start_periodic(data_timer_handle, 1000000); // us级定时，1s
    if (err == ESP_OK)
    {
        printf("ok!\r\n");
    }

    if (esp_timer_get_time() - start < 1000000)
    {
        printf("start timer = %lld\n", (1000 - ((esp_timer_get_time() - start) / 1000)));
        vTaskDelay((1000 - ((esp_timer_get_time() - start) / 1000)) / portTICK_PERIOD_MS);
    }

    mui_t *p_mui = mui();
    p_mui->u8g2 = u8g2;
    mui_init(p_mui);

    mini_app_launcher_t *p_launcher = mini_app_launcher();
    mini_app_launcher_init(p_launcher);

    // esp_timer_handle_t mui_tick_timer_handle = 0; // 定时器句柄
    // 定时器结构体初始化
    // esp_timer_create_args_t fw_timer = {
    //     .callback = &mui_tick, // 定时器回调函数
    //     .arg = p_mui,          // 传递给回调函数的参数
    //     .name = "mui_tick",    // 定时器名称
    // };

    /**
     * 创建定时器
     *     返回值为定时器句柄，用于后续对定时器进行其他操作。
     */
    // err = esp_timer_create(&fw_timer, &mui_tick_timer_handle);
    // 启动定时器 以循环方式启动定时器
    // err = esp_timer_start_periodic(mui_tick_timer_handle, 50000); // us级定时，50ms
    // if (err == ESP_OK)
    // {
    //     printf("ok2!\r\n");
    // }

    // xTaskCreate(main_thread, "HumanReadableNameofTask", 4096, p_mui, tskIDLE_PRIORITY, NULL);
    // xTaskCreatePinnedToCore(main_thread, "Main_thread", 4096, p_mui, 8, NULL, tskNO_AFFINITY);

    // while (1)
    // {
    //     mui_tick(p_mui);
    //     vTaskDelay(50 / portTICK_PERIOD_MS);
    // }

    // printf("Hello world!\n");

    // /* Print chip information */
    // esp_chip_info_t chip_info;
    // uint32_t flash_size;
    // esp_chip_info(&chip_info);
    // printf("This is %s chip with %d CPU core(s), WiFi%s%s%s, ",
    //        CONFIG_IDF_TARGET,
    //        chip_info.cores,
    //        (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
    //        (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
    //        (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    // unsigned major_rev = chip_info.revision / 100;
    // unsigned minor_rev = chip_info.revision % 100;
    // printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    // if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    // {
    //     printf("Get flash size failed");
    //     return;
    // }

    // printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
    //        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    // printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    // for (int i = 10; i >= 0; i--)
    // {
    //     printf("Restarting in %d seconds...\n", i);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // printf("Restarting now.\n");
    // fflush(stdout);
    // esp_restart();
}
