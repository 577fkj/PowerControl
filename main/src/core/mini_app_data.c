#include "mini_app_defines.h"
#include "app_desktop.h"
#include "app_list.h"
#include "app_control_bar.h"
#include "app_module_info.h"
#include "app_input_info.h"
#include "app_output_info.h"
#include "app_settings.h"
#include "app_module_offset.h"

#include <stddef.h>

const mini_app_t *mini_app_registry[] = {
    &app_control_bar_info,
    &app_desktop_info,
    &app_list_info,
    &app_module_info_info,
    &app_input_info_info,
    &app_output_info_info,
    &app_module_offset_info,
    &app_settings_info,
};

const uint32_t mini_app_num = sizeof(mini_app_registry) / sizeof(mini_app_registry[0]);
