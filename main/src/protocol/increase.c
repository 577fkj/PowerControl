#include "eps6020.h"
#include "can.h"
#include "utils.h"

#include "app_config.h"

#define VOLTAGE_OFFSET 1
#define CURRENT_OFFSET 1

static uint32_t base_id = 0x1307C080;
static power_protocol_data_t power_data = {0};
static app_data_t app_data = {0};

static const uint8_t empty_data[8] = {0};
static const uint8_t data2[8] = {0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t data3[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void increase_set_status(bool status)
{
    uint8_t can_data[8] = {0};
    can_data[0] = 0x02;
    can_data[7] = status ? 0x55 : 0xaa;
    can_send(base_id + 1, can_data, 8);
}

static void set_voltage_current(float voltage, float current)
{
    uint16_t v = voltage / VOLTAGE_OFFSET * 1000;
    uint16_t a = current / CURRENT_OFFSET * 1000;
    uint8_t can_data[8] = {0};
    can_data[0] = 0x00;
    can_data[1] = 0x00;

    can_data[2] = (uint8_t)(a >> 8);
    can_data[3] = (uint8_t)(a);

    can_data[4] = (uint8_t)(v >> 24);
    can_data[5] = (uint8_t)(v >> 16);
    can_data[6] = (uint8_t)(v >> 8);
    can_data[7] = (uint8_t)(v);
    can_send(base_id + 1, can_data, 8);
}

void increase_set_voltage(float c, bool perm, bool callback)
{
    app_data.voltage = c;
    set_voltage_current(app_data.voltage, app_data.current);
}

static void increase_set_current(float current, bool online, bool callback)
{
    app_data.current = current;
    set_voltage_current(app_data.voltage, app_data.current);
}

void increase_can_data_handle(uint32_t can_id, uint8_t *can_data)
{
    if (can_id != 0x1207C081 && can_id != 0x1207A081 && can_id != 0x12008081)
    {
        return;
    }
    uint16_t cid = unpack_uint16_big_endian(can_data);
    if (can_id == 0x1207C081 && cid == 1)
    {
        uint16_t j = unpack_uint16_big_endian(can_data + 2);
        uint16_t k = unpack_uint16_big_endian(can_data + 4);
        power_data.output_current = j / 10 * CURRENT_OFFSET;
        power_data.output_voltage = k / 10 * VOLTAGE_OFFSET;
        power_data.output_power = power_data.output_current * power_data.output_voltage;

        uint8_t status1 = can_data[6];
        uint8_t status0 = can_data[7];
        LOGI("status1: %d, status0: %d\n", status1, status0);
        /*
            bin(status0)
            0b10000000 输出欠压
            0b01000000 输出过压
            0b00100000 输入欠压
            0b00010000 输入过压
            0b00001000 风机故障
            0b00000100 恒压状态
            0b00000010 模块故障
            0b00000001 模块关机

            bin(status1)
            0b00000100 设置关机
            0b00000010 过热保护
            0b00000001 过流保护
         */
    }
    else if (can_id == 0x12008081)
    {
        power_data.output_temp = unpack_uint32_big_endian(can_data + 8) / 10;
    }
    else if (can_id == 0x1207A081)
    {
        power_data.input_voltage = unpack_uint32_big_endian(can_data + 4) / 32;
    }
}

void increase_send_get_data()
{
}

power_protocol_data_t *increase_get_data()
{
    return &power_data;
}

void increase_init_power_protocol()
{
    config_t *config = get_config();
    app_data.current = config->set_current;
    app_data.voltage = config->set_voltage;
}

static int tick_count = 0;
void increase_tick()
{
    config_t *config = get_config();
    tick_count++;

    can_send(base_id + 1, data3, 8);
    can_send(0x13008081, empty_data, 8);
    can_send(0x1307A081, data2, 8);

    switch (tick_count)
    {
    case 2: // 2000ms
        set_voltage_current(config->set_voltage, config->set_current);
        increase_set_status(true);
        tick_count = 0;
        break;
    default:
        break;
    }
}

const power_protocol_app_t increase_info = {
    .name = "Increase",
    .can_data_handle = increase_can_data_handle,
    .set_status = increase_set_status,
    .set_current = increase_set_current,
    .set_voltage = increase_set_voltage,
    .set_power = NULL,
    .draw_module_info = NULL,
    .loop_get_data = increase_send_get_data,
    .init = increase_init_power_protocol,
    .get_data = increase_get_data,
    .tick = increase_tick,
    .tick_rate = MS2US(1000),
    .can_speed = 125000,
};
