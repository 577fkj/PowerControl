#include "bsp_btn.h"

#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "freertos/queue.h"

#include "esp_err.h"
#include "driver/gpio.h"

#include "driver/gptimer.h"
#include "esp_timer.h"

#include "main.h"

#include "mui_input.h"

#define BSP_BUTTON_LONG_PUSH_TIME_MS 1000
#define BSP_BUTTON_REPEAT_PUSH_TIMEOUT_MS 200

typedef enum
{
    BTN_STATE_IDLE,
    BTN_STATE_PRESSED,
    BTN_STATE_LONG_PRESSED,
    BTN_STATE_REPEAT
} btn_state_t;

typedef enum
{
    APP_BUTTON_PUSH,
    APP_BUTTON_RELEASE,
    BSP_BUTTON_ACTION_LONG_PUSH,
    BSP_BUTTON_ACTION_REPEAT_PUSH
} btn_action_t;

typedef struct
{
    btn_state_t state;
    esp_timer_handle_t m_bsp_button_long_push_tmr;
    esp_timer_handle_t m_bsp_button_repeat_push_tmr;
    bool press_key;
} bsp_btn_t;

xQueueHandle gpio_evt_queue = NULL; // 新建一个队列句柄
bsp_btn_event_cb_t m_bsp_btn_event_cb = NULL;

static bsp_btn_t m_bsp_btns[3] = {0};

static void bsp_button_event_handler(uint8_t pin_no, btn_action_t button_action);

static void button_long_push_timer_handler(uint16_t p_context)
{
    bsp_button_event_handler(p_context, BSP_BUTTON_ACTION_LONG_PUSH);
}

static void button_repeat_push_timer_handler(uint16_t p_context)
{
    bsp_button_event_handler(p_context, BSP_BUTTON_ACTION_REPEAT_PUSH);
}

static uint8_t bsp_button_pin_to_index(uint8_t pin_no)
{
    switch (pin_no)
    {
    case UP_KEY:
        if (m_bsp_btns[0].m_bsp_button_long_push_tmr == NULL)
        {
            esp_timer_create_args_t m_bsp_button_long_push_tmr = {
                .callback = &button_long_push_timer_handler, // 定时器回调函数
                .arg = UP_KEY,                               // 传递给回调函数的参数
                .name = "up_bsp_button_long_push_tmr",       // 定时器名称
            };
            esp_timer_create(&m_bsp_button_long_push_tmr, &m_bsp_btns[0].m_bsp_button_long_push_tmr);
        }
        if (m_bsp_btns[0].m_bsp_button_repeat_push_tmr == NULL)
        {
            esp_timer_create_args_t m_bsp_button_repeat_push_tmr = {
                .callback = &button_repeat_push_timer_handler, // 定时器回调函数
                .arg = UP_KEY,                                 // 传递给回调函数的参数
                .name = "up_bsp_button_repeat_push_tmr",       // 定时器名称
            };
            esp_timer_create(&m_bsp_button_repeat_push_tmr, &m_bsp_btns[0].m_bsp_button_repeat_push_tmr);
        }
        return 0;
    case CENTER_KEY:
        if (m_bsp_btns[1].m_bsp_button_long_push_tmr == NULL)
        {
            esp_timer_create_args_t m_bsp_button_long_push_tmr = {
                .callback = &button_long_push_timer_handler, // 定时器回调函数
                .arg = CENTER_KEY,                           // 传递给回调函数的参数
                .name = "center_bsp_button_long_push_tmr",   // 定时器名称
            };
            esp_timer_create(&m_bsp_button_long_push_tmr, &m_bsp_btns[1].m_bsp_button_long_push_tmr);
        }
        if (m_bsp_btns[1].m_bsp_button_repeat_push_tmr == NULL)
        {
            esp_timer_create_args_t m_bsp_button_repeat_push_tmr = {
                .callback = &button_repeat_push_timer_handler, // 定时器回调函数
                .arg = CENTER_KEY,                             // 传递给回调函数的参数
                .name = "center_bsp_button_repeat_push_tmr",   // 定时器名称
            };
            esp_timer_create(&m_bsp_button_repeat_push_tmr, &m_bsp_btns[1].m_bsp_button_repeat_push_tmr);
        }
        return 1;
    case DOWN_KEY:
        if (m_bsp_btns[2].m_bsp_button_long_push_tmr == NULL)
        {
            esp_timer_create_args_t m_bsp_button_long_push_tmr = {
                .callback = &button_long_push_timer_handler, // 定时器回调函数
                .arg = DOWN_KEY,                             // 传递给回调函数的参数
                .name = "down_bsp_button_long_push_tmr",     // 定时器名称
            };
            esp_timer_create(&m_bsp_button_long_push_tmr, &m_bsp_btns[2].m_bsp_button_long_push_tmr);
        }
        if (m_bsp_btns[2].m_bsp_button_repeat_push_tmr == NULL)
        {
            esp_timer_create_args_t m_bsp_button_repeat_push_tmr = {
                .callback = &button_repeat_push_timer_handler, // 定时器回调函数
                .arg = DOWN_KEY,                               // 传递给回调函数的参数
                .name = "down_bsp_button_repeat_push_tmr",     // 定时器名称
            };
            esp_timer_create(&m_bsp_button_repeat_push_tmr, &m_bsp_btns[2].m_bsp_button_repeat_push_tmr);
        }
        return 2;
    default:
        return 0xFF;
    }
}

static void bsp_button_callback(uint8_t idx, bsp_btn_event_t event)
{
    if (m_bsp_btn_event_cb)
    {
        m_bsp_btn_event_cb(idx, event);
    }
}

// IO 口中断回调函数
void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    // 插入一个中断到队列中
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void bsp_button_event_handler(uint8_t pin_no, btn_action_t button_action)
{
    esp_err_t err_code = ESP_OK;
    uint8_t idx = bsp_button_pin_to_index(pin_no);
    switch (button_action)
    {
    case APP_BUTTON_PUSH:
    {
        switch (m_bsp_btns[idx].state)
        {
        case BTN_STATE_IDLE:
            err_code = esp_timer_start_once(m_bsp_btns[idx].m_bsp_button_long_push_tmr, BSP_BUTTON_LONG_PUSH_TIME_MS * 1000);
            if (err_code == ESP_OK)
            {
                m_bsp_btns[idx].state = BTN_STATE_PRESSED;
            }
            break;

        case BTN_STATE_LONG_PRESSED:
        case BTN_STATE_PRESSED:
        case BTN_STATE_REPEAT:
            // TODO
            break;
        }
    }
    break;

    case APP_BUTTON_RELEASE:
    {
        switch (m_bsp_btns[idx].state)
        {
        case BTN_STATE_IDLE:
            break;

        case BTN_STATE_PRESSED:
            esp_timer_stop(m_bsp_btns[idx].m_bsp_button_long_push_tmr);
            bsp_button_callback(idx, BSP_BTN_EVENT_SHORT);
            break;

        case BTN_STATE_LONG_PRESSED:
        case BTN_STATE_REPEAT:
            esp_timer_stop(m_bsp_btns[idx].m_bsp_button_repeat_push_tmr);
            break;
        }
        m_bsp_btns[idx].state = BTN_STATE_IDLE;
    }
    break;

    case BSP_BUTTON_ACTION_LONG_PUSH:
    {
        bsp_button_callback(idx, BSP_BTN_EVENT_LONG);
        err_code = esp_timer_start_periodic(m_bsp_btns[idx].m_bsp_button_repeat_push_tmr, BSP_BUTTON_REPEAT_PUSH_TIMEOUT_MS * 1000);
        if (err_code == ESP_OK)
        {
            m_bsp_btns[idx].state = BTN_STATE_LONG_PRESSED;
        }
    }
    break;

    case BSP_BUTTON_ACTION_REPEAT_PUSH:
    {
        m_bsp_btns[idx].state = BTN_STATE_REPEAT;
        bsp_button_callback(idx, BSP_BTN_EVENT_REPEAT);
    }
    break;

    default:
        break;
    }
}

// 返回长按键和短按键
esp_err_t key_scan()
{
    uint32_t io_num;
    BaseType_t press_key = pdFALSE;
    BaseType_t lift_key = pdFALSE;
    int backup_time = 0;

    while (1)
    {
        // 接收从消息队列发来的消息
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY) == pdPASS)
        {
            uint8_t idx = bsp_button_pin_to_index(io_num);
            if (gpio_get_level(io_num) == 0)
            {
                m_bsp_btns[idx].press_key = true;
                bsp_button_event_handler(io_num, APP_BUTTON_PUSH);
            }
            else if (m_bsp_btns[idx].press_key)
            {
                m_bsp_btns[idx].press_key = false;
                bsp_button_event_handler(io_num, APP_BUTTON_RELEASE);
            }
        }
    }
}

void init_key()
{
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    /* 定义一个gpio配置结构体 */
    gpio_config_t gpio_config_structure;
    /* 初始化全部内容为0 */
    memset(&gpio_config_structure, 0, sizeof(gpio_config_structure));
    /* 初始化gpio配置结构体*/
    gpio_config_structure.pin_bit_mask |= (1ULL << UP_KEY);
    gpio_config_structure.pin_bit_mask |= (1ULL << CENTER_KEY);
    gpio_config_structure.pin_bit_mask |= (1ULL << DOWN_KEY);
    gpio_config_structure.mode = GPIO_MODE_INPUT;        /* 输入模式 */
    gpio_config_structure.pull_up_en = 1;                /* 上拉 */
    gpio_config_structure.pull_down_en = 0;              /* 不下拉 */
    gpio_config_structure.intr_type = GPIO_INTR_ANYEDGE; /* ！<GPIO中断类型：上升沿 & 下降沿*/

    /* 根据设定参数初始化并使能 */
    gpio_config(&gpio_config_structure);

    gpio_install_isr_service(1);                                            // 安装驱动程序的GPIO ISR处理程序服务
    gpio_isr_handler_add(UP_KEY, gpio_isr_handler, (void *)UP_KEY);         // 为相应的GPIO引脚添加ISR处理程序
    gpio_isr_handler_add(CENTER_KEY, gpio_isr_handler, (void *)CENTER_KEY); // 为相应的GPIO引脚添加ISR处理程序
    gpio_isr_handler_add(DOWN_KEY, gpio_isr_handler, (void *)DOWN_KEY);     // 为相应的GPIO引脚添加ISR处理程序

    xTaskCreate(key_scan, "key_trigger", 1024 * 2, NULL, 10, NULL);
}

void bsp_btn_init(bsp_btn_event_cb_t p_event_cb)
{
    m_bsp_btn_event_cb = p_event_cb;
}