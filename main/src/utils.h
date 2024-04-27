#ifndef __UTILS_H
#define __UTILS_H

#include <stdint.h>
// #include "huawei_r48xx.h"

#define US2MS(us) ((us) / 1000)
#define MS2US(ms) ((ms) * 1000)

#define create_timer(timer, callback_, user_data) \
    esp_timer_create_args_t timer##_args = {      \
        .callback = (callback_),                  \
        .arg = (user_data),                       \
        .name = #timer "_timer"};                 \
    esp_timer_handle_t timer##_timer_handle = 0;  \
    esp_err_t timer_##timer##_err = esp_timer_create(&timer##_args, &timer##_timer_handle);

#define create_timer_with_handle(handle, timer, callback_, user_data) \
    esp_timer_create_args_t timer##_args = {                          \
        .callback = (callback_),                                      \
        .arg = (user_data),                                           \
        .name = #timer "_timer"};                                     \
    esp_err_t timer_##timer##_err = esp_timer_create(&timer##_args, handle);

uint16_t unpack_uint16_big_endian(const uint8_t *data);
uint32_t unpack_uint32_big_endian(const uint8_t *data);
uint16_t unpack_uint16_little_endian(const uint8_t *data);
uint32_t unpack_uint32_little_endian(const uint8_t *data);
// void doubleToString(double num, int precision, char *str);
void hexdump(const void *data, uint16_t size);
int starts_with(const char *str, const char *prefix);
void get_substring(const char *str, int start, int end, char *result);
int ends_with(char *str, char *suffix);

#endif