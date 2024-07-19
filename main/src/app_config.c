#include "app_config.h"

#include "nvs_flash.h"

#include <string.h>
#include <math.h>

#include "log.h"

void load_config(config_t *config)
{
    // 打开NVS命名空间
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvsHandle);
    if (err != ESP_OK)
    {
        LOGI("Error opening NVS namespace!\n");
        return;
    }

    // 从NVS中读取二进制数据，并将其复制到结构体中
    size_t dataSize = sizeof(config_t);
    err = nvs_get_blob(nvsHandle, "config", config, &dataSize);
    if (err != ESP_OK)
    {
        LOGI("Error reading config from NVS!\n");
    }

    // 关闭NVS命名空间
    nvs_close(nvsHandle);
}

inline void check_float(float *value, float default_value)
{
    if (isinf(*value) || isnan(*value))
    {
        *value = default_value;
    }
}

void config_init()
{
    // Initialize NVS.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    config_t *config = get_config();
    load_config(config);

    if (config->magic != 0x01)
    {
        memset(config, 0, sizeof(config_t));
        config->magic = 0x01;

        // 默认配置
        config->max_output_voltage = 100.0;
        config->max_output_current = 50.0;

        config->min_output_voltage = 0.0;
        config->min_output_current = 0.0;

        config->set_voltage = 50.0;
        config->set_current = 10.0;

        // 默认偏移
        config->set_offset_voltage = 1.0;
        config->set_offset_current = 1.0;
        config->display_offset_voltage = 1.0;
        config->display_offset_current = 1.0;

        config->zte4875_display_offset_voltage = 1.0;
        config->zte4875_set_offset_voltage = 1.0;

        strcpy(config->ble_name, "PowerControl");

        save_config(config);
    }

    check_float(&config->max_output_voltage, 100.0);
    check_float(&config->max_output_current, 50.0);
    check_float(&config->min_output_voltage, 0.0);
    check_float(&config->min_output_current, 0.0);
    check_float(&config->set_voltage, 50.0);
    check_float(&config->set_current, 10.0);
    check_float(&config->set_offset_voltage, 1.0);
    check_float(&config->set_offset_current, 1.0);
    check_float(&config->display_offset_voltage, 1.0);
    check_float(&config->display_offset_current, 1.0);
    check_float(&config->zte4875_display_offset_voltage, 1.0);
    check_float(&config->zte4875_set_offset_voltage, 1.0);
}

void save_config(config_t *config)
{
    // 打开NVS命名空间
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK)
    {
        LOGI("Error opening NVS namespace!\n");
        return;
    }

    // 将结构体作为二进制数据存储在NVS中
    err = nvs_set_blob(nvsHandle, "config", config, sizeof(config_t));
    if (err != ESP_OK)
    {
        LOGI("Error storing config in NVS!\n");
    }

    // 提交更改并关闭NVS命名空间
    err = nvs_commit(nvsHandle);
    if (err != ESP_OK)
    {
        LOGI("Error committing NVS!\n");
    }
    nvs_close(nvsHandle);

    LOGI("Save config to NVS!\n");
}

config_t *get_config()
{
    static config_t config;
    return &config;
}