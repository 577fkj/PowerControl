#ifndef __BSP_H
#define __BSP_H

#include "stdint.h"

typedef enum
{
    UP_KEY_EVENT = 0,
    UP_KEY_LONG_EVENT = 1,
    CENTER_KEY_EVENT = 2,
    CENTER_KEY_LONG_EVENT = 3,
    DOWN_KEY_EVENT = 4,
    DOWN_KEY_LONG_EVENT = 5,
} bsp_event_t;

typedef enum
{
    BSP_BTN_EVENT_PRESSED,
    BSP_BTN_EVENT_RELEASED,
    BSP_BTN_EVENT_SHORT,
    BSP_BTN_EVENT_LONG,
    BSP_BTN_EVENT_REPEAT
} bsp_btn_event_t;

typedef void (*bsp_btn_event_cb_t)(uint8_t btn, bsp_btn_event_t evt);

void bsp_btn_init(bsp_btn_event_cb_t p_event_cb);

void init_key(void);

#endif