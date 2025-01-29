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

#ifndef __AME_LOGGER_H__
#define __AME_LOGGER_H__

#include <cassert>

#include <array>
#include <chrono>
#include <format>
#include <iostream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    OFF,
};

constexpr int VALID_LOG_LEVEL_COUNT = int(LogLevel::OFF);


class Logger {
public:
    [[nodiscard]] LogLevel GetLevel() const noexcept {
        return _level;
    }

    void SetLevel(LogLevel level) noexcept {
        _level = level;
    }

    template <typename... Args>
    void Debug(std::format_string<Args...> format, Args &&...args) {
        _Output(LogLevel::DEBUG, format.get(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Info(std::format_string<Args...> format, Args &&...args) {
        _Output(LogLevel::INFO, format.get(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Warning(std::format_string<Args...> format, Args &&...args) {
        _Output(LogLevel::WARNING, format.get(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Error(std::format_string<Args...> format, Args &&...args) {
        _Output(LogLevel::ERROR, format.get(), std::forward<Args>(args)...);
    }

protected:
    void _Output(LogLevel level, std::string_view format, auto &&...args) {
        if (level < _level) {
            return;
        }
        assert(int(level) < VALID_LOG_LEVEL_COUNT);

        std::ostream &stream = (level < LogLevel::ERROR) ? std::clog : std::cerr;
        auto now = std::chrono::system_clock::now();
        std::string message = std::vformat(format, std::make_format_args(args...));

        // "\033[39m" -> default color
        stream << std::format("{}[{:%T}][{}] {}\033[39m\n", _colorStr[int(level)], now, _levelStr[int(level)], message);
    }

    static constexpr std::array<std::string, VALID_LOG_LEVEL_COUNT> _levelStr = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
    };

    static constexpr std::array<std::string, VALID_LOG_LEVEL_COUNT> _colorStr = {
        "\033[32m", // green
        "",
        "\033[33m", // yellow
        "\033[31m", // red
    };

    LogLevel _level = LogLevel::DEBUG;

} inline logger;

#endif // __AME_LOGGER_H__
