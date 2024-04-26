#ifndef __ADC_H__
#define __ADC_H__

#include "driver/adc.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

void adc_init();

float get_temp();

#endif // __ADC_H__