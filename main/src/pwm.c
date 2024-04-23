#include "driver/ledc.h"
#include "main.h"
#include "log.h"

volatile uint8_t fan_speed = 0;

void init_pwm(void)
{
    ledc_timer_config_t timer_initer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // 设置分辨率,最大为2^13-1
        .freq_hz = 8000,                      // PWM信号频率
        .speed_mode = LEDC_LOW_SPEED_MODE,    // 定时器模式（“高速”或“低速”）
        .timer_num = LEDC_TIMER_1,            // 设置定时器源（0-3）
        .clk_cfg = LEDC_AUTO_CLK,             // 配置LEDC时钟源（这里是自动选择）
    };
    ledc_timer_config(&timer_initer);

    gpio_reset_pin(FAN_CTRL);

    ledc_channel_config_t channel_initer = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = FAN_CTRL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1,
    };
    ledc_channel_config(&channel_initer);

    gpio_reset_pin(LED_GPIO_PIN);

    ledc_channel_config_t led_channel_initer = {
        .channel = LEDC_CHANNEL_1,
        .duty = 0,
        .gpio_num = LED_GPIO_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1,
    };
    ledc_channel_config(&led_channel_initer);
}

void led_set_level(uint8_t level)
{
    if (level > 100)
        level = 100;

    int duty = (int)(level * 81.91f);

    LOGI("led level: %d, duty: %d\n", level, duty);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    // ledc_timer_resume(LEDC_LOW_SPEED_MODE, LEDC_TIMER_1);
}

void fan_set_speed(uint8_t duty)
{
    if (duty > 100)
        duty = 100;

    fan_speed = duty;

    int fan_duty = (int)(duty * 81.91f);

    LOGI("fan speed: %d, duty: %d\n", duty, fan_duty);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, fan_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    // ledc_timer_resume(LEDC_LOW_SPEED_MODE, LEDC_TIMER_1);
}

uint16_t fan_get_speed()
{
    return (uint16_t)fan_speed;
}