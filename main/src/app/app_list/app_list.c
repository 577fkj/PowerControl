#include "app_list.h"
#include "mini_app_registry.h"

#include "mui_include.h"

#include "mini_app_launcher.h"
#include "mui_list_view.h"

#include "app_control_bar.h"

static uint16_t force = 0;
static uint32_t offset = 0;

typedef enum
{
    LIST_VIEW_ID_MAIN
} list_view_id_t;

static void app_list_on_run(mini_app_inst_t *p_app_inst);
static void app_list_on_kill(mini_app_inst_t *p_app_inst);
static void app_list_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event);

typedef struct
{
    mui_list_view_t *p_list_view;
    mui_view_dispatcher_t *p_view_dispatcher;
} app_list_t;

static void app_list_control_view_on_draw(void *user_data, mui_canvas_t *p_canvas)
{
    mui_canvas_set_font(p_canvas, u8g2_font_siji_t_6x10);
    mui_canvas_draw_glyph(p_canvas, 0, 14, 0xe060);
    mui_canvas_draw_glyph(p_canvas, 0, 35, 0xe105);
    mui_canvas_draw_glyph(p_canvas, 0, 57, 0xe061);
}

static void app_list_list_view_on_selected(mui_list_view_event_t event, mui_list_view_t *p_view, mui_list_item_t *p_item)
{
    mini_app_launcher_run(mini_app_launcher(), (uint32_t)p_item->user_data);
}

void app_list_on_run(mini_app_inst_t *p_app_inst)
{
    app_list_t *p_app_handle = mui_mem_malloc(sizeof(app_list_t));

    p_app_inst->p_handle = p_app_handle;
    p_app_handle->p_view_dispatcher = mui_view_dispatcher_create();
    p_app_handle->p_list_view = mui_list_view_create();

    for (uint32_t i = 0; i < mini_app_registry_get_app_num(); i++)
    {
        const mini_app_t *p_app = mini_app_registry_find_by_index(i);
        if (!p_app->hide)
        {
            mui_list_view_add_item(p_app_handle->p_list_view, p_app->icon, p_app->name, (void *)p_app->id);
        }
    }

    mui_list_view_add_item(p_app_handle->p_list_view, 0xe1f0, "返回主页", MINI_APP_ID_DESKTOP);

    mui_list_view_set_focus(p_app_handle->p_list_view, force);
    p_app_handle->p_list_view->scroll_offset = offset;

    mui_list_view_set_selected_cb(p_app_handle->p_list_view, app_list_list_view_on_selected);

    mui_view_dispatcher_add_view(p_app_handle->p_view_dispatcher, LIST_VIEW_ID_MAIN,
                                 mui_list_view_get_view(p_app_handle->p_list_view));
    mui_view_dispatcher_attach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher, LIST_VIEW_ID_MAIN);

    app_control_bar_set_draw(NULL, app_list_control_view_on_draw);
}

void app_list_on_kill(mini_app_inst_t *p_app_inst)
{
    app_list_t *p_app_handle = p_app_inst->p_handle;

    force = mui_list_view_get_focus(p_app_handle->p_list_view);
    offset = p_app_handle->p_list_view->scroll_offset;

    app_control_bar_set_draw(NULL, NULL);

    mui_view_dispatcher_detach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_free(p_app_handle->p_view_dispatcher);
    mui_list_view_free(p_app_handle->p_list_view);

    mui_mem_free(p_app_handle);

    p_app_inst->p_handle = NULL;
}

void app_list_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event) {}

const mini_app_t app_list_info = {.id = MINI_APP_ID_APP_LIST,
                                  .name = "软件列表",
                                  .icon = 0xe005,
                                  .deamon = false,
                                  .sys = true,
                                  .hide = true,
                                  .run_cb = app_list_on_run,
                                  .kill_cb = app_list_on_kill,
                                  .on_event_cb = app_list_on_event};
