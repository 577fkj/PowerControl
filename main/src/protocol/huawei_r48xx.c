#include "huawei_r48xx.h"
#include <string.h>
#include "utils.h"
#include "app_config.h"
#include "ble_service.h"
#include "can.h"
#include "utils.h"
#include "log.h"

#include "mui_include.h"
#include "mui_list_view.h"

static RectifierInfo power_info = {0};

static uint16_t proto_id = 0x00;

int extractValue(const char *text, const char *key, char **value)
{
    char *copy = strdup(text);
    const char *line = strtok(copy, "\n");
    while (line != NULL)
    {
        if (strstr(line, key) != NULL)
        {
            const char *delimiter = "=";
            char *token = strtok((char *)line, delimiter);
            if (token != NULL)
            {
                token = strtok(NULL, delimiter);
                if (token != NULL)
                {
                    *value = strdup(token); // 动态分配足够的内存来存储值
                    return 1;               // 返回成功
                }
            }
        }
        line = strtok(NULL, "\n");
    }

    free(copy);

    return 0; // 没有找到匹配的键，返回失败
}

void extractTextAtIndex(const char *text, const char *delimiter, int index, char **result)
{
    char *copy = strdup(text); // 创建文本的副本以便修改
    char *token = strtok(copy, delimiter);
    int currentIndex = 0;

    while (token != NULL)
    {
        if (currentIndex == index)
        {
            *result = strdup(token); // 动态分配足够的内存来存储结果
            break;
        }

        token = strtok(NULL, delimiter);
        currentIndex++;
    }

    free(copy);
}

void app_module_info_set_info(void *data, void *user_data)
{
    char *info = (char *)data;
    mui_list_view_t *p_list_view = (mui_list_view_t *)user_data;

    mui_list_view_clear_items(p_list_view);

    if (!info)
    {
        mui_list_view_add_item(p_list_view, 0x0, "连接超时", NULL);
        mui_update(mui());
        return;
    }

    char *buffer = NULL;
    char *buffer2 = NULL;
    char buffer3[20];

    if (extractValue(info, "VendorName", &buffer) == 1)
    {
        mui_list_view_add_item(p_list_view, 0x0, "厂商", NULL);
        mui_list_view_add_item(p_list_view, 0x0, buffer, NULL);
        free(buffer);
    }

    if (extractValue(info, "Description", &buffer) == 1)
    {
        extractTextAtIndex(buffer, ",", 1, &buffer2);
        mui_list_view_add_item(p_list_view, 0x0, "型号", NULL);
        mui_list_view_add_item(p_list_view, 0x0, buffer2, NULL);
        free(buffer2);
        extractTextAtIndex(buffer, ",", 3, &buffer2);
        mui_list_view_add_item(p_list_view, 0x0, buffer2, NULL);
        free(buffer2);

        mui_list_view_add_item(p_list_view, 0x0, "主板型号", NULL);
        extractTextAtIndex(buffer, ",", 2, &buffer2);
        mui_list_view_add_item(p_list_view, 0x0, buffer2, NULL);
        free(buffer2);
        free(buffer);
    }

    if (extractValue(info, "BarCode", &buffer) == 1)
    {
        mui_list_view_add_item(p_list_view, 0x0, "序列号", NULL);
        mui_list_view_add_item(p_list_view, 0x0, buffer, NULL);
        free(buffer);
    }

    if (extractValue(info, "Manufactured", &buffer) == 1)
    {
        mui_list_view_add_item(p_list_view, 0x0, "出厂时间", NULL);
        mui_list_view_add_item(p_list_view, 0x0, buffer, NULL);
        free(buffer);
    }

    mui_list_view_add_item(p_list_view, 0x0, "运行时间 (H)", NULL);
    sprintf(buffer3, "%ld", power_info.run_hour);
    mui_list_view_add_item(p_list_view, 0x0, buffer3, NULL);

    mui_update(mui());
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

void huawei_r48xx_send_get_data()
{
    HuaweiEAddr addr = {
        .protoId = proto_id,
        .addr = 0x00,
        .cmdId = HUAWEI_R48XX_MSG_DATA_ID,
        .fromSrc = 0x01,
        .rev = 0x3F,
        .count = 0x00,
    };
    uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    can_send(HuaweiEAddr_pack(&addr), data, 8);
}

void send_get_info()
{
    HuaweiEAddr addr = {
        .protoId = proto_id,
        .addr = 0x00,
        .cmdId = HUAWEI_R48XX_MSG_INFO_ID,
        .fromSrc = 0x01,
        .rev = 0x3F,
        .count = 0x00,
    };
    uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    can_send(HuaweiEAddr_pack(&addr), data, 8);
}

void send_get_desc()
{
    HuaweiEAddr addr = {
        .protoId = proto_id,
        .addr = 0x00,
        .cmdId = HUAWEI_R48XX_MSG_DESC_ID,
        .fromSrc = 0x01,
        .rev = 0x3F,
        .count = 0x00,
    };
    uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    can_send(HuaweiEAddr_pack(&addr), data, 8);
}

void set_reg(uint8_t reg, uint16_t val)
{
    HuaweiEAddr addr = {
        .protoId = proto_id,
        .addr = 0x01, // 0x00 广播，电源不会回复
        .cmdId = HUAWEI_R48XX_MSG_CONTROL_ID,
        .fromSrc = 0x01,
        .rev = 0x3F,
        .count = 0x00,
    };
    uint8_t data[8] = {0x01, reg, 0x00, 0x00, 0x00, 0x00, (val >> 8) & 0xFF, val & 0xFF};
    can_send(HuaweiEAddr_pack(&addr), data, 8);
}

void power_off()
{
    HuaweiEAddr addr = {
        .protoId = proto_id,
        .addr = 0x01, // 0x00 广播，电源不会回复
        .cmdId = HUAWEI_R48XX_MSG_CONTROL_ID,
        .fromSrc = 0x01,
        .rev = 0x3F,
        .count = 0x00,
    };
    uint8_t data[8] = {0x01, 0x32, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    can_send(HuaweiEAddr_pack(&addr), data, 8);
}

void power_on()
{
    HuaweiEAddr addr = {
        .protoId = proto_id,
        .addr = 0x01, // 0x00 广播，电源不会回复
        .cmdId = HUAWEI_R48XX_MSG_CONTROL_ID,
        .fromSrc = 0x01,
        .rev = 0x3F,
        .count = 0x00,
    };
    uint8_t data[8] = {0x01, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    can_send(HuaweiEAddr_pack(&addr), data, 8);
}

void huawei_r48xx_set_current(float c, bool online)
{
    uint16_t i = c * get_config()->set_offset_current;
    uint8_t reg = online ? 0x03 : 0x04;
    set_reg(reg, i);
}

void huawei_r48xx_set_voltage(float c, bool online)
{
    uint16_t i = c * get_config()->set_offset_voltage;
    uint8_t reg = online ? 0x00 : 0x01;
    set_reg(reg, i);
}

void huawei_r48xx_set_online_voltage_current(float voltage, float current)
{
    huawei_r48xx_set_voltage(voltage, true);
    huawei_r48xx_set_current(current, true);
}

void huawei_r48xx_set_offline_voltage_current(float voltage, float current)
{
    huawei_r48xx_set_voltage(voltage, false);
    huawei_r48xx_set_current(current, false);
}

// void set_power(float p, bool perm, bool callback)
// {
//     uint16_t i = p * RATIO_MULTIPLIER;
//     set_reg(0x02, i, callback);
// }

void huawei_r48xx_can_data_handle(uint32_t can_id, uint8_t *can_data)
{
    config_t *config = get_config();
    HuaweiEAddr h_data = {0};
    HuaweiEAddr_unpack(&h_data, can_id);
    switch (h_data.cmdId)
    {
    case HUAWEI_R48XX_MSG_CURRENT_ID: // 当前输出电流
    {
        if (can_data[3] == 0x00)
        {
            // power_data.status = POWER_STATUS_POWER_ON;
            LOGI("Power ready\n");
            power_data.status = can_data[5] == 1 ? POWER_STATUS_POWER_ON : POWER_STATUS_POWER_OFF;
            power_data.output_current = unpack_uint16_big_endian(can_data + 6) / config->offset_current;
            if (h_data.fromSrc == 0x00)
            {
                power_data.amp_hours += power_data.output_current * 0.377; // 377ms
            }
        }
        else
        {
            power_data.status = POWER_STATUS_NOT_READY;
            LOGI("Power not ready\n");
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
            power_info.max_output_current = val / MAX_CURRENT_MULTIPLIER;
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
            power_info.current_limit = val / 100;
            break; // 限流点? 输出电流?
        case 0x182:
            power_data.output_current = val / config->offset_current;
            break;
        case 0x10E:
            power_info.run_hour = val;
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
            LOGI("Unknown message ID: 0x%0x\n", cid);
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
        uint16_t offset = (count - 1) * 6;
        memcpy(power_info.desc + offset, can_data + 2, 6);
        if (!h_data.count)
        {
            power_info.desc[offset + 1] = 0x00;
            call_ack(HUAWEI_DESC_ACK, power_info.desc);
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
            LOGI("%s setting on-line voltage to %.1fV\n", error ? "Error" : "Success", val / config->offset_voltage);
        }
        break;

        case 0x01: // 设置离线电压
        {
            LOGI("%s setting non-volatile (off-line) voltage to %.1fV\n", error ? "Error" : "Success", val / RATIO_MULTIPLIER);
        }
        break;

        case 0x02: // 设置过流保护
        {
            LOGI("%s setting overvoltage protection to %.1fA\n", error ? "Error" : "Success", val / RATIO_MULTIPLIER);
        }
        break;

        case 0x03: // 设置在线输出电流
        {
            LOGI("%s setting on-line current to %.1fA\n", error ? "Error" : "Success", val / config->offset_current);
        }
        break;

        case 0x04: // 设置离线输出电流
        {
            LOGI("%s setting non-volatile (off-line) current to %.1fA\n", error ? "Error" : "Success", val / MAX_CURRENT_MULTIPLIER);
        }
        break;

        case 0x32: // 开关机状态
            power_data.status = can_data[3] == 0 ? POWER_STATUS_POWER_ON : POWER_STATUS_POWER_OFF;
            LOGI("%s Change power status %s\n", error ? "Error" : "Success", can_data[3] == 0 ? "on" : "off");
            break;

        default:
            break;
        }
    }
    break;

    case HUAWEI_R48XX_SYSTEM_INIT_ID: // 首次上电信息
        break;

    default:
        LOGI("Unknown cmd: 0x%0x", h_data.cmdId);
        LOGI("Unknown id: 0x%0x", (unsigned int)can_id);
        LOGI("Unknown data:");
        hexdump(can_data, 8);
        break;
    }
}

void huawei_r48xx_set_status(bool status)
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

void huawei_r48xx_draw_module_info(mui_list_view_t *p_list_view)
{
    mui_list_view_add_item(p_list_view, 0x0, "等待电源回复...", NULL);
    send_get_desc();

    add_ack(HUAWEI_DESC_ACK, app_module_info_set_info, p_list_view, 500);
}

static int tick_count = 0;
void huawei_r48xx_tick()
{
    huawei_r48xx_send_get_data();

    tick_count++;
    config_t *config = get_config();
    switch (tick_count)
    {
    case 10:
        huawei_r48xx_set_voltage(config->set_voltage, true);
        huawei_r48xx_set_current(config->set_current, true);
        tick_count = 0;
        break;
    default:
        break;
    }
}

void huawei_r48xx_init_power_protocol()
{
    proto_id = HUAWEI_R48XX_PROTOCOL_ID;
    ack_init();
}

void huawei_mppt_init_power_protocol()
{
    proto_id = HUAWEI_MPPT_PROTOCOL_ID;
    ack_init();
}

void huawei_c28005g1_init_power_protocol()
{
    proto_id = HUAWEI_C28005G1_PROTOCOL_ID;
    ack_init();
}

void huawei_r48xx_can_init_handle()
{
    huawei_r48xx_set_status(true);

    uint8_t data[8];
    data[0] = 0x01;
    data[1] = 0x02;
    data[6] = 0xec;
    can_send(0x108181fe, data, 8);

    memset(data, 0, 8);
    data[0] = 0x01;
    data[1] = 0x09;
    can_send(0x108181fe, data, 8);

    memset(data, 0, 8);
    data[0] = 0x01;
    data[1] = 0x0c;
    can_send(0x108181fe, data, 8);

    memset(data, 0, 8);
    data[0] = 0x01;
    data[1] = 0x0d;
    can_send(0x108181fe, data, 8);

    memset(data, 0, 8);
    data[0] = 0x01;
    data[1] = 0x01;
    data[6] = 0xd6;
    can_send(0x108181fe, data, 8);

    memset(data, 0, 8);
    data[0] = 0x01;
    data[1] = 0x04;
    data[7] = 0x94;
    can_send(0x108181fe, data, 8);

    memset(data, 0, 8);
    data[0] = 0x01;
    data[1] = 0x06;
    data[6] = 0x04;
    can_send(0x108181fe, data, 8);

    memset(data, 0, 8);
    data[0] = 0x01;
    data[1] = 0x0c;
    data[7] = 0x00; // TODO: soft_start_time
    can_send(0x108180fe, data, 8);
}

const power_protocol_app_t huawei_r48xx_info = {
    .name = "Huawei r48xx",
    .can_init_handle = huawei_r48xx_can_init_handle,
    .can_data_handle = huawei_r48xx_can_data_handle,
    .set_status = huawei_r48xx_set_status,
    .set_online_voltage_current = huawei_r48xx_set_online_voltage_current,
    .set_offline_voltage_current = huawei_r48xx_set_offline_voltage_current,
    .draw_module_info = huawei_r48xx_draw_module_info,
    .init = huawei_r48xx_init_power_protocol,
    .get_data = NULL,
    .tick = huawei_r48xx_tick,
    .tick_rate = 1000000,
    .can_speed = 125000,
};

const power_protocol_app_t huawei_mppt_info = {
    .name = "Huawei MPPT",
    .can_init_handle = huawei_r48xx_can_init_handle,
    .can_data_handle = huawei_r48xx_can_data_handle,
    .set_status = huawei_r48xx_set_status,
    .set_online_voltage_current = huawei_r48xx_set_online_voltage_current,
    .set_offline_voltage_current = huawei_r48xx_set_offline_voltage_current,
    .draw_module_info = huawei_r48xx_draw_module_info,
    .init = huawei_mppt_init_power_protocol,
    .get_data = NULL,
    .tick = huawei_r48xx_tick,
    .tick_rate = 1000000,
    .can_speed = 125000,
};

const power_protocol_app_t huawei_c28005g1_info = {
    .name = "Huawei C28005G1",
    .can_init_handle = huawei_r48xx_can_init_handle,
    .can_data_handle = huawei_r48xx_can_data_handle,
    .set_status = huawei_r48xx_set_status,
    .set_online_voltage_current = huawei_r48xx_set_online_voltage_current,
    .set_offline_voltage_current = huawei_r48xx_set_offline_voltage_current,
    .draw_module_info = huawei_r48xx_draw_module_info,
    .init = huawei_c28005g1_init_power_protocol,
    .get_data = NULL,
    .tick = huawei_r48xx_tick,
    .tick_rate = 1000000,
    .can_speed = 125000,
};
