#ifndef __POWER_PROTOCOL_H__
#define __POWER_PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>

#include "mui_list_view.h"

typedef void (*can_data_handle_t)(uint32_t can_id, uint8_t *can_data);

typedef void (*set_status_t)(bool status);

typedef void (*set_current_t)(float current, bool online, bool callback);

typedef void (*set_voltage_t)(float voltage, bool online, bool callback);

typedef void (*set_power_t)(float power, bool online, bool callback);

typedef void (*callback_t)(void *user_data, void *data, uint32_t size);

typedef void (*draw_module_info_t)(mui_list_view_t *p_app_handle);

typedef void (*get_desc_t)(callback_t callback, void *user_data);

typedef void (*loop_get_data_t)(void);

typedef void (*init_t)(void);

typedef void (*tick_t)(void);

typedef enum
{
    POWER_STATUS_OFFLINE = 0,
    POWER_STATUS_NOT_READY,
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
    float amp_hours; // 单位：mAh -> mAh / 3600 = Ah

    power_status status;
} power_protocol_data_s;

typedef power_protocol_data_s power_protocol_data_t;

typedef power_protocol_data_t *(*get_data_t)(void);

typedef struct
{
    char name[16];
    can_data_handle_t can_data_handle;
    set_status_t set_status;
    set_current_t set_current;
    set_voltage_t set_voltage;
    set_power_t set_power;
    draw_module_info_t draw_module_info;
    loop_get_data_t loop_get_data;
    init_t init;
    get_data_t get_data;
    tick_t tick;
    uint64_t tick_rate; // 1000000 = 1s
} power_protocol_app_s;

typedef power_protocol_app_s power_protocol_app_t;

typedef void (*callback_function_t)(void *data, void *user_data);

typedef struct
{
    callback_function_t callback;
    void *user_data;
    uint16_t timeout;
    int64_t time;
} ack_data_t;

DICT_DEF2(ack_dict, uint16_t, M_DEFAULT_OPLIST, ack_data_t *, M_PTR_OPLIST)

void add_ack(ack_dict_t *dict, uint16_t ack_id, callback_function_t callback, void *user_data, uint16_t timeout_ms);

void call_ack(ack_dict_t *dict, uint16_t ack_id, void *data);

void del_ack(ack_dict_t *dict, uint16_t ack_id);

void check_ack_timeout(ack_dict_t *dict);

power_protocol_app_t *get_current_power_protocol();
#endif // __POWER_PROTOCOL_H__