#include <stdarg.h>
#include "simple_map.h"
class Logger
{
    public:
        enum LogType { Info, Warning, Error, Critical, Exception, Disabled};
        static Logger& logger(const char* name);
        static void logLevel(const char* name, LogType level);
        void info(const char* message, ...);
        void warning(const char* message, ...);
        void error(const char* message, ...);
        void critical(const char* message, ...);
        void exception(const char* message, ...);
    protected:
        Logger() {}
        void log(LogType type, const char* format, va_list args);
        void name(const char* value) { _name = value; }
    private:
        const char* enum_name(LogType type);
        const char* _name = "";
        static Map<const char*, LogType, 64> _loggers;
        static Logger _logger;
};