/* this is just a draft for the logging API, it is not complete and mssing all the backend

ToDo:
- define all areas
- complete the logging functions for all levels
- define the backend logging class that can be inherited and overriden
- define an interface to redirect the logging stream

Usage:
- define KNX_LOG_LVL and KNX_LOG_AREAS globally to your desired value
- KNX_LOG_TRACE<KNX_LOG_LL | KNX_LOG_IP>("Unhandled service identifier: %02x", code);


*/
#pragma once

#include <utility>
#include <string>
#include <cstdarg>

#include "platform.h"

typedef void (*KnxLoggerCallback)(const char* message, va_list& values);

class KnxLogger
{
    protected:
        KnxLoggerCallback _callback = nullptr;
    public:
        void log(const char* message, va_list& values);
        void setCallback(KnxLoggerCallback callback);
};

extern KnxLogger knxLogger;


constexpr uint64_t KNX_LOG_LVL_ERROR = 1;
constexpr uint64_t KNX_LOG_LVL_INFO  = 2;
constexpr uint64_t KNX_LOG_LVL_DEBUG = 3;
constexpr uint64_t KNX_LOG_LVL_TRACE = 4;


constexpr uint64_t KNX_LOG_LL       = 0x0001;
constexpr uint64_t KNX_LOG_NL       = 0x0002;
constexpr uint64_t KNX_LOG_TL       = 0x0004;
constexpr uint64_t KNX_LOG_AL       = 0x0008;
constexpr uint64_t KNX_LOG_TPUART   = 0x0010;
constexpr uint64_t KNX_LOG_IP       = 0x0011;
constexpr uint64_t KNX_LOG_MEM      = 0x0012;


#ifndef KNX_LOG_AREAS
 #define KNX_LOG_AREAS 0
#endif

#ifndef KNX_LOG_LVL
 #define KNX_LOG_LVL 0
#endif

constexpr uint64_t LOGLEVEL = KNX_LOG_LVL;
constexpr uint64_t LOGAREAS = KNX_LOG_AREAS;

template<uint64_t x, typename... Args>
 __attribute__((always_inline)) constexpr void KNX_LOG_TRACE(Args&&... args)
{
    if constexpr((LOGLEVEL >= KNX_LOG_LVL_TRACE) && (x & LOGAREAS))
        knxLogger.log(std::forward<Args>(args)...);
}

template<uint64_t x, typename... Args>
 __attribute__((always_inline)) constexpr void KNX_LOG_DEBUG(Args&&... args)
{
    if constexpr((LOGLEVEL >= KNX_LOG_LVL_DEBUG) && (x & LOGAREAS))
        knxLogger.log(std::forward<Args>(args)...);
}

template<uint64_t x, typename... Args>
 __attribute__((always_inline)) constexpr void KNX_LOG_INFO(Args&&... args)
{
    if constexpr((LOGLEVEL >= KNX_LOG_LVL_INFO) && (x & LOGAREAS))
        knxLogger.log(std::forward<Args>(args)...);
}

template<uint64_t x, typename... Args>
 __attribute__((always_inline)) constexpr void KNX_LOG_ERROR(Args&&... args)
{
    if constexpr((LOGLEVEL >= KNX_LOG_LVL_ERROR) && (x & LOGAREAS))
        knxLogger.log(std::forward<Args>(args)...);
}