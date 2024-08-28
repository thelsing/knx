#pragma once

#include <stdarg.h>
#include "simple_map.h"

namespace Knx
{
    class IPrintable
    {
        public:
            virtual void printIt() const = 0;
            virtual ~IPrintable() = default;
    };

#ifndef KNX_NO_PRINT
    void println();
    void print(const char*);
#else
    #define print(...)      do {} while(0)
    #define println(...)    do {} while(0)
#endif
    class Logger
    {
        public:
            enum LogType { Info, Warning, Error, Critical, Exception, Disabled};
            static Logger& logger(const char* name);
            static void logLevel(const char* name, LogType level);
            void info(const char* message, IPrintable& object)
            {
                if (!log(LogType::Info))
                    return;

                print(message);
                object.printIt();
                println();
            }
            void info(const char* message, ...);
            void warning(const char* message, IPrintable& object)
            {
                if (!log(LogType::Warning))
                    return;

                print(message);
                object.printIt();
                println();
            }
            void warning(const char* message, ...);
            void error(const char* message, IPrintable& object)
            {
                if (!log(LogType::Error))
                    return;

                print(message);
                object.printIt();
                println();
            }
            void error(const char* message, ...);
            void critical(const char* message, IPrintable& object)
            {
                if (!log(LogType::Critical))
                    return;

                print(message);
                object.printIt();
                println();
            }
            void critical(const char* message, ...);
            void exception(const char* message, IPrintable& object)
            {
                if (!log(LogType::Exception))
                    return;

                print(message);
                object.printIt();
                println();
            }
            void exception(const char* message, ...);
        protected:
            Logger() {}
            bool log(LogType type);
            void log(LogType type, const char* format, va_list args);
            void name(const char* value)
            {
                _name = value;
            }
        private:
            const char* enum_name(LogType type);
            const char* _name = "";
            static Map<const char*, LogType, 64> _loggers;
            static Logger _logger;
    };
}