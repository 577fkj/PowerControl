#ifndef __MAIN_H
#define __MAIN_H

#include <stdint.h>

#define LED_GPIO_PIN GPIO_NUM_7
#define LED2_GPIO_PIN GPIO_NUM_18

#define OLED_CS GPIO_NUM_10
#define OLED_CLK GPIO_NUM_12
#define OLED_SDA GPIO_NUM_11

#define OLED_RESET GPIO_NUM_14
#define OLED_DC GPIO_NUM_13

#define UP_KEY GPIO_NUM_17
#define CENTER_KEY GPIO_NUM_9
#define DOWN_KEY GPIO_NUM_8

void can_send(uint32_t can_id, uint8_t data[]);

#endif