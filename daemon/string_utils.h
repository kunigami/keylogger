#pragma once

#include <iostream>
#include <sstream>
#include <vector>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// format string using printf() syntax
std::string format(const char *format, ...);

std::vector<std::string> split(std::string s, std::string delimiter);

// do not use directly. Prefer format
std::string _format_list(const char *format, va_list argp);
