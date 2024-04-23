
#include "adc.h"
#include "esp_err.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "utils.h"
#include "log.h"

#include "math.h"

#include <string.h>

static TaskHandle_t s_task_handle;

static adc_cali_handle_t cali_handle = NULL;
static adc_continuous_handle_t adc_handle = NULL;

float temp = 0;

float get_temp()
{
    return temp;
}

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    // Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

float temp_trans(uint16_t ADC_value)
{
    float B = 3435;                                         // B值
    float R = 10000;                                        // 10K固定阻值电阻
    float T0 = 273.15 + 25;                                 // 转换为开尔文温度
    float Ka = 273.15;                                      // K值
    float VR = ADC_value / 1000.0;                          // 电压值
    float Rt = (R * VR) / (3.3f - VR);                      // 电阻值
    float temp = 1 / (1 / T0 + log(Rt / R) / B) - Ka + 0.5; // 计算温度
    return temp;
}

void data_output_task(void *pvParameters)
{
    uint8_t result[256] = {0};
    memset(result, 0xcc, 256);
    uint32_t ret_num = 0;
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // 等待ADC转换完成

        esp_err_t ret = adc_continuous_read(adc_handle, result, 256, &ret_num, 0);
        if (ret == ESP_OK)
        {
            uint64_t rs = 0;
            for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
            {
                adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
                uint32_t chan_num = p->type2.channel;
                uint32_t data = p->type2.data;
                uint32_t out = 0;
                if (chan_num == 0)
                {
                    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, data, &out));
                    rs += out;
                }
            }
            rs /= (ret_num / SOC_ADC_DIGI_RESULT_BYTES); // 求电压平均值
            temp = temp_trans(rs);
        }
    }
}

void adc_init()
{
    s_task_handle = xTaskGetCurrentTaskHandle();

    // 创建校准
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = SOC_ADC_DIGI_MAX_BITWIDTH,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));

    // 创建ADC
    adc_continuous_handle_cfg_t adc_handle_cfg = {
        .max_store_buf_size = 1024,
        .conv_frame_size = 256,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_handle_cfg, &adc_handle));

    // 配置ADC通道
    adc_digi_pattern_config_t dig_config = {
        .atten = ADC_ATTEN_DB_11,
        .channel = ADC_CHANNEL_0 & 0x7,
        .unit = ADC_UNIT_1,
        .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
    };

    // 配置ADC
    adc_continuous_config_t adc_config = {
        .pattern_num = 1,
        .adc_pattern = &dig_config,
        .sample_freq_hz = SOC_ADC_SAMPLE_FREQ_THRES_HIGH,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
    };
    ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &adc_config));

    xTaskCreate(data_output_task, "adc", 1024 * 10, NULL, 0, &s_task_handle);

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(adc_handle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));
}