#include "increase.h"
#include "can.h"
#include "utils.h"

#include "app_config.h"

#define VOLTAGE_OFFSET 1
#define CURRENT_OFFSET 1

static const uint8_t data2[8] = {0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 读取输入电压值
static const uint8_t data3[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 读取模块信息

uint32_t IncreaseEAddr_pack(IncreaseEAddr *self)
{
    return (self->is_recv ? 0x12 : 0x13) << 24 | (self->command & 0xFFF) << 11 | (self->group & 0x0F) << 7 | (self->addr & 0x7F);
}

void IncreaseEAddr_unpack(IncreaseEAddr *self, uint32_t val)
{
    self->addr = val & 0x7F;
    self->group = (val >> 7) & 0x0F;
    self->command = (val >> 11) & 0xFFF;
    self->is_recv = ((val >> 24) & 0x1F) == 0x12;
}

void increase_set_status(bool status)
{
    uint8_t can_data[8] = {0};
    can_data[0] = 0x02;
    can_data[7] = status ? 0x55 : 0xaa;
    can_send(0x1307c081, can_data, 8);
}

static void set_voltage_current(float voltage, float current)
{
    uint32_t v = voltage / VOLTAGE_OFFSET * 1000;
    uint32_t a = current / CURRENT_OFFSET * 1000;
    uint8_t can_data[8] = {0};
    can_data[0] = 0x00;

    can_data[1] = (uint8_t)(a >> 0x10);
    can_data[2] = (uint8_t)(a >> 0x8);
    can_data[3] = (uint8_t)(a);

    can_data[4] = (uint8_t)(v >> 0x18);
    can_data[5] = (uint8_t)(v >> 0x10);
    can_data[6] = (uint8_t)(v >> 0x8);
    can_data[7] = (uint8_t)(v);
    can_send(0x1307c081, can_data, 8);
}

void increase_can_init_handle(uint32_t can_id, uint8_t *can_data)
{
    increase_set_status(true);
}

void increase_can_data_handle(uint32_t can_id, uint8_t *can_data)
{
    if (can_id != 0x1207C081 && can_id != 0x1207A081 && can_id != 0x12008081)
    {
        return;
    }
    uint16_t cid = unpack_uint16_big_endian(can_data);
    if (can_id == 0x1207C081 && cid == 0x1)
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

power_protocol_data_t *increase_get_data()
{
    return &power_data;
}

static int tick_count = 0;
void increase_tick()
{
    config_t *config = get_config();
    tick_count++;

    can_send(0x1307c081, data3, 8);
    can_send(0x13008081, NULL, 0);
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
    .can_init_handle = increase_can_init_handle,
    .can_data_handle = increase_can_data_handle,
    .set_status = increase_set_status,
    .set_online_voltage_current = set_voltage_current,
    .draw_module_info = NULL,
    .init = NULL,
    .get_data = increase_get_data,
    .tick = increase_tick,
    .tick_rate = MS2US(1000),
    .can_speed = 125000,
};
