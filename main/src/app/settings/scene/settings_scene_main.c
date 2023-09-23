#include "app_settings.h"
#include "mini_app_launcher.h"
#include "settings_scene.h"
#include "app_config.h"

enum settings_main_menu_t
{
    SETTINGS_MAIN_MENU_BACK,
};

static void settings_scene_main_list_view_on_selected(mui_list_view_event_t event, mui_list_view_t *p_list_view,
                                                      mui_list_item_t *p_item)
{
    app_settings_t *app = p_list_view->user_data;
    uint32_t selection = (uint32_t)p_item->user_data;
    switch (selection)
    {
    case SETTINGS_MAIN_MENU_BACK:
        mini_app_launcher_run(mini_app_launcher(), MINI_APP_ID_APP_LIST);
    }
}

void settings_scene_main_on_enter(void *user_data)
{
    app_settings_t *app = user_data;
    mui_list_view_add_item(app->p_list_view, 0xe1c7, "系统版本 [1.0.0]", (void *)NULL);
    mui_list_view_add_item(app->p_list_view, 0xe1f0, "返回", (void *)SETTINGS_MAIN_MENU_BACK);

    mui_list_view_set_selected_cb(app->p_list_view, settings_scene_main_list_view_on_selected);

    mui_view_dispatcher_switch_to_view(app->p_view_dispatcher, SETTINGS_VIEW_ID_MAIN);
}

void settings_scene_main_on_exit(void *user_data)
{
    app_settings_t *app = user_data;
    mui_list_view_clear_items(app->p_list_view);
    mui_list_view_set_selected_cb(app->p_list_view, NULL);
}