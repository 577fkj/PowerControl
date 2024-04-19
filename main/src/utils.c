#include "utils.h"
#include <string.h>
// #include "huawei_r48xx.h"
#include "driver/twai.h"

#define USE_GCC_BSWAP_FUNCTIONS 1

uint16_t unpack_uint16_big_endian(const uint8_t *data)
{
#if USE_GCC_BSWAP_FUNCTIONS
    return __builtin_bswap16(*(uint16_t *)data);
#else
    uint16_t result = 0;
    result |= ((uint16_t)data[0]) << 8;
    result |= ((uint16_t)data[1]) << 0;
    return result;
#endif
}

uint32_t unpack_uint32_big_endian(const uint8_t *data)
{
#if USE_GCC_BSWAP_FUNCTIONS
    return __builtin_bswap32(*(uint32_t *)data);
#else
    uint32_t result = 0;
    result |= ((uint32_t)data[0]) << 24;
    result |= ((uint32_t)data[1]) << 16;
    result |= ((uint32_t)data[2]) << 8;
    result |= ((uint32_t)data[3]) << 0;
    return result;
#endif
}

// 将一个浮点数转换为字符串
// 参数：
//   num - 要转换的浮点数
//   precision - 小数点后保留的位数
//   str - 返回文本
// 返回值：
//   转换后的字符串
// void doubleToString(double num, int precision, char *str)
// {
//     int i = 0;
//     int int_part = (int)num;
//     double frac_part = num - int_part;

//     // 处理整数部分
//     if (int_part == 0)
//     {
//         str[i++] = '0';
//     }
//     else
//     {
//         while (int_part > 0)
//         {
//             str[i++] = int_part % 10 + '0';
//             int_part /= 10;
//         }
//     }

//     // 反转整数部分
//     int j = 0;
//     while (j < i / 2)
//     {
//         char temp = str[j];
//         str[j] = str[i - j - 1];
//         str[i - j - 1] = temp;
//         j++;
//     }

//     // 添加小数点
//     if (precision > 0)
//     {
//         str[i++] = '.';
//     }

//     // 处理小数部分
//     while (precision-- > 0)
//     {
//         frac_part *= 10;
//         int digit = (int)frac_part;
//         if (digit < 0)
//         {
//             str[i++] = '0';
//         }
//         else
//         {
//             str[i++] = digit + '0';
//         }
//         frac_part -= digit;
//     }

//     // 添加字符串结束符
//     str[i] = '\0';
// }

// void doubleToString(double value, int decimals, char *buffer)
// {
//     int intPart = (int)value;
//     double doublePart = value - intPart;

//     // 将整数部分转换为字符串
//     sprintf(buffer, "%d", intPart);

//     // 如果有小数部分，则将小数部分转换为字符串并添加到缓冲区中
//     if (decimals > 0)
//     {
//         strcat(buffer, ".");
//         char decimalBuffer[10];
//         for (int i = 0; i < decimals; i++)
//         {
//             doublePart *= 10;
//             int digit = (int)doublePart;
//             if (digit < 0)
//             {
//                 strcat(buffer, "0");
//             }
//             else
//             {
//                 doublePart -= digit;
//                 sprintf(decimalBuffer, "%d", digit);
//                 strcat(buffer, decimalBuffer);
//             }
//         }
//     }
// }

/*
 * @brief: 打印16进制数据
 * @param: data：数据
 * @param: size：数据长度
 * @return: void
 */
void hexdump(const void *data, uint16_t size)
{
    printf("\n");
    printf("**************\n");
    const uint8_t *p = (const uint8_t *)data;
    char ascii[17];
    uint16_t i, j;

    ascii[16] = '\0';

    for (i = 0; i < size; ++i)
    {
        printf("%02X ", *p);
        if (*p >= ' ' && *p <= '~')
        {
            ascii[i % 16] = *p;
        }
        else
        {
            ascii[i % 16] = '.';
        }
        ++p;

        if ((i + 1) % 8 == 0 || i + 1 == size)
        {
            printf(" ");
            if ((i + 1) % 16 == 0)
            {
                printf("|  %s \n", ascii);
            }
            else if (i + 1 == size)
            {
                ascii[(i + 1) % 16] = '\0';
                if ((i + 1) % 16 <= 8)
                {
                    printf(" ");
                }
                for (j = (i + 1) % 16; j < 16; ++j)
                {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
    printf("**************\n");
}

int starts_with(const char *str, const char *prefix)
{
    int len1 = strlen(str);
    int len2 = strlen(prefix);

    if (len1 < len2)
    {
        return 0;
    }

    return strncmp(str, prefix, len2) == 0;
}

int ends_with(char *str, char *suffix)
{
    int str_len = strlen(str);
    int suffix_len = strlen(suffix);

    if (str_len < suffix_len)
    {
        return 0;
    }

    char *ptr = strchr(str, '\0') - suffix_len;

    return strcmp(ptr, suffix) == 0;
}

void get_substring(const char *str, int start, int end, char *result)
{
    int len = strlen(str);

    if (start < 0 || end >= len || start > end)
    {
        result[0] = '\0';
        return;
    }

    int i, j;
    for (i = start, j = 0; i <= end; i++, j++)
    {
        result[j] = str[i];
    }
    result[j] = '\0';
}

double myln(double a)
{
    int N = 15; // 取了前15+1项来估算
    int k, nk;
    double x, xx, y;
    x = (a - 1) / (a + 1);
    xx = x * x;
    nk = 2 * N + 1;
    y = 1.0 / nk;
    for (k = N; k > 0; k--)
    {
        nk = nk - 2;
        y = 1.0 / nk + xx * y;
    }
    return 2.0 * x * y;
}

uint16_t get_temperature_valut(uint32_t adc)
{
    if (adc < 186)
    {
        return -400;
    }
    if (adc > 3895)
    {
        return 1250;
    }
    double ntc_value = 0.0;
    if (adc < NTC_Table[82][0])
    { // 低于 42
        for (int i = 0; i < 82; i++)
        {
            if ((adc >= NTC_Table[i][0]) && (adc <= NTC_Table[i][1]))
            {
                ntc_value = NTC_Table[i][2] + ((((adc - NTC_Table[i][0]) * 10) / (NTC_Table[i][1] - NTC_Table[i][0])) * 0.1);
                break;
            }
            else if ((adc >= NTC_Table[i - 1][1]) && (adc <= NTC_Table[i][0]))
            {
                ntc_value = NTC_Table[i - 1][2] + ((((NTC_Table[i][0] - adc) * 10) / (NTC_Table[i][0] - NTC_Table[i - 1][1])) * 0.1);
                break;
            }
        }
    }
    else if (adc > NTC_Table[82][1])
    { // 大于 42
        for (int i = 83; i < 165; i++)
        {
            if ((adc >= NTC_Table[i][0]) && (adc <= NTC_Table[i][1]))
            {
                ntc_value = NTC_Table[i][2] + ((((adc - NTC_Table[i][0]) * 10) / (NTC_Table[i][1] - NTC_Table[i][0])) * 0.1);
                break;
            }
            else if ((adc >= NTC_Table[i - 1][1]) && (adc <= NTC_Table[i][0]))
            {
                ntc_value = NTC_Table[i - 1][2] + ((((NTC_Table[i][0] - adc) * 10) / (NTC_Table[i][0] - NTC_Table[i - 1][1])) * 0.1);
                break;
            }
        }
    }
    else
    {
        ntc_value = NTC_Table[82][2] + ((((adc - NTC_Table[82][0]) * 10) / (NTC_Table[82][1] - NTC_Table[82][0])) * 0.1);
    }
    return (uint16_t)(ntc_value * 10);
}

/*
 * @brief: 将CAN数据包打印出来
 * @param: flag：0-send 1-receive
 * @param: msg：CAN数据包
 * @return: void
 */
void printf_can_msg(int flag, twai_message_t *msg) // flag：0-send 1-receive
{
    int j;
    if (flag)
        printf("Receive: ");
    else
        printf("Send   : ");
    if (msg->extd)
        printf("Extended ");
    else
        printf("Standard ");
    if (msg->rtr)
        printf("Remote Frame, ");
    else
        printf("Data  Frame,  ");
    printf("ID: 0x%x    ", (unsigned int)msg->identifier);
    if (msg->rtr == 0)
    {
        for (j = 0; j < msg->data_length_code; j++)
        {
            printf("D%d: %d\t", j, msg->data[j]);
        }
    }
    printf("\n");
}