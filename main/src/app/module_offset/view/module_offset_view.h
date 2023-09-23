#ifndef CONTEOL_BAR_VIEW_H
#define CONTEOL_BAR_VIEW_H

#include "mui_include.h"

#include "app_module_offset.h"

#include "driver/gptimer.h"
#include "esp_timer.h"

typedef struct
{
    mui_view_t *p_view;
    esp_timer_handle_t timer;
    void *user_data;
} module_offset_view_t;

module_offset_view_t *module_offset_view_create();
void module_offset_view_free(module_offset_view_t *p_view);
mui_view_t *module_offset_view_get_view(module_offset_view_t *p_view);

#endif