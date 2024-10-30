#ifndef __KNX_LOG_H__
#define __KNX_LOG_H__

#pragma once

// #ifdef ESP32

#include "esp_log.h"

// Uncomment enable KNX SDK LOGS
#define KNX_SDK_ENABLE_LOGS

static const char* KTAG = "KNX_LIB";

#ifdef KNX_SDK_ENABLE_LOGS

#define KNX_LOGE ESP_LOGE
#define KNX_LOGW ESP_LOGW
#define KNX_LOGI ESP_LOGI
#define KNX_LOGD ESP_LOGD
#define KNX_LOGV ESP_LOGV
#else

#define KNX_LOGE(...) ((void)0)
#define KNX_LOGW(...) ((void)0)
#define KNX_LOGI(...) ((void)0)
#define KNX_LOGD(...) ((void)0)
#define KNX_LOGV(...) ((void)0)

#endif // KNX_SDK_ENABLE_LOGS

// #endif // ESP32

#endif // __KNX_LOG_H__
