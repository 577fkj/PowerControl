#include "app_settings.h"
#include "mini_app_launcher.h"
#include "settings_scene.h"
#include "app_config.h"

#include "mui_icons.h"

typedef enum
{
    SETTINGS_MAIN_MENU_NONE,
    SETTINGS_MAIN_MENU_BACKLIGHT,
    SETTINGS_MAIN_MENU_FAN_SPEED,
    SETTINGS_MAIN_MENU_SWITCH_PROTOCOL,
    SETTINGS_MAIN_MENU_BACK,
} settings_main_menu_t;

static void settings_scene_main_list_view_on_selected(mui_list_view_event_t event, mui_list_view_t *p_list_view,
                                                      mui_list_item_t *p_item)
{
    app_settings_t *app = p_list_view->user_data;
    settings_main_menu_t selection = (settings_main_menu_t)p_item->user_data;
    switch (selection)
    {
    case SETTINGS_MAIN_MENU_BACK:
        mini_app_launcher_run(mini_app_launcher(), MINI_APP_ID_APP_LIST);
        break;

    case SETTINGS_MAIN_MENU_FAN_SPEED:
        mui_scene_dispatcher_next_scene(app->p_scene_dispatcher, SETTINGS_SCENE_FAN_SPEED);
        break;

    case SETTINGS_MAIN_MENU_BACKLIGHT:
        mui_scene_dispatcher_next_scene(app->p_scene_dispatcher, SETTINGS_SCENE_BACKLIGHT);
        break;

    case SETTINGS_MAIN_MENU_SWITCH_PROTOCOL:
        mui_scene_dispatcher_next_scene(app->p_scene_dispatcher, SETTINGS_SCENE_PROTOCOL);
        break;

    default:
        break;
    }
}

void settings_scene_main_on_enter(void *user_data)
{
    app_settings_t *app = user_data;
    mui_list_view_add_item(app->p_list_view, 0xe1c7, "系统版本 [1.0.0]", (void *)SETTINGS_MAIN_MENU_NONE);
    mui_list_view_add_item(app->p_list_view, 0xe1c8, "屏幕亮度", (void *)SETTINGS_MAIN_MENU_BACKLIGHT);
    mui_list_view_add_item(app->p_list_view, 0xe100, "风扇速度", (void *)SETTINGS_MAIN_MENU_FAN_SPEED);
    mui_list_view_add_item(app->p_list_view, 0xe1b3, "切换电源协议", (void *)SETTINGS_MAIN_MENU_SWITCH_PROTOCOL);
    mui_list_view_add_item(app->p_list_view, ICON_HOME, "返回", (void *)SETTINGS_MAIN_MENU_BACK);

    mui_list_view_set_selected_cb(app->p_list_view, settings_scene_main_list_view_on_selected);

    mui_view_dispatcher_switch_to_view(app->p_view_dispatcher, SETTINGS_VIEW_ID_MAIN);
}

void settings_scene_main_on_exit(void *user_data)
{
    app_settings_t *app = user_data;
    mui_list_view_clear_items(app->p_list_view);
    mui_list_view_set_selected_cb(app->p_list_view, NULL);
}