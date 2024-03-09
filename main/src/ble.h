#ifndef __BLE_H
#define __BLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_bt_defs.h"
#include "esp_gatt_defs.h"

enum
{
    IDX_SVC,
    IDX_CHAR_A,
    IDX_CHAR_VAL_A,
    IDX_CHAR_CFG_A,

    IDX_CHAR_B,
    IDX_CHAR_VAL_B,

    IDX_CHAR_C,
    IDX_CHAR_VAL_C,

    HRS_IDX_NB,
};

typedef void (*ble_data_callback_t)(uint8_t *data, uint16_t len, void *user_data);
typedef void (*ble_connected_callback_t)(uint8_t *mac, void *user_data);

typedef struct
{
    esp_gatt_if_t gatts_if;
    uint16_t conn_id;
    bool connected;
    bool is_notify;
    uint8_t esp_bd_addr[ESP_BD_ADDR_LEN];
    ble_data_callback_t callback;
    void *callback_user_data;
    ble_connected_callback_t connected_callback;
    void *connected_user_data;
    ble_connected_callback_t disconnected_callback;
    void *disconnected_user_data;
} ble_gatts_t;

void ble_init(void);
void set_ble_data_callback(ble_data_callback_t callback, void *user_data);
void set_ble_connected_callback(ble_connected_callback_t callback, void *user_data);
void set_ble_disconnect_callback(ble_connected_callback_t callback, void *user_data);
bool is_notify(void);
bool is_connected(void);
bool ble_send_data(uint8_t *data, uint16_t len);

#endif