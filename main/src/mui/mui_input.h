#ifndef MUI_INPUT_H
#define MUI_INPUT_H

#include "mui_defines.h"
#include <stdint.h>
#include "bsp_btn.h"

typedef enum
{
    INPUT_KEY_LEFT = 0,
    INPUT_KEY_CENTER = 1,
    INPUT_KEY_RIGHT = 2
} input_key_t;

typedef enum
{
    INPUT_TYPE_PRESS,
    INPUT_TYPE_RELEASE,
    INPUT_TYPE_SHORT,
    INPUT_TYPE_LONG,
    INPUT_TYPE_REPEAT
} input_type_t;

typedef struct
{
    input_key_t key;
    input_type_t type;
} mui_input_event_t;

void mui_input_init();

void mui_input_on_bsp_event(bsp_event_t evt);

#endif