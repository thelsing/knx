#include "logger.h"

Logger Logger::logger(const std::string name)
{
    return Logger(name);
}

void Logger::info(const std::string message, ...)
{
#ifndef KNX_NO_PRINT
    va_list objects;
    va_start( objects, message);
    log(LogType::Info, message.c_str(), objects);
    va_end(objects);
#endif
}

void Logger::warning(const std::string message, ...)
{
#ifndef KNX_NO_PRINT
    va_list objects;
    va_start( objects, message);
    log(LogType::Warning, message.c_str(), objects);
    va_end(objects);
#endif
}

void Logger::error(const std::string message, ...)
{
#ifndef KNX_NO_PRINT
    va_list objects;
    va_start( objects, message);
    log(LogType::Error, message.c_str(), objects);
    va_end(objects);
#endif
}

void Logger::critical(const std::string message, ...)
{
#ifndef KNX_NO_PRINT
    va_list objects;
    va_start( objects, message);
    log(LogType::Critical, message.c_str(), objects);
    va_end(objects);
#endif
}

void Logger::exception(const std::string message, ...)
{
#ifndef KNX_NO_PRINT
    va_list objects;
    va_start( objects, message);
    log(LogType::Exception, message.c_str(), objects);
    va_end(objects);
#endif
}

void Logger::log(LogType type, const char* format, va_list args)
{
#ifndef KNX_NO_PRINT
    print(millis());
    print(" ");
    print(_name.c_str());
    print("\t");
    print(enum_name(type).c_str());
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
            else if (*format == 'S')
            {
                print(va_arg(args, std::string).c_str());
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

const std::string Logger::enum_name(LogType type)
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
    }

    return std::to_string(type);
}
