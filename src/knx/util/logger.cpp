#include "logger.h"

#include "../bits.h"

namespace Knx
{
    Map<loggername_t, Logger::LogType, 64> Logger::_loggers;
    Logger Logger::_logger;

    Logger& Logger::logger(loggername_t name)
    {
        _logger.name(name);
        return _logger;
    }

    void Logger::logLevel(loggername_t name, LogType level)
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
#ifdef __linux__
    void print(std::string msg)
    {
        print(msg.c_str());
    }
#endif

    bool Logger::log(LogType type)
    {
#ifndef KNX_NO_PRINT
        LogType* level = _loggers.get(_name);

        if (level == nullptr)
        {
            print("Logger ");
            print(_name);
            print(" is set to Warning. Use Logger::logLevel(\"");
            print(_name);
            println("\", Logger::Info) to show more logging.");
            _loggers.insertOrAssign(_name, Warning);
            level = _loggers.get(_name);
        }

        if (*level > type)
            return false;

        print(millis());
        print(" ");
        print(_name);
        print("\t");
        print(enum_name(type));
        print(" ");
        return true;
#else
        return false;
#endif
    }

    void Logger::log(LogType type, const char* format, va_list args)
    {
#ifndef KNX_NO_PRINT

        if (!log(type))
            return;

        while (*format)
        {
            if (*format == '%')
            {
                format++;

                if (*format == 'd')
                {
                    print(va_arg(args, int));
                }

                if (*format == 'x')
                {
                    print(va_arg(args, int), HEX);
                }
                else if (*format == 's')
                {
                    print(va_arg(args, char*));
                }
                else if (*format == 'f')
                {
                    print(va_arg(args, double));
                }
                else if (*format == 'B')
                {
                    uint8_t* data = va_arg(args, uint8_t*);
                    size_t length = va_arg(args, int);
                    printHex("", data, length, false);
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
}