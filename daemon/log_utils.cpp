#include "log_utils.h"

#include <iostream>

#include "string_utils.h"
#include "time_utils.h"

void log(const char *format, ...) {
	va_list argp;
	va_start(argp, format);
	std::string ret = _format_list(format, argp);
	va_end(argp);

    std::cout << "[" << get_current_time() << "] " << ret << std::endl;
}

void log_error(const char *format, ...) {
	va_list argp;
	va_start(argp, format);
	std::string ret = _format_list(format, argp);
	va_end(argp);

    std::cerr << "[" << get_current_time() << "] " << ret << std::endl;
	exit(EXIT_FAILURE);
}
