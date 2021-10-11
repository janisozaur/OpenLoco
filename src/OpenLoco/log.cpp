#include "log.hpp"
#include "Platform/Platform.h"
#include <cstdarg>
#include <fstream>
static std::ofstream _log_stream;

void consoleinit()
{
    if (!_log_stream)
    {
        auto env = OpenLoco::Platform::getCurrentExecutablePath().parent_path() / "OpenLoco.log";
        std::string path = env.string();
        printf("Logging to %s\n", path.c_str());
        _log_stream.open(path, std::ios::out);
    }
}
void WriteLine(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteLine_VA(format, args);
    va_end(args);
}

void WriteLine_VA(const char* format, va_list args)
{
    consoleinit();
    char buffer[4096];
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    //std::printf("%s\n", buffer);
    _log_stream << buffer << std::endl;
}
