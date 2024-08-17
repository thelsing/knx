#include "../bits.h"
#include <stdarg.h>
#include <string>

class Logger
{
    public:
        static Logger logger(const std::string name);
        void info(const std::string message, ...);
        void warning(const std::string message, ...);
        void error(const std::string message, ...);
        void critical(const std::string message, ...);
        void exception(const std::string message, ...);
    private:
        enum LogType { Info, Warning, Error, Critical, Exception};
        const std::string enum_name(LogType type);
        const std::string _name;
        Logger(const std::string name) : _name(name) {}
        inline void log(LogType type, const char* format, va_list args);
};