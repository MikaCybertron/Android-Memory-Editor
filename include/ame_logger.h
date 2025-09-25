/*
 * Copyright (C) 2024, 2025  Dicot0721
 *
 * This file is part of Android-Memory-Editor.
 *
 * Android-Memory-Editor is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Android-Memory-Editor is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Android-Memory-Editor.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef AME_LOGGER_H
#define AME_LOGGER_H

#include <array>
#include <chrono>
#include <format>
#include <iostream>
#include <source_location>

namespace ame {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    OFF,
};

class Logger {
public:
    [[nodiscard, gnu::visibility("default")]] static Logger &Instance() {
        static Logger logger;
        return logger;
    }

    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger &operator=(Logger &&) = delete;

    [[nodiscard]] LogLevel GetLevel() const noexcept {
        return _level;
    }

    void SetLevel(LogLevel level) noexcept {
        _level = level;
    }

    template <typename... Args>
    void Debug(std::source_location location, std::format_string<Args...> format, Args &&...args) {
        Output(location, LogLevel::DEBUG, format.get(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Info(std::source_location location, std::format_string<Args...> format, Args &&...args) {
        Output(location, LogLevel::INFO, format.get(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Warn(std::source_location location, std::format_string<Args...> format, Args &&...args) {
        Output(location, LogLevel::WARN, format.get(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Error(std::source_location location, std::format_string<Args...> format, Args &&...args) {
        Output(location, LogLevel::ERROR, format.get(), std::forward<Args>(args)...);
    }

protected:
    Logger() = default;
    ~Logger() = default;

    void Output(std::source_location location, LogLevel level, std::string_view format, auto &&...args) {
        if (level < _level) {
            return;
        }

        std::ostream &stream = (level < LogLevel::ERROR) ? std::clog : std::cerr;
        const auto now = std::chrono::system_clock::now();
        const std::string message = std::vformat(format, std::make_format_args(args...));

        // "\033[39m" -> default color
        std::println(stream, "{}[{:%T}][{}] [{}] {}\033[39m", _colorStrs[int(level)], now, _levelChars[int(level)], location.function_name(), message);
    }

    static constexpr std::array<std::string_view, 4> _colorStrs = {
        "\033[32m", // green
        "",
        "\033[33m", // yellow
        "\033[31m", // red
    };

    static constexpr std::array<char, 4> _levelChars = {'D', 'I', 'W', 'E'};

    LogLevel _level = LogLevel::DEBUG;
};

} // namespace ame

#define LOG_DEBUG(...) ame::Logger::Instance().Debug(std::source_location::current(), __VA_ARGS__)
#define LOG_INFO(...) ame::Logger::Instance().Info(std::source_location::current(), __VA_ARGS__)
#define LOG_WARN(...) ame::Logger::Instance().Warn(std::source_location::current(), __VA_ARGS__)
#define LOG_ERROR(...) ame::Logger::Instance().Error(std::source_location::current(), __VA_ARGS__)

#endif // AME_LOGGER_H
