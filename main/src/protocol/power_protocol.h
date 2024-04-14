#ifndef __POWER_PROTOCOL_H__
#define __POWER_PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>

typedef void (*can_data_handle_t)(uint32_t can_id, uint8_t *can_data);

typedef void (*set_status_t)(bool status);

typedef void (*set_current_t)(float current, bool online, bool callback);

typedef void (*set_voltage_t)(float voltage, bool online, bool callback);

typedef void (*set_power_t)(float power, bool online, bool callback);

typedef void (*callback_t)(void *user_data, void *data, uint32_t size);

typedef void (*get_info_t)(callback_t callback, void *user_data);

typedef void (*get_desc_t)(callback_t callback, void *user_data);

typedef power_protocol_app_t *(*get_data_t)(void);

typedef void (*loop_get_data_t)(void);

typedef void (*init_t)(void);

typedef enum
{
    POWER_STATUS_NOT_READY = 0,
    POWER_STATUS_POWER_ON,
    POWER_STATUS_POWER_OFF,
} power_status;

typedef struct
{
    float input_voltage;
    float input_current;
    float input_power;
    float input_frequency;
    float input_temp;

    float output_voltage;
    float output_current;
    float output_power;
    float output_temp;

    float efficiency;
    float amp_hours; // 单位：Ah

    power_status status;
} power_protocol_data_s;

typedef struct
{
    char name[16];
    can_data_handle_t can_data_handle;
    set_status_t set_status;
    set_current_t set_current;
    set_voltage_t set_voltage;
    set_power_t set_power;
    get_info_t get_info;
    get_desc_t get_desc;
    loop_get_data_t loop_get_data;
    init_t init;
    get_data_t get_data;
} power_protocol_app_s;

typedef power_protocol_data_s power_protocol_data_t;

typedef power_protocol_app_s power_protocol_app_t;

#endif // __POWER_PROTOCOL_H__