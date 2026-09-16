#pragma once
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

namespace SafeTime {

inline std::string formatLocalTime(std::time_t time, const char* format = "%Y-%m-%d %H:%M:%S") {
    std::tm tm_buf{};
#ifdef _WIN32
    if (localtime_s(&tm_buf, &time) != 0) {
        return "Unknown Time";
    }
#else
    if (localtime_r(&time, &tm_buf) == nullptr) {
        return "Unknown Time";
    }
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm_buf, format);
    return ss.str();
}

inline std::string now() {
    return formatLocalTime(std::time(nullptr));
}

} // namespace SafeTime