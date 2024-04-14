#include "huawei_r48xx.h"
#include "main.h"
#include <string.h>
#include "utils.h"
#include "mui_include.h"
#include "app_module_info.h"
#include "app_config.h"
#include "ble_service.h"

RectifierParameters power_data = {
    0,
    .power_status = true};
RectifierInfo power_info = {0};

void HuaweiEAddr_init(HuaweiEAddr *self, uint8_t protoId, uint8_t addr, uint8_t cmdId, uint8_t fromSrc, uint8_t rev, uint8_t count)
{
    self->protoId = protoId;
    self->addr = addr;
    self->cmdId = cmdId;
    self->fromSrc = fromSrc;
    self->rev = rev;
    self->count = count;
}

// char *HuaweiEAddr_to_string(const HuaweiEAddr *self)
// {
//     char result[80];
//     snprintf(result, 80, "protoId: 0x%02X, addr: 0x%02X, cmdId: 0x%02X, fromSrc: 0x%02X, rev: 0x%02X, count: 0x%02X",
//              self->protoId, self->addr, self->cmdId, self->fromSrc, self->rev, self->count);
//     return result;
// }

uint32_t HuaweiEAddr_pack(const HuaweiEAddr *self)
{
    return ((uint32_t)self->protoId << 23) | ((uint32_t)self->addr << 16) | ((uint32_t)self->cmdId << 8) |
           ((uint32_t)self->fromSrc << 7) | ((uint32_t)self->rev << 1) | ((uint32_t)self->count);
}

void HuaweiEAddr_unpack(HuaweiEAddr *self, uint32_t val)
{
    self->protoId = (val >> 23) & 0x3F;
    self->addr = (val >> 16) & 0x7F;
    self->cmdId = (val >> 8) & 0xFF;
    self->fromSrc = (val >> 7) & 0x01;
    self->rev = (val >> 1) & 0x3F;
    self->count = val & 0x01;
}

void send_get_data()
{
    HuaweiEAddr addr = {0};
    HuaweiEAddr_init(&addr, HUAWEI_R48XX_PROTOCOL_ID, 0x00, HUAWEI_R48XX_MSG_DATA_ID, 0x01, 0x3F, 0x00);
    can_send(HuaweiEAddr_pack(&addr), (uint8_t[]){0, 0, 0, 0, 0, 0, 0, 0});
}

void send_get_info()
{
    HuaweiEAddr addr = {0};
    HuaweiEAddr_init(&addr, HUAWEI_R48XX_PROTOCOL_ID, 0x00, HUAWEI_R48XX_MSG_INFO_ID, 0x01, 0x3F, 0x00);
    can_send(HuaweiEAddr_pack(&addr), (uint8_t[]){0, 0, 0, 0, 0, 0, 0, 0});
}

void send_get_desc()
{
    HuaweiEAddr addr = {0};
    HuaweiEAddr_init(&addr, HUAWEI_R48XX_PROTOCOL_ID, 0x00, HUAWEI_R48XX_MSG_DESC_ID, 0x01, 0x3F, 0x00);
    can_send(HuaweiEAddr_pack(&addr), (uint8_t[]){0, 0, 0, 0, 0, 0, 0, 0});
}

void set_reg(uint8_t reg, uint16_t val, bool callback)
{
    HuaweiEAddr addr = {0};
    HuaweiEAddr_init(&addr, HUAWEI_R48XX_PROTOCOL_ID, callback ? 0x01 : 0x00, HUAWEI_R48XX_MSG_CONTROL_ID, 0x01, 0x3F, 0x00);
    can_send(HuaweiEAddr_pack(&addr), (uint8_t[]){0x01, reg, 0x00, 0x00, 0x00, 0x00, (val >> 8) & 0xFF, val & 0xFF});
}

void power_off(bool callback)
{
    HuaweiEAddr addr = {0};
    HuaweiEAddr_init(&addr, HUAWEI_R48XX_PROTOCOL_ID, callback ? 0x01 : 0x00, HUAWEI_R48XX_MSG_CONTROL_ID, 0x01, 0x3F, 0x00);
    can_send(HuaweiEAddr_pack(&addr), (uint8_t[]){0x01, 0x32, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00});
}

void power_on(bool callback)
{
    HuaweiEAddr addr = {0};
    HuaweiEAddr_init(&addr, HUAWEI_R48XX_PROTOCOL_ID, callback ? 0x01 : 0x00, HUAWEI_R48XX_MSG_CONTROL_ID, 0x01, 0x3F, 0x00);
    can_send(HuaweiEAddr_pack(&addr), (uint8_t[]){0x01, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
}

void set_current(float c, bool perm, bool callback)
{
    uint16_t i = c * get_config()->set_offset_current;
    uint8_t reg = perm ? 0x04 : 0x03;
    set_reg(reg, i, callback);
}

void set_voltage(float c, bool perm, bool callback)
{
    uint16_t i = c * get_config()->set_offset_voltage;
    uint8_t reg = perm ? 0x01 : 0x00;
    set_reg(reg, i, callback);
}

void set_power(float p, bool callback)
{
    uint16_t i = p * RATIO_MULTIPLIER;
    set_reg(0x02, i, callback);
}

void can_data_handle(uint32_t can_id, uint8_t *can_data)
{
    power_data.power_connected = true;

    ConfigStruct *config = get_config();
    HuaweiEAddr h_data = {0};
    HuaweiEAddr_unpack(&h_data, can_id);
    switch (h_data.cmdId)
    {
    case HUAWEI_R48XX_MSG_CURRENT_ID: // 当前输出电流
    {
        if (can_data[3] == 0x00)
        {
            power_data.ready_status = true;
        }
        power_data.output_status = can_data[5] == 1;
        power_data.output_current = unpack_uint16_big_endian(can_data + 6) / config->offset_current;
        if (h_data.fromSrc == 0x00)
        {
            power_data.amp_hour += power_data.output_current * 0.377; // 377ms
        }
    }
    break;

    case HUAWEI_R48XX_MSG_DATA_ID: // 电源信息
    {
        uint16_t cid = unpack_uint16_big_endian(can_data) & ~0x3000;
        uint32_t val = unpack_uint32_big_endian(can_data + 4);
        // printf("cid: 0x%0x val: %ld\n", cid, val);
        switch (cid)
        {
        case 0x170:
            power_data.input_power = val / RATIO_MULTIPLIER;
            break;
        case 0x171:
            power_data.input_frequency = val / RATIO_MULTIPLIER;
            break;
        case 0x172:
            power_data.input_current = val / config->offset_current_in;
            break;
        case 0x173:
            power_data.output_power = val / RATIO_MULTIPLIER;
            break;
        case 0x174:
            power_data.efficiency = (val / RATIO_MULTIPLIER) * 100;
            break;
        case 0x175:
            power_data.output_voltage = val / config->offset_voltage;
            break;
        case 0x176:
            power_data.max_output_current = val / MAX_CURRENT_MULTIPLIER;
            break;
        case 0x178:
            power_data.input_voltage = val / config->offset_voltage_in;
            break;
        case 0x17F:
            power_data.output_temp = val / RATIO_MULTIPLIER;
            break;
        case 0x180:
            power_data.input_temp = val / RATIO_MULTIPLIER;
            break;
        case 0x181:
            // hexdump(can_data, 8);
            power_data.current_limit = val / 100;
            break; // 限流点? 输出电流?
        case 0x182:
            power_data.output_current = val / config->offset_current;
            break;
        case 0x10E:
            power_data.run_hour = val;
            break;
        case 0x000: // 数据开始?
            break;
        case 0x183:
            // printf("Input Voltage %.02fV @ %.02fHz\n", power_data.input_voltage, power_data.input_frequency);
            // printf("Input Current %.02fA\n", power_data.input_current);
            // printf("Input Power %.02fW\n\n", power_data.input_power);
            // printf("Output Voltage %.02fV\n", power_data.output_voltage);
            // printf("Output Current %.02fA of %.02fA Max, %.03fAh\n", power_data.output_current, power_data.max_output_current, power_data.amp_hour / 3600);
            // printf("Output Power %.02fW\n\n", power_data.output_power);
            // printf("Input Temperature %.01f DegC\n", power_data.input_temp);
            // printf("Output Temperature %.01f DegC\n", power_data.output_temp);
            // printf("Efficiency %.01f%%\n\n", power_data.efficiency * 100);
            // printf("System ready: %s\n", power_data.ready_status ? "Yes" : "No");
            // printf("Output Status %s\n", power_data.output_status ? "ON" : "OFF");
            // printf("Run time %d years, %d months, %d days, %d hours\n\n",
            //        (int)(power_data.run_hour / (365 * 24)),               /* 计算年数 */
            //        (int)((power_data.run_hour % (365 * 24)) / (30 * 24)), /* 计算月数 */
            //        (int)((power_data.run_hour % (30 * 24)) / 24),         /* 计算天数 */
            //        (int)(power_data.run_hour % 24));                      /* 计算小时数 */
            // doubleToString(power_data.input_voltage, 2, power_data_string.input_voltage);
            // doubleToString(power_data.input_frequency, 2, power_data_string.input_frequency);
            // doubleToString(power_data.input_current, 2, power_data_string.input_current);
            // doubleToString(power_data.input_power, 2, power_data_string.input_power);
            // doubleToString(power_data.output_voltage, 2, power_data_string.output_voltage);
            // doubleToString(power_data.output_current, 2, power_data_string.output_current);
            // doubleToString(power_data.max_output_current, 2, power_data_string.max_output_current);
            // doubleToString(power_data.output_power, 2, power_data_string.output_power);
            // doubleToString(power_data.input_temp, 2, power_data_string.input_temp);
            // doubleToString(power_data.output_temp, 2, power_data_string.output_temp);
            // doubleToString(power_data.efficiency, 2, power_data_string.efficiency);
            // doubleToString(power_data.amp_hour / 3600, 3, power_data_string.amp_hour);
            // printf("Input Voltage %sV @ %sHz\n", power_data_string.input_voltage, power_data_string.input_frequency);
            // printf("Input Current %sA\n", power_data_string.input_current);
            // printf("Input Power %sW\n\n", power_data_string.input_power);
            // printf("Output Voltage %sV\n", power_data_string.output_voltage);
            // printf("Output Current %sA of %sA Max, %.03fAh\n", power_data_string.output_current, power_data_string.max_output_current, power_data.amp_hour / 3600);
            // printf("Output Power %sW\n\n", power_data_string.output_power);
            // printf("Input Temperature %s DegC\n", power_data_string.input_temp);
            // printf("Output Temperature %s DegC\n", power_data_string.output_temp);
            // printf("Efficiency %s%%\n\n", power_data_string.efficiency);
            // printf("System ready: %s\n", power_data.ready_status ? "Yes" : "No");
            // printf("Output Status %s\n", power_data.output_status ? "ON" : "OFF");
            // printf("Run time %d years, %d months, %d days, %d hours\n\n",
            //        (int)(power_data.run_hour / (365 * 24)),               /* 计算年数 */
            //        (int)((power_data.run_hour % (365 * 24)) / (30 * 24)), /* 计算月数 */
            //        (int)((power_data.run_hour % (30 * 24)) / 24),         /* 计算天数 */
            //        (int)(power_data.run_hour % 24));                      /* 计算小时数 */
            break; // 数据结束
        default:
            printf("Unknown message ID: 0x%0x\n", cid);
            break; // 未知
        }
    }
    break;

    case HUAWEI_R48XX_MSG_INFO_ID: // 电源内部信息
    {
        uint16_t sig = unpack_uint16_big_endian(can_data);
        switch (sig)
        {
        case 0x01:
            power_info.rate = ((unpack_uint32_big_endian(can_data + 4) >> 16) & 0x03FF) >> 1;
            break;

        case 0x05:
            for (uint32_t i = 0; i >= 5; i++)
            {
                power_info.version[i] = can_data[i] + 0x30;
            }
            power_info.version[5] = 0x00;
        default:
            break;
        }
    }
    break;

    case HUAWEI_R48XX_MSG_DESC_ID: // 电源型号
    {
        uint16_t count = unpack_uint16_big_endian(can_data);
        if (count == 1)
        {
            memset(power_info.desc, 0, 256);
        }
        memcpy(power_info.desc + ((count - 1) * 6), can_data + 2, 6);
        if (!h_data.count)
        {
            power_info.desc[((count - 1) * 6) + 1] = 0x00;
            app_module_info_set_info(power_info.desc);
        }
    }
    break;

    case HUAWEI_R48XX_MSG_CONFIG_ID: // 默认配置
    {
        uint16_t mode = unpack_uint16_big_endian(can_data);
        uint32_t val = unpack_uint32_big_endian(can_data + 4);
        switch (mode)
        {
        case 0x01:
        {
            power_info.default_output_voltage = val / RATIO_MULTIPLIER;
        }
        case 0x04:
        {
            power_info.default_output_current = val / MAX_CURRENT_MULTIPLIER;
        }
        break;

        default:
            break;
        }
    }
    break;

    case HUAWEI_R48XX_MSG_CONTROL_ID: // 设置回调
    {
        uint8_t error = can_data[0] & 0x20;
        uint8_t mode = can_data[1];
        uint32_t val = unpack_uint32_big_endian(can_data + 4);
        app_ble_set_callback(mode, error, can_data, 8);
        switch (mode)
        {
        case 0x00: // 设置在线电压
        {
            printf("%s setting on-line voltage to %.1fV\n", error ? "Error" : "Success", val / config->offset_voltage);
        }
        break;

        case 0x01: // 设置离线电压
        {
            printf("%s setting non-volatile (off-line) voltage to %.1fV\n", error ? "Error" : "Success", val / RATIO_MULTIPLIER);
        }
        break;

        case 0x02: // 设置过流保护
        {
            printf("%s setting overvoltage protection to %.1fA\n", error ? "Error" : "Success", val / RATIO_MULTIPLIER);
        }
        break;

        case 0x03: // 设置在线输出电流
        {
            printf("%s setting on-line current to %.1fA\n", error ? "Error" : "Success", val / config->offset_current);
        }
        break;

        case 0x04: // 设置离线输出电流
        {
            printf("%s setting non-volatile (off-line) current to %.1fA\n", error ? "Error" : "Success", val / MAX_CURRENT_MULTIPLIER);
        }
        break;

        case 0x32: // 开关机状态
            power_data.power_status = can_data[3] == 0;
            printf("%s Change power status %s\n", error ? "Error" : "Success", can_data[3] == 0 ? "on" : "off");
            break;

        default:
            break;
        }
    }
    break;

    case HUAWEI_R48XX_SYSTEM_INIT_ID: // 首次上电信息
        break;

    default:
        printf("Unknown cmd: 0x%0x", h_data.cmdId);
        printf("Unknown id: 0x%0x", (unsigned int)can_id);
        printf("Unknown data:");
        hexdump(can_data, 8);
        break;
    }
}

void init_power_protocol()
{
}

void set_status(bool status)
{
    if (status)
    {
        power_on(true);
    }
    else
    {
        power_off(true);
    }
}

power_protocol_app_t power_protocol_num = {
    .name = "Huawei r48xx",
    .can_data_handle = can_data_handle,
    .init = init_power_protocol,
    .set_status = set_status,
    .set_current = set_current,
    .set_voltage = set_voltage,
    .set_power = set_power,
    .get_info = send_get_info,
};