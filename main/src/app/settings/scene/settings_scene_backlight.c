#include "app_settings.h"
#include "mini_app_launcher.h"
#include "settings_scene.h"

static void settings_scene_backlight_event_cb(mui_progress_bar_event_t event, mui_progress_bar_t *p_progress_bar)
{
    app_settings_t *app = p_progress_bar->user_data;
    uint8_t value = mui_progress_bar_get_current_value(p_progress_bar);
    if (event == MUI_PROGRESS_BAR_EVENT_DECREMENT || event == MUI_PROGRESS_BAR_EVENT_INCREMENT)
    {
        mui_t *p_mui = mui();
        u8g2_SetContrast(&p_mui->u8g2, (value - 1) * (255.0 / 99.0));
    }
    else
    {
        mui_t *p_mui = mui();
        u8g2_SetContrast(&p_mui->u8g2, (value - 1) * (255.0 / 99.0));
        mui_scene_dispatcher_previous_scene(app->p_scene_dispatcher);
    }
}

void settings_scene_backlight_on_enter(void *user_data)
{
    app_settings_t *app = user_data;
    mui_progress_bar_set_header(app->p_progress_bar, "屏幕亮度");
    mui_progress_bar_set_format(app->p_progress_bar, "%ld%%");
    mui_progress_bar_set_min_value(app->p_progress_bar, 0);
    mui_progress_bar_set_max_value(app->p_progress_bar, 100);
    mui_progress_bar_set_current_value(app->p_progress_bar, 100);
    mui_progress_bar_set_event_cb(app->p_progress_bar, settings_scene_backlight_event_cb);
    mui_view_dispatcher_switch_to_view(app->p_view_dispatcher, SETTINGS_VIEW_ID_PROGRESS_BAR);
}

void settings_scene_backlight_on_exit(void *user_data)
{
    app_settings_t *app = user_data;
    mui_progress_bar_reset(app->p_progress_bar);
}