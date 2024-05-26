#include "zte3000.h"
#include "can.h"
#include "utils.h"

#include "app_config.h"

#define VOLTAGE_OFFSET 1
#define CURRENT_OFFSET 1
#define INPUT_CURRENT_OFFSET 1

static app_data_t app_data = {0};
static const uint8_t data[8] = {0x78, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static uint8_t id = 0;

static uint32_t get_send_id(uint32_t sid)
{
    return sid + (id * 0x100);
}

void zte3000_set_status(bool status)
{
    uint8_t can_data[8] = {0};
    can_data[0] = 0x78;
    can_data[1] = 0x30;
    can_data[2] = status ? 0x00 : 0x01;
    can_send(get_send_id(0x12780030), can_data, 8);
}

static void set_voltage_current(float voltage, float current)
{
    uint16_t v = voltage / VOLTAGE_OFFSET * 1000;
    uint16_t a = current / CURRENT_OFFSET * 1000;
    uint8_t can_data[8] = {0};
    can_data[0] = 0x78;
    can_data[1] = 0x20;
    can_data[2] = 0x02;
    can_data[3] = 0x58;
    can_data[4] = (uint8_t)(v >> 0x8);
    can_data[5] = (uint8_t)(v);

    can_data[6] = (uint8_t)(a >> 0x8);
    can_data[7] = (uint8_t)(a);

    can_send(0x12780020, can_data, 8); // 在线
    /* 延时多久之后开始执行调压?
        TxData[0] = 0x78;
        TxData[1] = 0x21;
        TxData[2] = 0;
        TxData[3] = 5;
        TxData[4] = 0;
        TxData[5] = 0;
        TxData[6] = 0;
        TxData[7] = 0;
        can1_transmit(0x12780021, TxData);
    */
}

static void set_offline_voltage_current(float voltage, float current)
{
    uint16_t v = voltage / VOLTAGE_OFFSET * 1000;
    uint16_t a = current / CURRENT_OFFSET * 1000;
    uint8_t can_data[8] = {0};
    can_data[0] = 0x78;
    can_data[1] = 0x22;
    can_data[2] = 0x02;
    can_data[3] = 0x58;
    can_data[4] = (uint8_t)(v >> 0x8);
    can_data[5] = (uint8_t)(v);

    can_data[6] = (uint8_t)(a >> 0x8);
    can_data[7] = (uint8_t)(a);

    can_send(0x12780022, can_data, 8); // 离线
}

void zte3000_can_init_handle(uint32_t can_id, uint8_t *can_data)
{
    id = (can_id >> 0x10) & 0xFF;
    zte3000_set_status(true);
    set_voltage_current(app_data.voltage, app_data.current);
}

void zte3000_can_data_handle(uint32_t can_id, uint8_t *can_data)
{
    if (can_id != 0x1a007810 + (id * 0x1000))
    {
        return;
    }
    uint8_t mode = can_data[1];
    uint16_t data = unpack_uint16_big_endian(can_data + 2);
    uint16_t data2 = unpack_uint16_big_endian(can_data + 4);
    uint16_t data3 = unpack_uint16_big_endian(can_data + 6);
    switch (mode)
    {
    case 0x00:
        power_data.input_voltage = data / 10;
        break;

    case 0x41:
        power_data.input_current = data / 10 * INPUT_CURRENT_OFFSET;
        power_data.input_power = power_data.input_voltage * power_data.input_current;
        break;

    case 0x42:
        power_data.output_current = data2 / 10 * CURRENT_OFFSET;
        power_data.output_temp = data3 / 10;
        power_data.output_voltage = data * VOLTAGE_OFFSET;
        power_data.output_power = power_data.output_voltage * power_data.output_current;
        break;

    case 0x84: // 不确定
        LOGI("Max current: %d", data3);
        break;

    default:
        break;
    }
}

power_protocol_data_t *zte3000_get_data()
{
    return &power_data;
}

void zte3000_init_power_protocol()
{
    config_t *config = get_config();
    app_data.current = config->set_current;
    app_data.voltage = config->set_voltage;
}

static int tick_count = 0;
void zte3000_tick()
{
    config_t *config = get_config();
    tick_count++;

    if (id != 0) // 1000ms
    {
        can_send(get_send_id(0x1A780012), data, 8);
    }

    switch (tick_count)
    {
    case 2: // 2000ms
        set_voltage_current(config->set_voltage, config->set_current);
        tick_count = 0;
        break;
    default:
        break;
    }
}

const power_protocol_app_t zte3000_info = {
    .name = "ZTE ZXD3000",
    .can_init_handle = zte3000_can_init_handle,
    .can_data_handle = zte3000_can_data_handle,
    .set_status = zte3000_set_status,
    .set_online_voltage_current = set_voltage_current,
    .set_offline_voltage_current = set_offline_voltage_current,
    .draw_module_info = NULL,
    .init = zte3000_init_power_protocol,
    .get_data = zte3000_get_data,
    .tick = zte3000_tick,
    .tick_rate = MS2US(1000),
    .can_speed = 500000,
};
