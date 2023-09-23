#ifndef __HUAWEI_R48XX_H__
#define __HUAWEI_R48XX_H__

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

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

typedef struct
{
    float input_voltage;
    float input_current;
    float input_frequency;
    float input_power;
    float input_temp;
    float output_voltage;
    float output_current;
    float max_output_current;
    float output_temp;
    float output_power;
    float efficiency;
    float current_limit;
    float amp_hour;
    int32_t run_hour;
    bool output_status;
    bool ready_status;
    bool power_status;
} RectifierParameters;

// typedef struct
// {
//     char input_power[10];
//     char input_frequency[10];
//     char input_current[10];
//     char output_power[10];
//     char efficiency[10];
//     char output_voltage[10];
//     char max_output_voltage[10];
//     char max_output_current[10];
//     char input_voltage[10];
//     char output_temp[10];
//     char input_temp[10];
//     char output_current[10];
//     char amp_hour[10];
// } RectifierParametersStringData;

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
} RectifierInfo;

extern RectifierParameters power_data;
// extern RectifierParametersStringData power_data_string;
extern RectifierInfo power_info;

void HuaweiEAddr_init(HuaweiEAddr *self, uint8_t protoId, uint8_t addr, uint8_t cmdId, uint8_t fromSrc, uint8_t rev, uint8_t count);
// char *HuaweiEAddr_to_string(const HuaweiEAddr *self);
uint32_t HuaweiEAddr_pack(const HuaweiEAddr *self);
void can_data_handle(uint32_t can_id, uint8_t *can_data);
void HuaweiEAddr_unpack(HuaweiEAddr *self, uint32_t val);
void send_get_data();
void send_get_info();
void send_get_desc();
void set_voltage(float c, bool perm, bool callback);
void set_current(float c, bool perm, bool callback);
void set_power(float p, bool callback);
void power_on(bool callback);
void power_off(bool callback);

#endif // __HUAWEI_R48XX_H__