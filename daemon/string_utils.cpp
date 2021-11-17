#include "string_utils.h"

std::string _format_list(const char *format, va_list argp) {
  char *buffer;
  vasprintf(&buffer, format, argp);
  std::string formatted(buffer);
  return formatted;
}

std::string format(const char *format, ...) {
	va_list argp;
	va_start(argp, format);
	std::string ret = _format_list(format, argp);
	va_end(argp);
  return ret;
}

std::vector<std::string> split (std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}
