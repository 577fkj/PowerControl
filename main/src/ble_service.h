#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include "ble.h"

#include "df_buffer.h"

typedef enum
{
    OK = 0x1000,                  // 正常
    DATA_ERROR = 0x1001,          // 数据错误
    DATA_CHECK_ERROR = 0x1002,    // 数据校验错误
    TOKEN_CHECK_ERROR = 0x1003,   // token校验错误
    CAN_CONNECT_ERROR = 0x1004,   // CAN通信异常
    DATA_TOO_LONG_ERROR = 0x1005, // 分片数据过长
    MODE_ERROR = 0x1006,          // 模式错误
    BLE_NEXT_CHUNK = 0x1007,      // 下一分片
} ble_app_error_t;

typedef enum
{
    BLE_APP_MODE_NONE = 0x00,
    BLE_APP_MODE_DATA = 0x01,   // 设置电源数据
    BLE_APP_MODE_TOKEN = 0x02,  // 验证相关
    BLE_APP_MODE_CONFIG = 0x03, // 配置相关
    BLE_APP_MODE_MAX = 0x04
} ble_app_mode_t;

typedef void (*ble_app_data_handle_t)(uint8_t *data, uint16_t len);

typedef enum
{
    POWER_GET_RAW_DATA = 0x00,  // 获取电源原始数据
    POWER_GET_BASE_DATA = 0x01, // 获取电源基础数据

    POWER_SET_DATA_VOLTAGE = 0x02,     // 设置电压
    POWER_SET_DATA_CURRENT = 0x03,     // 设置电流
    POWER_SET_DATA_POWER = 0x04,       // 设置功率
    POWER_SET_DATA_SWITCH = 0x05,      // 设置开关状态
    POWER_SET_DATA_VOLTAGE_MAX = 0x06, // 设置电压上限
    POWER_SET_DATA_VOLTAGE_MIN = 0x07, // 设置电压下限
    POWER_SET_DATA_CURRENT_MAX = 0x08, // 设置电流上限
    POWER_SET_OFFLINE_VOLTAGE = 0x09,  // 设置离线电压
    POWER_SET_OFFLINE_CURRENT = 0x0A,  // 设置离线电流

} ble_app_power_data_t;

typedef enum
{
    TOKEN_CHECK_TOKEN = 0x00, // 验证token
    TOKEN_SET_TOKEN = 0x01,   // 设置token
} ble_app_token_data_t;

typedef enum
{
    CONFIG_SET_OUTPUT_VOLTAGE_OFFSET = 0x00, // 设置电压偏移
    CONFIG_GET_OUTPUT_VOLTAGE_OFFSET = 0x01, // 获取电压偏移

    CONFIG_SET_OUTPUT_CURRENT_OFFSET = 0x02, // 设置电流偏移
    CONFIG_GET_OUTPUT_CURRENT_OFFSET = 0x03, // 获取电流偏移

    CONFIG_SET_INPUT_VOLTAGE_OFFSET = 0x04, // 设置输入电压偏移
    CONFIG_GET_INPUT_VOLTAGE_OFFSET = 0x05, // 获取输入电压偏移

    CONFIG_SET_INPUT_CURRENT_OFFSET = 0x06, // 设置输入电流偏移
    CONFIG_GET_INPUT_CURRENT_OFFSET = 0x07, // 获取输入电流偏移

    CONFIG_SET_OTHER_OFFSET = 0x08, // 设置其他偏移
    CONFIG_GET_OTHER_OFFSET = 0x09, // 获取其他偏移
} ble_app_config_data_t;

typedef struct
{
    uint16_t data_len;
    uint8_t data[4096];
} ble_app_chunk_t;

/*
数据结构体
--------------------------------------------
- 字段名 - 类型 - 长度(字节) -      说明     -
--------------------------------------------
- xor    - uint8  - 1      - 校验          -
- mode   - uint8  - 1      - 模式           -
- status - uint16 - 2      - 状态码         -
- chunk  - uint8  - 1      - 分片号(0无分片) -
- c_end  - uint8  - 1      - 是否结束分片    -
- data   - bytes  - N      - 数据           -
--------------------------------------------
注: 以上数据结构体为数据包的格式
    加密请在data字段加密，分片请在data字段分片
    状态码请求包无效（默认值0），状态码响应包有效
    分包：
        分片号每次传输加1
        c_end 为 1 时还有数据 0 结束分片
        为了提升传输效率
        分片最后一个包也请带上数据并设置 c_end 为 0
        分片最大长度为4096字节
    校验为mode到data的xor

模式 DATA(0x01)
--------------------------------------------
- 字段名 - 类型 - 长度(字节) -      说明     -
--------------------------------------------
- mode   - uint8  - 1      - 模式           -
- data   - bytes  - N      - 数据           -
--------------------------------------------

模式 TOKEN(0x02)
--------------------------------------------
- 字段名 - 类型 - 长度(字节) -      说明     -
--------------------------------------------
- mode   - uint8  - 1      - 模式           -
- data   - bytes  - N      - 数据           -
--------------------------------------------
注: data字段为8字节token

模式 CONFIG(0x03)
--------------------------------------------
- 字段名 - 类型 - 长度(字节) -      说明     -
--------------------------------------------
- mode   - uint8  - 1      - 模式           -
- data   - bytes  - N      - 数据           -
--------------------------------------------
*/

void init_ble_service(void);
void app_data_handle(uint8_t *ble_data, uint16_t len, void *user_data);
void app_ble_set_callback(uint8_t mode, bool success, uint8_t *data, uint8_t len);
bool send_app_data(uint8_t mode, uint16_t status, buffer_t *buf);

#endif // BLE_SERVICE_H
