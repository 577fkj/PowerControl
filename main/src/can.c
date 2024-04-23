#include "can.h"
#include "freertos/task.h"
#include "power_protocol.h"

#include "log.h"

void can_init()
{
    // CAN接口基本配置
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_PIN, RX_GPIO_PIN, TWAI_MODE_NO_ACK);
    // CAN接口时序配置官方提供了1K to 1Mbps的常用配置
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS(); // TWAI_TIMING_CONFIG_500KBITS()
    // 接收全部
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    // 初始化CAN接口
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        LOGI("Driver installed\n");
    }
    else
    {
        LOGI("Failed to install driver\n");
        return;
    }

    if (twai_start() == ESP_OK)
    {
        LOGI("Driver started\n");
    }
    else
    {
        LOGI("Failed to start driver\n");
        return;
    }
    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 1024 * 4, NULL, 10, NULL, tskNO_AFFINITY);
}

static void twai_receive_task(void *arg)
{
    twai_message_t r1;
    power_protocol_app_t *power_protocol = get_current_power_protocol();
    while (1)
    {
        esp_err_t err = twai_receive(&r1, portMAX_DELAY);
        switch (err)
        {
        case ESP_OK:
            // printf_msg(RECEIVEMSG, &r1);
            power_protocol->can_data_handle(r1.identifier, r1.data);
            break;
        case ESP_ERR_TIMEOUT:
            LOGI("Receive timeout\n");
            break;

        default:
            break;
        }
    }
    vTaskDelete(NULL);
}

void can_send(uint32_t can_id, uint8_t data[], uint8_t data_length)
{

    // twai_message_t s1 = {
    //     .extd = 0,                         // 0-标准帧; 1-扩展帧
    //     .rtr = 0,                          // 0-数据帧; 1-远程帧
    //     .ss = 1,                           // 0-错误重发; 1-单次发送(仲裁或丢失时消息不会被重发)，对接收消息无效
    //     .self = 0,                         // 0-不接收自己发送的消息，1-接收自己发送的消息，对接收消息无效
    //     .dlc_non_comp = 0,                 // 0-数据长度不大于8(ISO 11898-1); 1-数据长度大于8(非标);
    //     .identifier = 0xcc,                // 11/29位ID
    //     .data_length_code = 4,             // DLC数据长度4bit位宽
    //     .data = {0, 0, 0, 0, 0, 0, 0, 0}   //发送数据，对远程帧无效
    // };

    twai_message_t s1 = {
        .identifier = can_id,
        .extd = 1,
        .self = 0,
        .data_length_code = data_length,
    };
    for (int i = 0; i < data_length; i++)
    {
        s1.data[i] = data[i];
    }
    esp_err_t err = twai_transmit(&s1, portMAX_DELAY);
    switch (err)
    {
    case ESP_OK:
        break;
    case ESP_ERR_TIMEOUT:
        LOGI("Send timeout\n");
        break;

    default:
        break;
    }
}