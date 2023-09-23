#ifndef DESKTOP_VIEW_H
#define DESKTOP_VIEW_H

#include "mui_include.h"

#include "driver/gptimer.h"
#include "esp_timer.h"

typedef enum
{
    SET_NONE,
    SET_VOLTAGE,
    SET_CURRENT
} set_event_t;

typedef struct
{
    mui_view_t *p_view;
    set_event_t set_event;
    int set_pos;
    esp_timer_handle_t data_timer_handle; // 定时器句柄
} desktop_view_t;

desktop_view_t *desktop_view_create();
void desktop_view_free(desktop_view_t *p_view);
mui_view_t *desktop_view_get_view(desktop_view_t *p_view);

#endif