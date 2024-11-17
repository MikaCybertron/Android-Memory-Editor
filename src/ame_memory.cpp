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

#include "ame_memory.h"

#include <fstream>

std::optional<AddrRangeList> RawGetAddrRange(const pid_t &pid, std::function<bool(std::string_view)> predicate) {
    ShowInfo("Get Address Range Begin.");
    char filename[32];
    snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    std::fstream file(filename, std::ios::in);
    if (!file.is_open()) {
        ShowError("Get Address Range Failed.");
        return std::nullopt;
    }
    std::string str;
    uintptr_t beginAddr, endAddr;
    AddrRangeList addrRangeList;
    while (!file.eof()) {
        std::getline(file, str);
        if (str.find("rw") != std::string::npos && predicate(str)) {
            if (sscanf(str.c_str(), "%ld-%ld", &beginAddr, &endAddr) != EOF) {
                addrRangeList.push_back({beginAddr, endAddr});
            } else {
                ShowError("Error Get Address Range: {}", strerror(errno));
            }
        }
    }
    file.close();
    ShowInfo("Get Address Range End.");
    return addrRangeList;
}

std::optional<AddrRangeList> GetAddrRange(const pid_t &pid, MemoryZone zone) {
    switch (zone) {
        case MemoryZone::ALL: {
            return RawGetAddrRange(pid, [](std::string_view) { return true; });
        }
        case MemoryZone::ASHMEM: {
            return RawGetAddrRange(pid, [](std::string_view str) { return (str.find("/dev/ashmem/") != std::string::npos) && (str.find("dalvik") == std::string::npos); });
        }
        case MemoryZone::A_ANONMYOURS: {
            return RawGetAddrRange(pid, [](std::string_view str) { return str.size() < 42; });
        }
        case MemoryZone::B_BAD: {
            return RawGetAddrRange(pid, [](std::string_view str) { return str.find("/system/fonts") != std::string::npos; });
        }
        case MemoryZone::CODE_SYSTEM: {
            return RawGetAddrRange(pid, [](std::string_view str) { return str.find("/system") != std::string::npos; });
        }
        case MemoryZone::C_ALLOC: {
            return RawGetAddrRange(pid, [](std::string_view str) { return str.find("[anon:libc_malloc]") != std::string::npos; });
        }
        case MemoryZone::C_BSS: {
            return RawGetAddrRange(pid, [](std::string_view str) { return str.find("[anon:.bss]") != std::string::npos; });
        }
        case MemoryZone::C_DATA: {
            return RawGetAddrRange(pid, [](std::string_view str) { return str.find("/data/") != std::string::npos; });
        }
        case MemoryZone::C_HEAP: {
            return RawGetAddrRange(pid, [](std::string_view str) { return str.find("[heap]") != std::string::npos; });
        }
        case MemoryZone::JAVA_HEAP: {
            return RawGetAddrRange(pid, [](std::string_view str) { return str.find("/dev/ashmem/") != std::string::npos; });
        }
        case MemoryZone::STACK: {
            return RawGetAddrRange(pid, [](std::string_view str) { return str.find("[stack]") != std::string::npos; });
        }
        case MemoryZone::V: {
            return RawGetAddrRange(pid, [](std::string_view str) { return str.find("/dev/kgsl-3d0") != std::string::npos; });
        }
        default: {
            ShowError("Unexpected Case For MemoryZone: {}", static_cast<int>(zone));
            return std::nullopt;
        }
    }
};
