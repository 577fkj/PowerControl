#ifndef CONTEOL_BAR_VIEW_H
#define CONTEOL_BAR_VIEW_H

#include "mui_include.h"

#include "app_control_bar.h"

typedef struct
{
    mui_view_t *p_view;
    control_bar_draw_cb_t draw_cb;
    void *user_data;
} control_bar_view_t;

control_bar_view_t *control_bar_view_create();
void control_bar_view_free(control_bar_view_t *p_view);
mui_view_t *control_bar_view_get_view(control_bar_view_t *p_view);

#endif