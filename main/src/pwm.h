#ifndef __PWM_H
#define __PWM_H

#include <stdint.h>

uint16_t fan_get_speed();

void fan_set_speed(uint8_t duty);

void init_pwm(void);

#endif // PWM_H