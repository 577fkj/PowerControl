#ifndef __HUAWEI_R48XX_H__
#define __HUAWEI_R48XX_H__

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "power_protocol.h"

// Cmdid
#define HUAWEI_R48XX_PROTOCOL_ID 0x21
#define HUAWEI_R48XX_MSG_CONTROL_ID 0x80
#define HUAWEI_R48XX_MSG_CONFIG_ID 0x81
#define HUAWEI_R48XX_MSG_QUERY_ID 0x82
#define HUAWEI_R48XX_MSG_DATA_ID 0x40
#define HUAWEI_R48XX_MSG_INFO_ID 0x100
#define HUAWEI_R48XX_MSG_DESC_ID 0xD2
#define HUAWEI_R48XX_MSG_CURRENT_ID 0x11
#define HUAWEI_R48XX_SYSTEM_INIT_ID 0x10
// Cmdid end
#define MAX_CURRENT_MULTIPLIER 20.0
#define RATIO_MULTIPLIER 1024.0

typedef struct
{
    uint8_t protoId;
    uint8_t addr;
    uint8_t cmdId;
    uint8_t fromSrc;
    uint8_t rev;
    uint8_t count;
} HuaweiEAddr;

typedef enum
{
    HUAWEI_DESC_ACK = 0x01,
} HuaweiR48xxACKId;

// typedef struct
// {
//     // 输入
//     float input_voltage;   // 输入电压
//     float input_current;   // 输入电流
//     float input_frequency; // 输入频率
//     float input_power;     // 输入功率
//     float input_temp;      // 温度

//     // 输出
//     float output_voltage;     // 输出电压
//     float output_current;     // 输出电流
//     float max_output_current; // 最大输出电流
//     float output_temp;        // 温度
//     float output_power;       // 输出功率

//     // 其他
//     float efficiency;     // 效率
//     float current_limit;  // 电流限制
//     float amp_hour;       // 每秒充电量 使用时要处以 3600
//     int32_t run_hour;     // 总运行时间
//     bool output_status;   // 输出状态
//     bool ready_status;    // 就绪状态
//     bool power_status;    // 电源状态
//     bool power_connected; // 电源连接状态
// } RectifierParameters;

typedef struct
{
    float output_voltage;
    float output_current;
} power_flash_data_t;

typedef struct
{
    uint16_t rate;
    char version[6];
    char desc[512];
    float default_output_voltage;
    float default_output_current;
    float max_output_current; // 最大输出电流
    float current_limit;      // 电流限制
    int32_t run_hour;         // 总运行时间
} RectifierInfo;

extern power_protocol_app_t huawei_r48xx_info;

// char *HuaweiEAddr_to_string(const HuaweiEAddr *self);
uint32_t HuaweiEAddr_pack(const HuaweiEAddr *self);
void can_data_handle(uint32_t can_id, uint8_t *can_data);
void HuaweiEAddr_unpack(HuaweiEAddr *self, uint32_t val);
void send_get_data();
void send_get_info();
void send_get_desc();
void set_voltage(float c, bool perm, bool callback);
void set_current(float c, bool perm, bool callback);
// void set_power(float p, bool callback);
void power_on(bool callback);
void power_off(bool callback);

#endif // __HUAWEI_R48XX_H__