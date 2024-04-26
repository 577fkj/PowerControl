#include "app_settings.h"
#include "mini_app_launcher.h"
#include "app_config.h"
#include "settings_scene.h"
#include "power_protocol.h"

typedef struct
{
    uint8_t selection;
    void *user_data;
} settings_scene_protocol_data_t;

static void settings_scene_protocol_msg_box_cb(mui_msg_box_event_t event, mui_msg_box_t *p_msg_box)
{
    settings_scene_protocol_data_t *data = mui_msg_box_get_user_data(p_msg_box);
    app_settings_t *app = data->user_data;
    if (event == MUI_MSG_BOX_EVENT_SELECT_RIGHT)
    {
        config_t *config = get_config();
        config->power_protocol = data->selection;
        save_config(config);
        esp_restart();
        return;
    }
    mui_scene_dispatcher_previous_scene(app->p_scene_dispatcher);
    mui_msg_box_set_user_data(app->p_msg_box, app);
    mui_mem_free(data);
}

static void settings_scene_protocol_show_msg_box(app_settings_t *app, uint8_t selection)
{
    mui_msg_box_set_header(app->p_msg_box, "提示");
    mui_msg_box_set_message(app->p_msg_box, "确定要切换电源协议吗？");
    mui_msg_box_set_btn_text(app->p_msg_box, "返回", NULL, "确定");
    mui_msg_box_set_btn_focus(app->p_msg_box, 0);
    mui_msg_box_set_event_cb(app->p_msg_box, settings_scene_protocol_msg_box_cb);
    settings_scene_protocol_data_t *data = mui_mem_malloc(sizeof(settings_scene_protocol_data_t));
    data->selection = selection;
    data->user_data = app;
    mui_msg_box_set_user_data(app->p_msg_box, data);

    mui_view_dispatcher_switch_to_view(app->p_view_dispatcher, SETTINGS_VIEW_ID_MSG_BOX);
}

static void settings_scene_protocol_list_view_on_selected(mui_list_view_event_t event, mui_list_view_t *p_list_view,
                                                          mui_list_item_t *p_item)
{
    app_settings_t *app = p_list_view->user_data;
    uint8_t selection = (uint8_t)p_item->user_data;
    config_t *config = get_config();
    if (selection < power_protocol_num)
    {
        if (config->power_protocol != selection)
        {
            settings_scene_protocol_show_msg_box(app, selection);
            return;
        }
    }

    mui_scene_dispatcher_previous_scene(app->p_scene_dispatcher);
}

void settings_scene_protocol_on_enter(void *user_data)
{

    app_settings_t *app = user_data;
    for (uint8_t i = 0; i < power_protocol_num; i++)
    {
        mui_list_view_add_item(app->p_list_view, 0xe105, power_protocol_registry[i]->name, (void *)i);
    }
    mui_list_view_add_item(app->p_list_view, 0xe069, "返回", (void *)NULL_USER_DATA);

    mui_list_view_set_selected_cb(app->p_list_view, settings_scene_protocol_list_view_on_selected);
    mui_view_dispatcher_switch_to_view(app->p_view_dispatcher, SETTINGS_VIEW_ID_MAIN);
}

void settings_scene_protocol_on_exit(void *user_data)
{
    app_settings_t *app = user_data;
    mui_list_view_clear_items(app->p_list_view);
    mui_list_view_set_selected_cb(app->p_list_view, NULL);
}
