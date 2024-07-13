#ifndef CONTEOL_BAR_VIEW_H
#define CONTEOL_BAR_VIEW_H

#include "mui_include.h"

#include "app_module_offset.h"

module_offset_view_t *module_offset_view_create();
void module_offset_view_free(module_offset_view_t *p_view);
mui_view_t *module_offset_view_get_view(module_offset_view_t *p_view);

#endif