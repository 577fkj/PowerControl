#include "mui_event.h"

#include "freertos/FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "freertos/queue.h"

#include "log.h"

void mui_event_queue_init(mui_event_queue_t *p_queue)
{
    p_queue->event_queue = xQueueCreate(MAX_EVENT_MSG, sizeof(mui_event_t));
    xTaskCreate(mui_event_dispatch, "mui_tick", 1024 * 4, p_queue, 8, NULL);
}
void mui_event_set_callback(mui_event_queue_t *p_queue, mui_event_handler_t dispatcher,
                            void *context)
{
    p_queue->dispatch_context = context;
    p_queue->dispatcher = dispatcher;
}

void mui_event_post(mui_event_queue_t *p_queue, mui_event_t *p_event)
{
    // CRTIAL_ENTER
    if (xQueueSend(p_queue->event_queue, p_event, NULL) != pdPASS)
    {
        LOGI("event buffer is FULL!!");
    }
}

void mui_event_dispatch(mui_event_queue_t *p_queue)
{
    mui_event_t event;
    while (1)
    {
        if (xQueueReceive(p_queue->event_queue, &event, portMAX_DELAY) == pdPASS)
        {
            p_queue->dispatcher(p_queue->dispatch_context, &event);
        }
    }
}

void mui_event_dispatch_now(mui_event_queue_t *p_queue, mui_event_t *p_event)
{
    p_queue->dispatcher(p_queue->dispatch_context, p_event);
}