#pragma once

#include "simple_map.h"
#include <stdarg.h>

#ifdef __linux
#include <string>
#define loggername_t std::string
#else
#define loggername_t const char*
#endif

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
#define print(...) \
    do             \
    {              \
    } while (0)
#define println(...) \
    do               \
    {                \
    } while (0)
#endif
    class Logger
    {
        public:
            enum LogType
            {
                Info,
                Warning,
                Error,
                Critical,
                Exception,
                Disabled
            };
            static Logger& logger(loggername_t name);
            static void logLevel(loggername_t name, LogType level);
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
            void name(loggername_t value)
            {
                _name = value;
            }

        private:
            const char* enum_name(LogType type);
            loggername_t _name = "";
            static Map<loggername_t, LogType, 64> _loggers;
            static Logger _logger;
    };
} // namespace Knx