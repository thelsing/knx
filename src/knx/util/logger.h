#include <stdarg.h>
#include <string>
#include "simple_map.h"

class NoOpLogger;
class Logger
{
    public:
        enum LogType { Info, Warning, Error, Critical, Exception, Disabled};
        static Logger& logger(const std::string name);
        static void logLevel(const std::string name, LogType level);
        void info(const std::string message, ...);
        void warning(const std::string message, ...);
        void error(const std::string message, ...);
        void critical(const std::string message, ...);
        void exception(const std::string message, ...);
    protected:
        Logger() {}
        void log(LogType type, const char* format, va_list args);
        void name(std::string value) { _name = value; }
    private:
        const std::string enum_name(LogType type);
        std::string _name = "";
        static Map<std::string, LogType, 64> _loggers;
        static Logger _logger;
};