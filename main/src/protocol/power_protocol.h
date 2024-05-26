#ifndef __POWER_PROTOCOL_H__
#define __POWER_PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>

#include "mui_list_view.h"

typedef void (*can_data_handle_t)(uint32_t can_id, uint8_t *can_data);

typedef void (*set_status_t)(bool status);

typedef void (*set_voltage_current_t)(float valtage, float current);

typedef void (*set_power_t)(float power, bool online, bool callback);

typedef void (*callback_t)(void *user_data, void *data, uint32_t size);

typedef void (*draw_module_info_t)(mui_list_view_t *p_app_handle);

typedef void (*get_desc_t)(callback_t callback, void *user_data);

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
    can_data_handle_t can_init_handle;                 // 收到第一包数据时调用
    can_data_handle_t can_data_handle;                 // 收到数据时调用
    set_status_t set_status;                           // 设置开关机状态
    set_voltage_current_t set_online_voltage_current;  // 设置在线电压电流
    set_voltage_current_t set_offline_voltage_current; // 设置离线电压电流
    draw_module_info_t draw_module_info;               // 绘制模块信息
    init_t init;                                       // 初始化
    get_data_t get_data;                               // 获取电源数据，NULL着自动获取
    tick_t tick;                                       // 定时器回调
    uint64_t tick_rate;                                // 1000000 = 1s 定时器周期
    uint32_t can_speed;                                // CAN波特率
} power_protocol_app_t;

typedef void (*callback_function_t)(void *data, void *user_data);

typedef struct
{
    callback_function_t callback;
    void *user_data;
    uint16_t timeout;
    int64_t time;
} ack_data_t;

typedef struct
{
    float voltage;
    float current;
} app_data_t;

DICT_DEF2(ack_dict, uint16_t, M_DEFAULT_OPLIST, ack_data_t *, M_PTR_OPLIST)

extern const uint32_t power_protocol_num;
extern const power_protocol_app_t *power_protocol_registry[];

extern power_protocol_data_t power_data;

void ack_init();

void add_ack(uint16_t ack_id, callback_function_t callback, void *user_data, uint16_t timeout_ms);

void call_ack(uint16_t ack_id, void *data);

void del_ack(uint16_t ack_id);

void set_current_power_protocol(uint8_t power_protocol);

power_protocol_app_t *get_current_power_protocol();
#endif // __POWER_PROTOCOL_H__