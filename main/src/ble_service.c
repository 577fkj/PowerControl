#include "ble_service.h"

#include "df_buffer.h"
#include "app_config.h"
#include "huawei_r48xx.h"

#include "utils.h"

void init_ble_service(void)
{
    set_ble_data_callback(app_data_handle, NULL);
}

void ble_app_power_data_callback(uint8_t *s_data, uint16_t len)
{
    hexdump(s_data, len);
    NEW_BUFFER_READ(data_read, s_data, len);

    ble_app_power_data_t mode = buff_get_u8(&data_read);
    uint8_t *data = buff_get_data_ptr_pos(&data_read);

    ConfigStruct *config = get_config();

    NEW_BUFFER_LOCAL(output, 0xFF);
    switch (mode)
    {
    case 0x00: // 获取配置
    {
        buff_put_float(&output, config->set_voltage);
        buff_put_float(&output, config->set_current);
        send_app_data(0x00, &output);
    }
    break;
    case 0x01: // 获取数据
    {
        buff_put_byte_array(&output, &power_data, sizeof(RectifierParameters));
        send_app_data(0x01, &output);
    }
    break;
    case POWER_SET_DATA_VOLTAGE: // 设置电压
    {
        float v = unpack_uint16_big_endian(buff_get_data_ptr_pos(&data_read)) / 1024.0;
        set_voltage(v, false, true);

        config->set_voltage = v;
        save_config(config);
    }
    break;
    case POWER_SET_DATA_CURRENT: // 设置电流
    {
        float c = unpack_uint16_big_endian(buff_get_data_ptr_pos(&data_read)) / 1024.0;
        set_current(c, false, true);

        config->set_current = c;
        save_config(config);
    }
    break;
    // case 0x04: // 设置过流保护
    // {
    //     float p = unpack_uint32_big_endian(buff_get_data_ptr_pos(&data_read)) / 1024.0;
    //     set_power(p, true);
    // }
    // break;
    case POWER_SET_DATA_POWER: // 设置电源状态
    {
        printf("power status: %d\n", data[0]);
        if (data[0])
        {
            printf("power on\n");
            power_on(true);
        }
        else
        {
            printf("power off\n");
            power_off(true);
        }
    }
    break;
    }
}

void ble_app_token_data_callback(uint8_t *data, uint16_t len)
{
}

void ble_app_config_data_callback(uint8_t *data, uint16_t len)
{
}

static ble_app_data_handle_t ble_app_data_handle[BLE_APP_MODE_MAX] = {
    NULL,                         // BLE_APP_MODE_NONE
    ble_app_power_data_callback,  // BLE_APP_MODE_DATA
    ble_app_token_data_callback,  // BLE_APP_MODE_TOKEN
    ble_app_config_data_callback, // BLE_APP_MODE_CONFIG
    NULL,                         // BLE_APP_MODE_MAX
};

uint8_t get_xor(uint8_t *data, uint16_t len)
{
    uint8_t result = 0;
    for (int i = 0; i < len; i++)
    {
        result ^= data[i];
    }
    return result;
}

bool send_app_error(ble_app_error_t id)
{
    NEW_BUFFER_LOCAL(buffer, 0xFF);
    buff_put_u16(&buffer, id);
    return send_app_data(0xFF, &buffer);
}

bool send_app_data(uint8_t mode, buffer_t *buf)
{
    NEW_BUFFER_LOCAL(buffer, 0xFF);
    buff_put_u8(&buffer, 0x85);                                                            // magic
    buff_put_u8(&buffer, mode);                                                            // mode
    buff_put_u8(&buffer, buff_get_size(buf));                                              // len
    buff_put_byte_array(&buffer, buff_get_data(buf), buff_get_size(buf));                  // data
    buff_put_u8(&buffer, get_xor(buff_get_data(&buffer) + 1, buff_get_size(&buffer) - 2)); // xor
    buff_put_u8(&buffer, 0x95);                                                            // magic end
    return ble_send_data(buff_get_data(&buffer), buff_get_size(&buffer));
}

ble_app_chunk_t ble_app_chunk = {
    .data_len = 0,
    .data = {0},
};

void app_data_handle(uint8_t *ble_data, uint16_t len, void *user_data)
{
    NEW_BUFFER_READ(buffer, ble_data, len);
    uint8_t xor = buff_get_u8(&buffer);
    uint8_t mode = buff_get_u8(&buffer);
    uint8_t status = buff_get_u16(&buffer);
    uint8_t chunk = buff_get_u8(&buffer);
    uint8_t chunk_status = buff_get_u8(&buffer);
    uint8_t data_len = len - buff_get_pos(&buffer);
    uint8_t data[data_len];
    buff_get_byte_array(&buffer, data, data_len);

    if (get_xor(ble_data + 1, len - 2) != xor)
    {
        printf("data error!");
        if (!send_app_error(DATA_CHECK_ERROR))
        {
            printf("ble error!!!");
        }
        return;
    }

    if (mode >= BLE_APP_MODE_MAX || mode <= BLE_APP_MODE_NONE || !ble_app_data_handle[mode])
    {
        printf("mode error!");
        if (!send_app_error(MODE_ERROR))
        {
            printf("ble error!!!");
        }
        return;
    }

    /*
    分片处理
    如果分片为1并且模式不一致，初始化分片
    如果分片为0，分片结束
    需要判断模式是否一致
    */

    if (chunk == 1) // 初始化分片
    {
        ble_app_chunk.data_len = 0;
        memset(ble_app_chunk.data, 0, sizeof(ble_app_chunk.data));
    }

    if (chunk > 0)
    {
        if (ble_app_chunk.data_len + data_len > sizeof(ble_app_chunk.data))
        {
            printf("data too long!");
            if (!send_app_error(DATA_TOO_LONG_ERROR))
            {
                printf("ble error!!!");
            }
            return;
        }
        memcpy(ble_app_chunk.data + ble_app_chunk.data_len, data, data_len);
        ble_app_chunk.data_len += data_len;

        if (chunk_status == 0) // 分片结束
        {
            ble_app_data_handle[mode](ble_app_chunk.data, ble_app_chunk.data_len);
            ble_app_chunk.data_len = 0;
            memset(ble_app_chunk.data, 0, sizeof(ble_app_chunk.data));
        }
        else
        {
            if (!send_app_error(BLE_NEXT_CHUNK))
            {
                printf("ble error!!!");
            }
        }
    }
    else
    {
        ble_app_data_handle[mode](data, data_len);
    }
}

void app_ble_set_callback(uint8_t mode, bool success, uint8_t *data, uint8_t len)
{
    NEW_BUFFER_LOCAL(buffer, 0xFF);
    uint32_t val = unpack_uint32_big_endian(data + 4);
    buff_put_u8(&buffer, mode);
    buff_put_u8(&buffer, !success);
    switch (mode)
    {
    case 0x00: // 设置在线电压
    {
        buff_put_float(&buffer, val / RATIO_MULTIPLIER);
    }
    break;

    case 0x01: // 设置离线电压
    {
        buff_put_float(&buffer, val / RATIO_MULTIPLIER);
    }
    break;

    case 0x02: // 设置过流保护
    {
        buff_put_float(&buffer, val / RATIO_MULTIPLIER);
    }
    break;

    case 0x03: // 设置在线输出电流
    {
        buff_put_float(&buffer, val / MAX_CURRENT_MULTIPLIER);
    }
    break;

    case 0x04: // 设置离线输出电流
    {
        buff_put_float(&buffer, val / MAX_CURRENT_MULTIPLIER);
    }
    break;

    case 0x32: // 开关机状态
        buff_put_u8(&buffer, data[3] == 0);
        break;

    default:
        break;
    }
    send_app_data(0x10, &buffer);
}