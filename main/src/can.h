#ifndef __CAN_H
#define __CAN_H

#include "driver/twai.h"

#define TX_GPIO_PIN GPIO_NUM_5
#define RX_GPIO_PIN GPIO_NUM_4

void can_init();
static void twai_receive_task(void *arg);
void can_send(uint32_t can_id, uint8_t data[], uint8_t data_length);

#endif