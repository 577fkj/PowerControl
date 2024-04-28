#include "eps6020.h"
#include "can.h"
#include "utils.h"

#include "app_config.h"

#define VOLTAGE_OFFSET 1
#define CURRENT_OFFSET 1

static uint16_t id = -1;
static power_protocol_data_t power_data = {0};
static app_data_t app_data = {0};

static const uint8_t empty_data[8] = {0};

void eps_6020_set_status(bool status)
{
    uint8_t can_data[8] = {0};
    can_data[0] = status ? 0x00 : 0x01;
    can_send((0x029A00F0 + (id * 256)), can_data, 8);
}

static void set_voltage_current(float voltage, float current)
{
    uint16_t v = voltage / VOLTAGE_OFFSET * 1000;
    uint16_t a = current / CURRENT_OFFSET * 1000;
    uint8_t can_data[8] = {0};
    can_data[0] = (uint8_t)(v >> 24);
    can_data[1] = (uint8_t)(v >> 16);
    can_data[2] = (uint8_t)(v >> 8);
    can_data[3] = (uint8_t)(v);
    can_data[4] = (uint8_t)(a >> 24);
    can_data[5] = (uint8_t)(a >> 16);
    can_data[6] = (uint8_t)(a >> 8);
    can_data[7] = (uint8_t)(a);
    can_send((0x029C00F0 + (id * 256)), can_data, 8);
}

void eps_6020_set_voltage(float c, bool perm, bool callback)
{
    app_data.voltage = c;
    set_voltage_current(app_data.voltage, app_data.current);
}

void eps_6020_set_current(float current, bool online, bool callback)
{
    app_data.current = current;
    set_voltage_current(app_data.voltage, app_data.current);
}

void eps_6020_can_data_handle(uint32_t can_id, uint8_t *can_data)
{
    if (id == -1)
    {
        id = can_id & 0xFF;
    }
    if (can_id != (0x0286f000 + id) && can_id != 0x0281f03f && can_id != (0x0289f000 + id) && can_id != (0x028AF000 + id))
    {
        return;
    }
    if (can_id == (0x0289f000 + id))
    {
        uint32_t c = unpack_uint32_big_endian(can_data);
        uint32_t k = unpack_uint32_big_endian(can_data + 4);
        float ov = c / 1000 * VOLTAGE_OFFSET;
        float oa = k / 1000 * CURRENT_OFFSET;
        power_data.output_voltage = ov;
        power_data.output_current = oa;
        power_data.output_power = ov * oa;
    }
}

void eps_6020_send_get_data()
{
    can_send(0x02883FF0, empty_data, 8);
}

power_protocol_data_t *eps_6020_get_data()
{
    return &power_data;
}

void eps_6020_init_power_protocol()
{
    config_t *config = get_config();
    app_data.current = config->set_current;
    app_data.voltage = config->set_voltage;
}

static int tick_count = 0;
void eps_6020_tick()
{
    config_t *config = get_config();
    tick_count++;

    if (tick_count % 2 == 0) // 1000ms
    {
        can_send(0x028900F0 + (id * 256), empty_data, 8);
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
        can_send(0x028A00F0 + (id * 256), empty_data, 8);
        tick_count = 0;
        break;
    default:
        break;
    }
}

const power_protocol_app_t eps_6020_info = {
    .name = "EPS-6020",
    .can_data_handle = eps_6020_can_data_handle,
    .set_status = eps_6020_set_status,
    .set_current = eps_6020_set_current,
    .set_voltage = eps_6020_set_voltage,
    .set_power = NULL,
    .draw_module_info = NULL,
    .loop_get_data = eps_6020_send_get_data,
    .init = eps_6020_init_power_protocol,
    .get_data = eps_6020_get_data,
    .tick = eps_6020_tick,
    .tick_rate = MS2US(500),
    .can_speed = 125000,
};
