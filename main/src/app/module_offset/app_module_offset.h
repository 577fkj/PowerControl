#ifndef APP_MODULE_OFFSET_H
#define APP_MODULE_OFFSET_H

#include "mini_app_defines.h"
#include "mui_canvas.h"
#include "mui_list_view.h"
#include "mui_msg_box.h"

#include "driver/gptimer.h"
#include "esp_timer.h"

typedef enum
{
    MODULE_OFFSET_LIST_VIEW_ID_MAIN,
    MODULE_OFFSET_VIEW_ID_MSG_BOX,
    MODULE_OFFSET_VIEW_ID_MAIN
} module_offset_view_id_t;

typedef struct
{
    uint8_t selection;
    void *user_data;
} module_offset_data_t;

typedef struct
{
    mui_view_t *p_view;
    esp_timer_handle_t timer;
    uint8_t count;
    float offset;
    void *user_data;
    void *old_draw_cb;
} module_offset_view_t;

typedef struct
{
    module_offset_view_t *p_module_offset_view;
    mui_list_view_t *p_list_view;
    mui_msg_box_t *p_msg_box;
    mui_view_dispatcher_t *p_view_dispatcher;
} app_module_offset_t;

static void app_module_offset_on_run(mini_app_inst_t *p_app_inst);
static void app_module_offset_on_kill(mini_app_inst_t *p_app_inst);
static void app_module_offset_on_event(mini_app_inst_t *p_app_inst,
                                       mini_app_event_t *p_event);

extern const mini_app_t app_module_offset_info;

#endif