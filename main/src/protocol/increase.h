#ifndef __INCREASE_H
#define __INCREASE_H

#include "power_protocol.h"

#define INCREASE_SET_DATA_ID 0x07C
#define INCREASE_READ_DATA_ID 0x010
#define INCREASE_READ_INPUT_DATA_ID 0x07A
#define INCREASE_READ_TEMP_DATA_ID 0x008
#define INCREASE_CURRENT_SHARING_DATA_ID 0xC21

typedef struct
{
    bool is_recv;
    uint16_t command;
    uint8_t group;
    uint8_t addr;
} IncreaseEAddr;

extern const power_protocol_app_t increase_info;

#endif // __INCREASE_H