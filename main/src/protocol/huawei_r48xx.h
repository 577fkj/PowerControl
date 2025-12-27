#ifndef __HUAWEI_R48XX_H__
#define __HUAWEI_R48XX_H__

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "power_protocol.h"

// Protocol id
#define HUAWEI_R48XX_PROTOCOL_ID 0x21
#define HUAWEI_C28005G1_PROTOCOL_ID 0x0E
#define HUAWEI_MPPT_PROTOCOL_ID 0x2A
// Cmdid
#define HUAWEI_R48XX_MSG_CONTROL_ID 0x80
#define HUAWEI_R48XX_MSG_CONFIG_ID 0x81
#define HUAWEI_R48XX_MSG_QUERY_ID 0x82
#define HUAWEI_R48XX_MSG_DATA_ID 0x40
#define HUAWEI_R48XX_MSG_INFO_ID 0x100
#define HUAWEI_R48XX_MSG_DESC_ID 0xD2
#define HUAWEI_R48XX_MSG_CURRENT_ID 0x11
#define HUAWEI_R48XX_SYSTEM_INIT_ID 0x10
// Cmdid end
#define MAX_CURRENT_OFFSET 20.0
#define RATIO_OFFSER 1024.0


/**
https://github.com/patagonaa/huawei-r48xx#can-id
Interpretation:
Bits: 000a aaaa abbb bbbb cccc cccc deee eefg
    0 (bit 31-29): always zero (CAN ID is 29-bit)
    a (bit 28-23): protocol ID (always 21)
    b (bit 22-16): address (0 = broadcast, 1 = first, ...)
    c (bit 15-8): command id
    d (bit 7): message source (0 = from PSU, 1 = to PSU)
    e (bit 6-2): group mask (always 1F)
    f (bit 1): hardware / software address (0 = hw, 1 = sw, always 1)
    g (bit 0): finished marker (0 = finished, 1 = more data coming)
*/

typedef struct
{
    uint8_t protoId;   // bits 28-23
    uint8_t addr;      // bits 22-16 (0x00: broadcast, 0x01: PSU 1, 0x02: PSU 2, ...)
    uint8_t cmdId;     // bits 15-8
    uint8_t fromSrc;   // bits 7     (0: from PSU, 1: to PSU)
    uint8_t groupMask; // bits 6-2   (always 0x1F)
    uint8_t addrType;  // bits 1     (0: hw address, 1: sw address, always 1)
    uint8_t finished;  // bits 0     (0: finished, 1: more data coming)
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
    uint16_t rate;
    char version[6];
    char desc[512];
    float default_output_voltage;
    float default_output_current;
    float max_output_current; // 最大输出电流
    float current_limit;      // 电流限制
    int32_t run_hour;         // 总运行时间
} RectifierInfo;

extern const power_protocol_app_t huawei_r48xx_info;
extern const power_protocol_app_t huawei_mppt_info;
extern const power_protocol_app_t huawei_c28005g1_info;

#endif // __HUAWEI_R48XX_H__