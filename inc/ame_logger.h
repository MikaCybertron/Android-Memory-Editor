/*
 * Copyright (C) 2024  Dicot0721
 *
 * This file is part of Android-Memory-Editor.
 *
 * Android-Memory-Editor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Android-Memory-Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Android-Memory-Editor.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __INC_AME_LOGGER_H__
#define __INC_AME_LOGGER_H__

#include <cerrno>
#include <cstring>

#include <format>
#include <iostream>
#include <string_view>

inline void ShowInfo(std::string_view message) noexcept {
    std::cout << "[INFO] " << message << std::endl;
}

inline void ShowError(std::string_view message) noexcept {
    std::cerr << "[ERROR] " << message << std::endl;
}

template <class... Types>
void ShowInfo(const std::format_string<Types...> format, Types &&...args) {
    std::cout << "[INFO] " << std::vformat(format.get(), std::make_format_args(args...)) << std::endl;
}

template <class... Types>
void ShowError(const std::format_string<Types...> format, Types &&...args) {
    std::cerr << "[ERROR] " << std::vformat(format.get(), std::make_format_args(args...)) << std::endl;
}

#endif // __INC_AME_LOGGER_H__
