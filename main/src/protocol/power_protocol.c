#include "power_protocol.h"

#include "huawei_r48xx.h"

#include "utils.h"
#include "esp_timer.h"

#include "freertos/semphr.h"

SemaphoreHandle_t ack_lock; // 声明一个互斥锁

/*
回调队列
timeout_ms 超出这个时间还未调用着自动调用回调函数，并设置data为NULL
*/
void add_ack(ack_dict_t *dict, uint16_t ack_id, callback_function_t callback, void *user_data, uint16_t timeout_ms)
{
    xSemaphoreTake(ack_lock, portMAX_DELAY);

    ack_data_t **aack_data = ack_dict_get(dict, ack_id);
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
    ack_dict_set_at(dict, ack_id, ack_data);

    xSemaphoreGive(ack_lock);
}

void call_ack(ack_dict_t *dict, uint16_t ack_id, void *data)
{
    xSemaphoreTake(ack_lock, portMAX_DELAY);

    ack_data_t **aack_data = ack_dict_get(dict, ack_id);
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
        ack_dict_erase(dict, ack_id);
    }

    xSemaphoreGive(ack_lock);
}

void del_ack(ack_dict_t *dict, uint16_t ack_id)
{
    xSemaphoreTake(ack_lock, portMAX_DELAY);

    ack_data_t **aack_data = ack_dict_get(dict, ack_id);
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
        ack_dict_erase(dict, ack_id);
    }

    xSemaphoreGive(ack_lock);
}

void check_ack_timeout(ack_dict_t *dict)
{
    if (!dict)
    {
        return;
    }

    if (ack_dict_size(dict) == 0)
    {
        return;
    }

    xSemaphoreTake(ack_lock, portMAX_DELAY);
    ack_dict_it_t it;
    ack_dict_it(it, dict);
    while (!ack_dict_end_p(it))
    {
        ack_dict_itref_t *item = ack_dict_ref(it);
        if (!item || !item->key || !item->value)
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
            ack_dict_erase(dict, item->key);
        }
        ack_dict_next(it);
    }
    xSemaphoreGive(ack_lock);
}

void init_ack_lock()
{
    ack_lock = xSemaphoreCreateMutex();
    if (!ack_lock)
    {
        LOGE("create ack lock failed");
    }
}

const power_protocol_app_t *power_protocol_registry[] = {
    &huawei_r48xx_info,
};

const uint32_t power_protocol_num = sizeof(power_protocol_registry) / sizeof(power_protocol_registry[0]);

power_protocol_app_t *get_current_power_protocol()
{
    return power_protocol_registry[0];
}