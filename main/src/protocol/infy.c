#include "eps6020.h"
#include "can.h"
#include "utils.h"

#include "app_config.h"

#define VOLTAGE_OFFSET 1
#define CURRENT_OFFSET 1

static uint16_t id = 0;

static const uint8_t empty_data[8] = {0};

static uint32_t get_send_id(uint32_t sid)
{
    return sid + id;
}

void infy_set_status(bool status)
{
    uint8_t can_data[8] = {0};
    can_data[0] = status ? 0x00 : 0x01;
    can_send(0x029A00F0, can_data, 8);
}

static void set_voltage_current(float voltage, float current)
{
    uint16_t v = voltage / VOLTAGE_OFFSET * 1000;
    uint16_t a = current / CURRENT_OFFSET * 1000;
    uint8_t can_data[8] = {0};
    can_data[0] = 0x00;

    can_data[1] = (uint8_t)(a >> 0x10);
    can_data[2] = (uint8_t)(a >> 0x8);
    can_data[3] = (uint8_t)(a);

    can_data[4] = 0x00;
    can_data[5] = 0x00;

    can_data[6] = (uint8_t)(v >> 0x8);
    can_data[7] = (uint8_t)(v);
    can_send(get_send_id(0x029C00F0), can_data, 8);

    can_send(get_send_id(0x028600F0), empty_data, 8);
    can_send(get_send_id(0x028400F0), empty_data, 8);
    can_send(get_send_id(0x028900F0), empty_data, 8);
}

void infy_can_init_handle(uint32_t can_id, uint8_t *can_data)
{
    if (can_id == 0x028CF00C)
    {
        id = 0x0C00;
    }
    can_send(0x028A00F0, empty_data, 8);
    infy_set_status(true);
}

void infy_can_data_handle(uint32_t can_id, uint8_t *can_data)
{
    if (can_id != 0x0286f000 && can_id != 0x0281f03f && can_id != 0x0284F000 && can_id != 0x028AF000)
    {
        return;
    }
    if (can_id == 0x0286f000)
    {
        uint16_t iv = unpack_uint16_big_endian(can_data);
        if (iv == 0)
        {
            iv = unpack_uint16_big_endian(can_data + 8);
        }
        power_data.input_voltage = iv / 10;
        can_send(0x02813FF0, empty_data, 8);
    }
    else if (can_id == 0x0281f03f)
    {
        uint32_t v = unpack_uint32_big_endian(can_data);
        uint32_t a = unpack_uint32_big_endian(can_data + 4);
        power_data.output_voltage = v * VOLTAGE_OFFSET;
        power_data.output_current = a * CURRENT_OFFSET;
        power_data.output_power = power_data.output_voltage * power_data.output_current;
        can_send(0x02883FF0, empty_data, 8);
    }
    else if (can_id == 0x0284F000)
    {
        power_data.input_temp = can_data[4];
        can_send(0x028600F0, empty_data, 8);
        uint8_t status2 = can_data[5];
        uint8_t status1 = can_data[6];
        uint8_t status = can_data[7];
        LOGI("status2: %d, status1: %d, status: %d\n", status2, status1, status);
        /*
        bin(status)
        0b00100000 设置关机
        0b00000001 输出短路

        bin(status1)
        0b00100000 出过压
        0b00010000 过热保护
        0b00001000 风机故障
        0b00000100 恒压状态
        0b00000010 模块故障
        0b00000001 模块关机

        bin(status2)
        0b01000000 输入过压
        0b00100000 输入欠压
        0b00001000 输出欠压
        0b00000001 过流保护
        */
    }
    else if (can_id == 0x028AF000)
    {
        uint16_t max_voltage = unpack_uint16_big_endian(can_data + 2) * VOLTAGE_OFFSET;
        uint16_t min_voltage = unpack_uint16_big_endian(can_data + 4) * VOLTAGE_OFFSET;
        uint16_t min_current = unpack_uint16_big_endian(can_data + 6) * CURRENT_OFFSET;
        LOGI("max_voltage: %d, min_voltage: %d, min_current: %d\n", max_voltage, min_voltage, min_current);
    }
}

power_protocol_data_t *infy_get_data()
{
    return &power_data;
}

static int tick_count = 0;
void infy_tick()
{
    config_t *config = get_config();
    tick_count++;

    if (tick_count % 2 == 0) // 1000ms
    {
        infy_set_status(true);
    }

    switch (tick_count)
    {
    case 3: // 1500ms
        set_voltage_current(config->set_voltage, config->set_current);
        break;
    case 4: // 2000ms
        can_send(0x028A00F0, empty_data, 8);
        tick_count = 0;
        break;
    default:
        break;
    }
}

const power_protocol_app_t infy_info = {
    .name = "INFY",
    .can_init_handle = infy_can_init_handle,
    .can_data_handle = infy_can_data_handle,
    .set_status = infy_set_status,
    .set_online_voltage_current = set_voltage_current,
    .draw_module_info = NULL,
    .init = NULL,
    .get_data = infy_get_data,
    .tick = infy_tick,
    .tick_rate = MS2US(500),
    .can_speed = 125000,
};
