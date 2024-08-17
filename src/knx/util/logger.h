#include "../bits.h"
#include <stdarg.h>
#include <string>
using namespace std;

class Logger
{
    public:
        static Logger logger(const string name)
        {
            return Logger(name);
        }
        inline void info(const string message, ...)
        {
#ifndef KNX_NO_PRINT
            va_list objects;
            va_start( objects, message);
            log(LogType::Info, message.c_str(), objects);
            va_end(objects);
#endif
        }
        inline void warning(const string message, ...)
        {
#ifndef KNX_NO_PRINT
            va_list objects;
            va_start( objects, message);
            log(LogType::Warning, message.c_str(), objects);
            va_end(objects);
#endif
        }
        inline void error(const string message, ...)
        {
#ifndef KNX_NO_PRINT
            va_list objects;
            va_start( objects, message);
            log(LogType::Error, message.c_str(), objects);
            va_end(objects);
#endif
        }
        inline void critical(const string message, ...)
        {
#ifndef KNX_NO_PRINT
            va_list objects;
            va_start( objects, message);
            log(LogType::Critical, message.c_str(), objects);
            va_end(objects);
#endif
        }
        inline void exception(const string message, ...)
        {
#ifndef KNX_NO_PRINT
            va_list objects;
            va_start( objects, message);
            log(LogType::Exception, message.c_str(), objects);
            va_end(objects);
#endif
        }
    private:
        enum LogType { Info, Warning, Error, Critical, Exception};
        const string enum_name(LogType type)
        {
            switch (type)
            {
                case LogType::Info:
                    return "INFO";

                case LogType::Warning:
                    return "WARNING";

                case LogType::Error:
                    return "ERROR";

                case LogType::Critical:
                    return "CRITICAL";

                case LogType::Exception:
                    return "EXCEPTION";
            }
            return to_string(type);
        }
        const string _name;
        Logger(const string name) : _name(name) {}
        inline void log(LogType type, const char* format, va_list args)
        {
#ifndef KNX_NO_PRINT
            print(millis());
            print("\t");
            print(_name.c_str());
            print("\t");
            print(enum_name(type).c_str());
            print("\t");

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
};