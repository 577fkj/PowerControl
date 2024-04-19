#ifndef MUI_DEFINES_H
#define MUI_DEFINES_H

#define MUI_EVENT_ID_REDRAW 0
#define MUI_EVENT_ID_INPUT 1
#define MUI_EVENT_ID_ANIM 2

#define mui_check(__e)                       \
    do                                       \
    {                                        \
        if ((__e) == 0)                      \
        {                                    \
            printf("furi_check failed\r\n"); \
        }                                    \
    } while (0)

#define mui_assert(__e) \
    do                  \
    {                   \
        ((void)(__e));  \
    } while (0)

#endif