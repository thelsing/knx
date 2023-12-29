
#include "logger.h"

void KnxLogger::log(const char* message, va_list& values)
{
    printf(message, values);
}

KnxLogger knxLogger;