
#include "logger.h"

void KnxLogger::log(const char* message, va_list& values)
{
    if(_callback)
    {
        _callback(message, values);
        return;
    }
    printf(message, values);
}

void KnxLogger::setCallback(KnxLoggerCallback callback)
{
    _callback = callback;
}

KnxLogger knxLogger;