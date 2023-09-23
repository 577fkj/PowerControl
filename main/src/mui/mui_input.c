#include "mui_input.h"
#include "mui_core.h"
#include "mui_defines.h"
#include "mui_event.h"
#include "bsp_btn.h"

static void mui_input_post_event(mui_input_event_t *p_input_event)
{
    uint32_t arg = p_input_event->type;
    arg <<= 8;
    arg += p_input_event->key;
    mui_event_t mui_event = {.id = MUI_EVENT_ID_INPUT, .arg_int = arg};
    mui_post(mui(), &mui_event);
}

void mui_input_on_bsp_btn_event(uint8_t btn, bsp_btn_event_t evt)
{
    switch (evt)
    {

    case BSP_BTN_EVENT_PRESSED:
    {
        printf("Key %d pressed\n", btn);
        mui_input_event_t input_event = {.key = btn, .type = INPUT_TYPE_PRESS};
        mui_input_post_event(&input_event);
        break;
    }

    case BSP_BTN_EVENT_RELEASED:
    {
        printf("Key %d released\n", btn);
        mui_input_event_t input_event = {.key = btn, .type = INPUT_TYPE_RELEASE};
        mui_input_post_event(&input_event);
        break;
    }

    case BSP_BTN_EVENT_SHORT:
    {
        printf("Key %d short push\n", btn);
        mui_input_event_t input_event = {.key = btn,
                                         .type = INPUT_TYPE_SHORT};
        mui_input_post_event(&input_event);
        break;
    }

    case BSP_BTN_EVENT_LONG:
    {
        printf("Key %d long push\n", btn);
        mui_input_event_t input_event = {.key = btn,
                                         .type = INPUT_TYPE_LONG};
        mui_input_post_event(&input_event);

        break;
    }

    case BSP_BTN_EVENT_REPEAT:
    {
        printf("Key %d repeat push\n", btn);
        mui_input_event_t input_event = {.key = btn,
                                         .type = INPUT_TYPE_REPEAT};
        mui_input_post_event(&input_event);

        break;
    }

    default:
        break;
    }
}

void mui_input_init()
{
    bsp_btn_init(mui_input_on_bsp_btn_event);
    // TODO
}
