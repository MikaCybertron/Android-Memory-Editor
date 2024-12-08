/*
 * Copyright (C) 2024  Dicot0721
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

#ifndef __INC_AME_LOGGER_H__
#define __INC_AME_LOGGER_H__

#include <format>
#include <iostream>
#include <string_view>

enum class LogLevel {
    INFO,
    DEBUG,
    WARNING,
    ERROR,
};

class Logger {
public:
    void Debug(std::string_view message) noexcept {
        std::clog << "\033[32m"            // green
                  << "[DEBUG] " << message //
                  << "\033[39m"            // default color
                  << "\n";
    }

    void Info(std::string_view message) noexcept {
        std::cout << "[INFO] " << message << std::endl;
    }

    void Warning(std::string_view message) noexcept {
        std::cerr << "\033[33m"              // yellow
                  << "[WARNING] " << message //
                  << "\033[39m"              // default color
                  << "\n";
    }

    void Error(std::string_view message) noexcept {
        std::cerr << "\033[31m"            // red
                  << "[ERROR] " << message //
                  << "\033[39m"            // default color
                  << "\n";
    }

    template <class... Types>
    void Debug(const std::format_string<Types...> format, Types &&...args) {
        Debug(std::vformat(format.get(), std::make_format_args(args...)));
    }

    template <class... Types>
    void Info(const std::format_string<Types...> format, Types &&...args) {
        Info(std::vformat(format.get(), std::make_format_args(args...)));
    }

    template <class... Types>
    void Warning(const std::format_string<Types...> format, Types &&...args) {
        Warning(std::vformat(format.get(), std::make_format_args(args...)));
    }

    template <class... Types>
    void Error(const std::format_string<Types...> format, Types &&...args) {
        Error(std::vformat(format.get(), std::make_format_args(args...)));
    }

} inline logger;

#endif // __INC_AME_LOGGER_H__
