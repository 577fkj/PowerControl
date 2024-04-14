#include "power_protocol.h"

#include "huawei_r48xx.h"

const power_protocol_app_t *power_protocol_registry[] = {
    &huawei_r48xx_info,
};

const uint32_t power_protocol_num = sizeof(power_protocol_registry) / sizeof(power_protocol_registry[0]);