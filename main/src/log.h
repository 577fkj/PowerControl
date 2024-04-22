#ifndef __LOG_H__
#define __LOG_H__

#include "esp_log.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LOG(LEVEL, ...) printf(__FILE__ ":" TOSTRING(__LINE__) " " LEVEL ":" __VA_ARGS__)
#define LOGI(...) LOG("I", __VA_ARGS__)
#define LOGW(...) LOG("W", __VA_ARGS__)
#define LOGE(...) LOG("E", __VA_ARGS__)

#endif // __LOG_H__