#pragma once
#include <cstdarg>
void consoleinit();
void WriteLine(const char* format, ...);
void WriteLine_VA(const char* format, va_list args);