#include "zte4875.h"
#include "can.h"
#include "utils.h"

#include "app_config.h"

static uint8_t id = 0;
static uint8_t keys[3] = {0};

static uint32_t get_send_id(uint32_t sid)
{
    return sid + id;
}

power_protocol_data_t *zte4875_get_data()
{
    return &power_data;
}

uint16_t crc16_ccitt(uint8_t *data, uint16_t length)
{
    uint8_t i;
    uint16_t crc = 0; // Initial value
    while (length--)
    {
        crc ^= *data++; // crc ^= *data; data++;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0x8408; // 0x8408 = reverse 0x1021
            else
                crc = (crc >> 1);
        }
    }
    return crc;
}

void zte4875_set_status(bool status)
{
    uint8_t data[18];
    data[0] = 0xE0;
    data[1] = 0x7D;
    data[2] = 0x01;
    data[3] = 0x29;
    data[4] = 0x24;
    data[5] = status ? 0x05 : 0x04;
    data[6] = 0x80;
    data[7] = 0x01;

    data[11] = 0x06;
    data[15] = keys[0];
    data[16] = keys[1];
    data[17] = keys[2];

    uint16_t crc = crc16_ccitt(data, 18);

    uint8_t send_data[8];
    for (int i = 3; i != 0; i--)
    {
        send_data[0] = 0x00;
        memcpy(send_data + 1, data, 7);
        can_send(get_send_id(0x1FA04080), send_data, 18);

        memset(send_data, 0, 8);
        send_data[0] = 0x01;
        memcpy(send_data + 1, data + 7, 7);
        can_send(get_send_id(0x1FA04080), send_data, 18);

        memset(send_data, 0, 8);
        send_data[0] = 0x82;
        memcpy(send_data + 1, data + 14, 4);
        send_data[5] = crc >> 8;
        send_data[6] = crc;
        can_send(get_send_id(0x1FA04080), send_data, 8);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void zte4875_set_voltage_current(float voltage, float current)
{
    uint16_t v = voltage * 100;
    uint16_t a = current * 720;

    uint8_t data[0x28];
    data[0] = 0xE0;
    data[1] = 0x7D;
    data[2] = 0x01;
    data[3] = 0x26;
    data[7] = 0x01;
    data[8] = 0x08;
    data[11] = 0x1C;
    data[12] = 0x1B;
    data[13] = 0x02;
    data[14] = 0x62;

    data[15] = v >> 8;
    data[16] = v;

    data[17] = a >> 8;
    data[18] = a;

    data[20] = 0x01;
    data[22] = 0xFF;
    data[23] = 0xFF;
    data[25] = 0x28;

    data[27] = v >> 8;
    data[28] = v;

    data[29] = 0x01;
    data[30] = 0x01;
    data[32] = 0x01;
    data[34] = 0x01;

    uint16_t crc = crc16_ccitt(data, 0x28);

    uint8_t send_data[8];
    send_data[0] = 0x00;
    send_data[1] = 0xE0;
    send_data[2] = 0x7D;
    send_data[3] = 0x01;
    send_data[4] = 0x26;
    can_send(0x1FA04080, send_data, 8);

    memset(send_data, 0, 8);
    send_data[0] = 0x01;
    memcpy(send_data, data + 7, 7);
    can_send(0x1FA04080, send_data, 8);

    memset(send_data, 0, 8);
    send_data[0] = 0x02;
    memcpy(send_data, data + 14, 7);
    can_send(0x1FA04080, send_data, 8);

    memset(send_data, 0, 8);
    send_data[0] = 0x03;
    memcpy(send_data, data + 21, 7);
    can_send(0x1FA04080, send_data, 8);

    memset(send_data, 0, 8);
    send_data[0] = 0x04;
    memcpy(send_data, data + 28, 7);
    can_send(0x1FA04080, send_data, 8);

    memset(send_data, 0, 8);
    send_data[0] = 0x85;
    memcpy(send_data, data + 35, 5);
    send_data[6] = crc >> 8;
    send_data[7] = crc;
    can_send(0x1FA04080, send_data, 8);

    memset(send_data, 0, 8);
    send_data[1] = 0xE0;
    send_data[2] = 0x7D;
    send_data[3] = 0x01;
    send_data[4] = 0x20;
    send_data[7] = 0x80;
    can_send(get_send_id(0x1FA04080), send_data, 8);

    memset(send_data, 0, 8);
    send_data[0] = 0x81;
    send_data[1] = 0x01;
    send_data[6] = 0x3F;
    send_data[7] = 0x9B;
    can_send(get_send_id(0x1FA04080), send_data, 8);

    vTaskDelay(50 / portTICK_PERIOD_MS);

    memset(send_data, 0, 8);
    send_data[1] = 0xE0;
    send_data[2] = 0x7D;
    send_data[3] = 0x01;
    send_data[4] = 0x20;
    send_data[6] = 0x02;
    send_data[7] = 0x80;
    can_send(get_send_id(0x1FA04080), send_data, 8);

    memset(send_data, 0, 8);
    send_data[0] = 0x81;
    send_data[1] = 0x01;
    send_data[6] = 0x5F;
    send_data[7] = 0x78;
    can_send(get_send_id(0x1FA04080), send_data, 8);
}

void zte4875_can_init_handle(uint32_t can_id, uint8_t *can_data)
{
    id = (can_id >> 0x10) & 0xFF;

    keys[0] = can_data[5];
    keys[1] = can_data[6];
    keys[2] = can_data[7];

    zte4875_set_status(true);

    uint8_t data[8];
    for (int i = 3; i != 0; i--)
    {
        data[0] = 0x78;
        data[1] = 0x10;
        can_send(0x1A780012 + (id * 0x100), data, 8);

        vTaskDelay(300 / portTICK_PERIOD_MS);

        memset(data, 0, 8);
        data[1] = 0xE0;
        data[2] = 0x7D;
        data[3] = 0x01;
        data[4] = 0x20;
        data[7] = 0x80;
        can_send(get_send_id(0x1fa04080), data, 8);

        memset(data, 0, 8);
        data[0] = 0x81;
        data[1] = 0x01;
        data[6] = 0x3F;
        data[7] = 0x9B;
        can_send(get_send_id(0x1fa04080), data, 8);

        vTaskDelay(100 / portTICK_PERIOD_MS);

        memset(data, 0, 8);
        data[1] = 0xE0;
        data[2] = 0x7D;
        data[3] = 0x01;
        data[4] = 0x20;
        data[6] = 0x02;
        data[7] = 0x80;
        can_send(get_send_id(0x1fa04080), data, 8);

        memset(data, 0, 8);
        data[0] = 0x81;
        data[1] = 0x01;
        data[6] = 0x5F;
        data[7] = 0x78;
        can_send(get_send_id(0x1fa04080), data, 8);

        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

static uint8_t zte4875_status = 0;
void zte4875_can_data_handle(uint32_t can_id, uint8_t *can_data)
{
    if ((can_id | 0xff000) != 0x102ffe81)
    {
        return;
    }

    uint8_t mode = can_data[0];
    uint16_t data = unpack_uint16_big_endian(can_data + 1);
    switch (mode)
    {
    case 0x00:
        zte4875_status = can_data[6];
        break;

    case 0x02:
        if (zte4875_status == 0x00)
        {
            break;
        }
        power_data.input_power = data * 1.0; // TODO: offset = voltage_display_offset
        break;

    case 0x03:
        if (zte4875_status != 0x00)
        {
            break;
        }
        power_data.input_voltage = data / 1.0;
        power_data.input_temp = unpack_uint16_big_endian(can_data + 5) * 1.0;
        break;

    case 0x05:
        power_data.output_voltage = data * 1.0; // TODO: offset = zte4875_voltage_display_offset + (58.0 - voltage) * (zte4875_voltage_display_offset - voltage_display_offset) * 0.0625
        break;

    case 0x86:
        power_data.output_current = data * 1.0; // TODO: offset = current_display_offset
        break;

    default:
        break;
    }
}

const power_protocol_app_t zte4875_info = {
    .name = "ZTE R4875F1",
    .can_init_handle = zte4875_can_init_handle,
    .can_data_handle = zte4875_can_data_handle,
    .set_status = zte4875_set_status,
    .draw_module_info = NULL,
    .set_online_voltage_current = zte4875_set_voltage_current,
    .init = NULL,
    .get_data = zte4875_get_data,
    .tick = NULL,
    .tick_rate = MS2US(1000),
    .can_speed = 500000,
};