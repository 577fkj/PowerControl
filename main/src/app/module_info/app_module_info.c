#include "app_module_info.h"
#include "mini_app_registry.h"

#include "mui_include.h"

#include "mini_app_launcher.h"
#include "mui_list_view.h"
#include "app_control_bar.h"

#include "huawei_r48xx.h"

typedef enum
{
    MODULE_INFO_VIEW_ID_MAIN
} module_info_view_id_t;

static void app_module_info_on_run(mini_app_inst_t *p_app_inst);
static void app_module_info_on_kill(mini_app_inst_t *p_app_inst);
static void app_module_info_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event);

typedef struct
{
    mui_list_view_t *p_list_view;
    mui_view_dispatcher_t *p_view_dispatcher;
} app_module_info_t;

static void module_info_control_view_on_draw(void *user_data, mui_canvas_t *p_canvas)
{
    mui_canvas_set_font(p_canvas, u8g2_font_siji_t_6x10);
    mui_canvas_draw_glyph(p_canvas, 0, 14, 0xe060);
    mui_canvas_draw_glyph(p_canvas, 0, 35, 0xe069);
    mui_canvas_draw_glyph(p_canvas, 0, 57, 0xe061);
}

int extractValue(const char *text, const char *key, char **value)
{
    const char *line = strtok((char *)text, "\n");
    while (line != NULL)
    {
        if (strstr(line, key) != NULL)
        {
            const char *delimiter = "=";
            char *token = strtok((char *)line, delimiter);
            if (token != NULL)
            {
                token = strtok(NULL, delimiter);
                if (token != NULL)
                {
                    *value = strdup(token); // 动态分配足够的内存来存储值
                    return 1;               // 返回成功
                }
            }
        }
        line = strtok(NULL, "\n");
    }

    return 0; // 没有找到匹配的键，返回失败
}

void extractTextAtIndex(const char *text, const char *delimiter, int index, char **result)
{
    char *copy = strdup(text); // 创建文本的副本以便修改
    char *token = strtok(copy, delimiter);
    int currentIndex = 0;

    while (token != NULL)
    {
        if (currentIndex == index)
        {
            *result = strdup(token); // 动态分配足够的内存来存储结果
            break;
        }

        token = strtok(NULL, delimiter);
        currentIndex++;
    }

    free(copy);
}

static void app_module_info_list_view_on_selected(mui_list_view_event_t event, mui_list_view_t *p_view, mui_list_item_t *p_item)
{
    mini_app_launcher_run(mini_app_launcher(), MINI_APP_ID_APP_LIST);
}

void app_module_info_on_run(mini_app_inst_t *p_app_inst)
{
    app_control_bar_set_draw(NULL, module_info_control_view_on_draw);

    app_module_info_t *p_app_handle = mui_mem_malloc(sizeof(app_module_info_t));

    p_app_inst->p_handle = p_app_handle;
    p_app_handle->p_view_dispatcher = mui_view_dispatcher_create();
    p_app_handle->p_list_view = mui_list_view_create();

    mui_list_view_add_item(p_app_handle->p_list_view, 0x0, "等待电源回复...", NULL);

    mui_list_view_set_selected_cb(p_app_handle->p_list_view, app_module_info_list_view_on_selected);

    mui_view_dispatcher_add_view(p_app_handle->p_view_dispatcher, MODULE_INFO_VIEW_ID_MAIN,
                                 mui_list_view_get_view(p_app_handle->p_list_view));
    mui_view_dispatcher_attach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_switch_to_view(p_app_handle->p_view_dispatcher, MODULE_INFO_VIEW_ID_MAIN);

    send_get_desc();
}

void app_module_info_on_kill(mini_app_inst_t *p_app_inst)
{
    app_module_info_t *p_app_handle = p_app_inst->p_handle;

    mui_view_dispatcher_detach(p_app_handle->p_view_dispatcher, MUI_LAYER_WINDOW);
    mui_view_dispatcher_free(p_app_handle->p_view_dispatcher);

    mui_mem_free(p_app_handle);

    p_app_inst->p_handle = NULL;
}

void app_module_info_on_event(mini_app_inst_t *p_app_inst, mini_app_event_t *p_event) {}

void app_module_info_set_info(char *info)
{
    app_module_info_t *app = mini_app_launcher_get_app_handle(mini_app_launcher(), MINI_APP_ID_MODULE_INFO);

    mui_list_view_clear_items(app->p_list_view);

    char data[strlen(info)];
    char *buffer = NULL;
    char *buffer2 = NULL;
    char buffer3[20];

    strcpy(data, info);
    if (extractValue(data, "VendorName", &buffer) == 1)
    {
        mui_list_view_add_item(app->p_list_view, 0x0, "厂商", NULL);
        mui_list_view_add_item(app->p_list_view, 0x0, buffer, NULL);
        free(buffer);
    }

    strcpy(data, info);
    if (extractValue(data, "Description", &buffer) == 1)
    {
        extractTextAtIndex(buffer, ",", 1, &buffer2);
        mui_list_view_add_item(app->p_list_view, 0x0, "型号", NULL);
        mui_list_view_add_item(app->p_list_view, 0x0, buffer2, NULL);
        free(buffer2);
        extractTextAtIndex(buffer, ",", 3, &buffer2);
        mui_list_view_add_item(app->p_list_view, 0x0, buffer2, NULL);
        free(buffer2);

        mui_list_view_add_item(app->p_list_view, 0x0, "主板型号", NULL);
        extractTextAtIndex(buffer, ",", 2, &buffer2);
        mui_list_view_add_item(app->p_list_view, 0x0, buffer2, NULL);
        free(buffer2);
        free(buffer);
    }

    strcpy(data, info);
    if (extractValue(data, "BarCode", &buffer) == 1)
    {
        mui_list_view_add_item(app->p_list_view, 0x0, "序列号", NULL);
        mui_list_view_add_item(app->p_list_view, 0x0, buffer, NULL);
        free(buffer);
    }

    strcpy(data, info);
    if (extractValue(data, "Manufactured", &buffer) == 1)
    {
        mui_list_view_add_item(app->p_list_view, 0x0, "出厂时间", NULL);
        mui_list_view_add_item(app->p_list_view, 0x0, buffer, NULL);
        free(buffer);
    }

    mui_list_view_add_item(app->p_list_view, 0x0, "运行时间 (H)", NULL);
    sprintf(buffer3, "%ld", power_data.run_hour);
    mui_list_view_add_item(app->p_list_view, 0x0, buffer3, NULL);

    mui_update(mui());
}

const mini_app_t app_module_info_info = {.id = MINI_APP_ID_MODULE_INFO,
                                         .name = "电源信息",
                                         .icon = 0xe1c5,
                                         .deamon = false,
                                         .sys = false,
                                         .hide = false,
                                         .run_cb = app_module_info_on_run,
                                         .kill_cb = app_module_info_on_kill,
                                         .on_event_cb = app_module_info_on_event};
