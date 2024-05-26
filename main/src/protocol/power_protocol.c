#include "power_protocol.h"

#include "utils.h"
#include "esp_timer.h"

#include "freertos/semphr.h"

static ack_dict_t ack_dict;
static esp_timer_handle_t ack_check_timer_handle = 0;
static bool ack_lock = false;

void start_ack_check_timer()
{
    esp_err_t err = esp_timer_start_periodic(ack_check_timer_handle, MS2US(50));
    switch (err)
    {
    case ESP_OK:
        break;
    case ESP_ERR_INVALID_STATE:
        break;
    default:
        ESP_ERROR_CHECK(err);
        break;
    }
}

void stop_ack_check_timer()
{
    esp_err_t err = esp_timer_stop(ack_check_timer_handle);
    switch (err)
    {
    case ESP_OK:
        break;
    case ESP_ERR_INVALID_STATE:
        break;
    default:
        ESP_ERROR_CHECK(err);
        break;
    }
}

/*
回调队列
timeout_ms 超出这个时间还未调用着自动调用回调函数，并设置data为NULL
*/
void add_ack(uint16_t ack_id, callback_function_t callback, void *user_data, uint16_t timeout_ms)
{
    ack_lock = true;

    ack_data_t **aack_data = ack_dict_get(ack_dict, ack_id);
    ack_data_t *ack_data = NULL;
    if (ack_data)
    {
        ack_data = *aack_data;
    }
    if (!ack_data)
    {
        ack_data = malloc(sizeof(ack_data_t));
    }
    ack_data->user_data = user_data;
    ack_data->callback = callback;
    ack_data->timeout = MS2US(timeout_ms);
    ack_data->time = esp_timer_get_time();
    ack_dict_set_at(ack_dict, ack_id, ack_data);

    start_ack_check_timer();

    ack_lock = false;
}

void call_ack(uint16_t ack_id, void *data)
{
    ack_lock = true;

    ack_data_t **aack_data = ack_dict_get(ack_dict, ack_id);
    ack_data_t *ack_data = NULL;
    if (ack_data)
    {
        ack_data = *aack_data;
    }
    if (ack_data)
    {
        if (ack_data->time + ack_data->timeout >= esp_timer_get_time())
            ack_data->callback(data, ack_data->user_data);
        ack_data->callback = NULL;
        ack_data->user_data = NULL;
        free(ack_data);
        ack_dict_erase(ack_dict, ack_id);
    }

    if (ack_dict_size(ack_dict) == 0)
    {
        stop_ack_check_timer();
    }

    ack_lock = false;
}

void del_ack(uint16_t ack_id)
{
    ack_lock = true;

    ack_data_t **aack_data = ack_dict_get(ack_dict, ack_id);
    ack_data_t *ack_data = NULL;
    if (ack_data)
    {
        ack_data = *aack_data;
    }
    if (ack_data)
    {
        ack_data->callback = NULL;
        ack_data->user_data = NULL;
        free(ack_data);
        ack_dict_erase(ack_dict, ack_id);
    }

    if (ack_dict_size(ack_dict) == 0)
    {
        stop_ack_check_timer();
    }

    ack_lock = false;
}

void check_ack_timeout()
{
    if (ack_lock)
    {
        return;
    }

    if (ack_dict_size(ack_dict) == 0)
    {
        return;
    }

    ack_dict_it_t it;
    ack_dict_it(it, ack_dict);
    while (!ack_dict_end_p(it))
    {
        ack_dict_itref_t *item = ack_dict_ref(it);
        if (item == NULL || item->key == NULL || item->value == NULL)
        {
            ack_dict_next(it);
            continue;
        }
        ack_data_t *ack_data = item->value;
        if (ack_data->time + ack_data->timeout < esp_timer_get_time())
        {
            ack_data->callback(NULL, ack_data->user_data);
            ack_data->callback = NULL;
            ack_data->user_data = NULL;
            free(ack_data);
            ack_dict_erase(ack_dict, item->key);
        }
        ack_dict_next(it);
    }
}

void ack_init()
{
    ack_dict_init(ack_dict);

    create_timer_with_handle(&ack_check_timer_handle, ack_check, &check_ack_timeout, NULL);
}

#include "huawei_r48xx.h"
#include "eps6020.h"
#include "increase.h"
#include "infy.h"
#include "zte3000.h"
#include "zte4875.h"

power_protocol_data_t power_data = {0}; // global power data

power_protocol_data_t *get_data()
{
    return &power_data;
}

const power_protocol_app_t *power_protocol_registry[] = {
    &huawei_r48xx_info,
    &huawei_mppt_info,
    &huawei_c28005g1_info,
    &eps_6020_info,
    &increase_info,
    &infy_info,
    &zte3000_info,
    &zte4875_info,
};

const uint32_t power_protocol_num = sizeof(power_protocol_registry) / sizeof(power_protocol_registry[0]);

power_protocol_app_t *current_power_protocol;

void set_current_power_protocol(uint8_t power_protocol)
{
    if (power_protocol < power_protocol_num)
    {
        current_power_protocol = power_protocol_registry[power_protocol];
    }
    else
    {
        current_power_protocol = power_protocol_registry[0];
    }

    if (!current_power_protocol->get_data)
    {
        current_power_protocol->get_data = get_data;
    }
}

power_protocol_app_t *get_current_power_protocol()
{
    return current_power_protocol;
}