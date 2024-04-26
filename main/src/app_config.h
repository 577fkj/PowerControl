#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#include <stdint.h>

typedef struct
{
    uint8_t magic; // 0x57
    uint32_t run_timer;

    float max_output_voltage;
    float max_output_current;

    float min_output_voltage;
    float min_output_current;

    float set_voltage;
    float set_current;

    // output offset
    float set_offset_voltage;
    float set_offset_current;
    float offset_voltage;
    float offset_current;

    // input offset
    float offset_voltage_in;
    float offset_current_in;

    float other_offset;

    uint16_t start_time;

    char ble_name[32];

    char ssid[32];
    char password[32];

    uint16_t fan_speed;
    uint16_t screen_backlight;

    uint8_t power_protocol;
} config_t;

void config_init();
config_t *get_config();
void save_config(config_t *config);

#endif