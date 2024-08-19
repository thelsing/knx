#include "logger.h"

#include "../bits.h"

Map<const char*, Logger::LogType, 64> Logger::_loggers;
Logger Logger::_logger;

Logger& Logger::logger(const char* name)
{
    _logger.name(name);
    return _logger;
}

void Logger::logLevel(const char* name, LogType level)
{
    _loggers.insertOrAssign(name, level);
}

void Logger::info(const char* message, ...)
{
#ifndef KNX_NO_PRINT
    va_list objects;
    va_start( objects, message);
    log(LogType::Info, message, objects);
    va_end(objects);
#endif
}

void Logger::warning(const char* message, ...)
{
#ifndef KNX_NO_PRINT
    va_list objects;
    va_start( objects, message);
    log(LogType::Warning, message, objects);
    va_end(objects);
#endif
}

void Logger::error(const char* message, ...)
{
#ifndef KNX_NO_PRINT
    va_list objects;
    va_start( objects, message);
    log(LogType::Error, message, objects);
    va_end(objects);
#endif
}

void Logger::critical(const char* message, ...)
{
#ifndef KNX_NO_PRINT
    va_list objects;
    va_start( objects, message);
    log(LogType::Critical, message, objects);
    va_end(objects);
#endif
}

void Logger::exception(const char* message, ...)
{
#ifndef KNX_NO_PRINT
    va_list objects;
    va_start( objects, message);
    log(LogType::Exception, message, objects);
    va_end(objects);
#endif
}

void Logger::log(LogType type, const char* format, va_list args)
{
#ifndef KNX_NO_PRINT
    LogType* level = _loggers.get(_name);
    if(level == nullptr) {
        print("Logger ");
        print(_name);
        print(" is disabled. Use Logger::logLevel(\"");
        print(_name);
        println("\", Logger::Info) to enable.");
        _loggers.insertOrAssign(_name, Disabled);
        return;
    }

    if(*level > type)
        return;

    print(millis());
    print(" ");
    print(_name);
    print("\t");
    print(enum_name(type));
    print(" ");

    while (*format)
    {
        if (*format == '%')
        {
            format++;

            if (*format == 'd')
            {
                print(va_arg(args, int));
            }
            else if (*format == 's')
            {
                print(va_arg(args, char*));
            }
            else if (*format == 'f')
            {
                print(va_arg(args, double));
            }
        }
        else
        {
            print(*format);
        }

        format++;
    }

    va_end(args);
    println();
#endif
}
#ifndef KNX_NO_PRINT
const char* Logger::enum_name(LogType type)
{
    switch (type)
    {
        case LogType::Info:
            return "INFO";

        case LogType::Warning:
            return "WARN";

        case LogType::Error:
            return "ERR ";

        case LogType::Critical:
            return "CRIT";

        case LogType::Exception:
            return "EXCE";

        case LogType::Disabled:
            return "DISA";
    }

    return "";
}
#endif
