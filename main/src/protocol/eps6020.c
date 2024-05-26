#include "eps6020.h"
#include "can.h"
#include "utils.h"

#include "app_config.h"

#define VOLTAGE_OFFSET 1
#define CURRENT_OFFSET 1

static uint16_t id = 0;
static const uint8_t empty_data[8] = {0};

void eps_6020_set_status(bool status)
{
    uint8_t can_data[8] = {0};
    can_data[0] = status ? 0x00 : 0x01;
    can_send(0x29a00f0, can_data, 8);
}

static uint32_t get_send_id(uint32_t sid)
{
    return sid + id;
}

static void set_voltage_current(float voltage, float current)
{
    uint16_t v = voltage / VOLTAGE_OFFSET * 1000;
    uint16_t a = current / CURRENT_OFFSET * 1000;
    uint8_t can_data[8] = {0};

    can_data[0] = 0x00;

    can_data[1] = (uint8_t)(v >> 0x10);
    can_data[2] = (uint8_t)(v >> 0x8);
    can_data[3] = (uint8_t)(v);

    can_data[4] = 0x00;
    can_data[5] = 0x00;

    can_data[6] = (uint8_t)(a >> 0x8);
    can_data[7] = (uint8_t)(a);
    can_send(get_send_id(0x029C00F0), can_data, 8);
}

void eps_6020_can_init_handle(uint32_t can_id, uint8_t *can_data)
{
    if (can_id == 0x028cf00c)
    {
        id = 0x0C00;
    }
    can_send(get_send_id(0x028A00F0), empty_data, 8);
    eps_6020_set_status(true);
}

void eps_6020_can_data_handle(uint32_t can_id, uint8_t *can_data)
{
    uint16_t real_id = id;
    if (id < 0)
    {
        real_id = id + 0xFF;
    }

    can_id = can_id - (real_id >> 0x8);
    if (can_id != 0x0286f000 && can_id != 0x0289f000)
    {
        return;
    }

    uint32_t c = unpack_uint32_big_endian(can_data);
    uint32_t k = unpack_uint32_big_endian(can_data + 4);
    if (can_id == 0x0286f000)
    {
        float iv = c / 1000 * VOLTAGE_OFFSET;
        float ia = k / 1000 * CURRENT_OFFSET;
        power_data.input_voltage = iv;
        power_data.input_current = ia;
        power_data.input_power = iv * ia;
    }
    else if (can_id == 0x0289f000)
    {
        float ov = c / 1000 * VOLTAGE_OFFSET;
        float oa = k / 1000 * CURRENT_OFFSET;
        power_data.output_voltage = ov;
        power_data.output_current = oa;
        power_data.output_power = ov * oa;
    }
}

power_protocol_data_t *eps_6020_get_data()
{
    return &power_data;
}

static int tick_count = 0;
void eps_6020_tick()
{
    config_t *config = get_config();
    tick_count++;

    if (tick_count % 2 == 0) // 1000ms
    {
        can_send(0x02883FF0, empty_data, 8);
        can_send(get_send_id(0x028900F0), empty_data, 8);
    }

    switch (tick_count)
    {
    case 4: // 2000ms
        eps_6020_set_status(true);
        break;
    case 5: // 2500ms
        set_voltage_current(config->set_voltage, config->set_current);
        break;
    case 6: // 3000ms
        can_send(get_send_id(0x028A00F0), empty_data, 8);
        tick_count = 0;
        break;
    default:
        break;
    }
}

const power_protocol_app_t eps_6020_info = {
    .name = "EPS-6020",
    .can_init_handle = eps_6020_can_init_handle,
    .can_data_handle = eps_6020_can_data_handle,
    .set_status = eps_6020_set_status,
    .set_online_voltage_current = set_voltage_current,
    .draw_module_info = NULL,
    .init = NULL,
    .get_data = eps_6020_get_data,
    .tick = eps_6020_tick,
    .tick_rate = MS2US(500),
    .can_speed = 125000,
};
