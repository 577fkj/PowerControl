#include "app_config.h"

#include "nvs_flash.h"

#include <string.h>

void load_config(ConfigStruct *config)
{
    // 打开NVS命名空间
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvsHandle);
    if (err != ESP_OK)
    {
        printf("Error opening NVS namespace!\n");
        return;
    }

    // 从NVS中读取二进制数据，并将其复制到结构体中
    size_t dataSize = sizeof(ConfigStruct);
    err = nvs_get_blob(nvsHandle, "config", config, &dataSize);
    if (err != ESP_OK)
    {
        printf("Error reading config from NVS!\n");
    }

    // 关闭NVS命名空间
    nvs_close(nvsHandle);
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

    ConfigStruct *config = get_config();
    load_config(config);

    if (config->magic != 0x57)
    {
        memset(config, 0, sizeof(ConfigStruct));
        config->magic = 0x57;

        config->set_offset_voltage = 1024.0;
        config->set_offset_current = 30.0;
        config->offset_voltage = 1024.0;
        config->offset_current = 30.0;

        config->max_output_voltage = 58.5;
        config->max_output_current = 50.0;

        config->min_output_voltage = 41.1;
        config->min_output_current = 0.0;

        config->offset_voltage_in = 1024.0;
        config->offset_current_in = 30.0;

        config->other_offset = 1024.0;

        strcpy(config->ble_name, "PowerControl");

        save_config(config);
    }
}

void save_config(ConfigStruct *config)
{
    // 打开NVS命名空间
    nvs_handle_t nvsHandle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK)
    {
        printf("Error opening NVS namespace!\n");
        return;
    }

    // 将结构体作为二进制数据存储在NVS中
    err = nvs_set_blob(nvsHandle, "config", config, sizeof(ConfigStruct));
    if (err != ESP_OK)
    {
        printf("Error storing config in NVS!\n");
    }

    // 提交更改并关闭NVS命名空间
    err = nvs_commit(nvsHandle);
    if (err != ESP_OK)
    {
        printf("Error committing NVS!\n");
    }
    nvs_close(nvsHandle);
}

ConfigStruct *get_config()
{
    static ConfigStruct config;
    return &config;
}