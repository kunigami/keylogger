#include "time_utils.h"

std::string get_today_date() {
    auto time = std::time(0);
    std::ostringstream stream;
    stream << std::put_time(std::localtime(&time), "%F");
    return stream.str();
}

std::string get_current_time() {
    auto time = std::time(0);
    std::ostringstream stream;
    stream << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return stream.str();
}
