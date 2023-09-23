#ifndef APP_CONTROL_BAR_H
#define APP_CONTROL_BAR_H

#include "mini_app_defines.h"
#include "mui_canvas.h"

typedef void (*control_bar_draw_cb_t)(void *user_data, mui_canvas_t *p_canvas);

extern const mini_app_t app_control_bar_info;

void app_control_bar_set_draw(void *user_data, control_bar_draw_cb_t draw);

#endif