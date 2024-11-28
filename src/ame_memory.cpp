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

#include "ame_memory.h"
#include "ame_logger.h"

#include <sys/types.h>

#include <cstdint>

#include <format>
#include <fstream>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

std::optional<AddrRangeList> GetAddrRange(pid_t pid, std::function<bool(const std::string &)> predicate) {
    logger.Debug("Get address range start.");
    std::string mapsPath = std::format("/proc/{}/maps", pid);
    std::ifstream mapsFile(mapsPath);
    if (!mapsFile.is_open()) {
        logger.Error("Failed to open: {}:", mapsPath);
        return std::nullopt;
    }
    std::string line;
    std::istringstream lineStream;
    uint64_t beginAddr, endAddr;
    AddrRangeList addrRangeList;
    while (std::getline(mapsFile, line)) {
        if (line.find("rw") != std::string::npos && predicate(line)) {
            char tmpChar; // to skip character '-'
            lineStream.str(line);
            lineStream >> std::hex >> beginAddr >> tmpChar >> endAddr;
            addrRangeList.push_front({beginAddr, endAddr});
        }
    }
    mapsFile.close();
    logger.Debug("Get address range end.");
    return addrRangeList;
}

std::optional<AddrRangeList> GetAddrRangeByZone(pid_t pid, MemoryZone zone) {
    switch (zone) {
        case MemoryZone::ALL: {
            return GetAddrRange(pid, [](auto &) { //
                return true;
            });
        }
        case MemoryZone::ASHMEM: {
            return GetAddrRange(pid, [](auto &str) {                 //
                return str.find("/dev/ashmem/") != std::string::npos //
                    && str.find("dalvik") == std::string::npos;
            });
        }
        case MemoryZone::A_ANONMYOURS: {
            return GetAddrRange(pid, [](auto &str) { //
                return str.length() < 42;
            });
        }
        case MemoryZone::B_BAD: {
            return GetAddrRange(pid, [](auto &str) { //
                return str.find("/system/fonts") != std::string::npos;
            });
        }
        case MemoryZone::CODE_SYSTEM: {
            return GetAddrRange(pid, [](auto &str) { //
                return str.find("/system") != std::string::npos;
            });
        }
        case MemoryZone::C_ALLOC: {
            return GetAddrRange(pid, [](auto &str) { //
                return str.find("[anon:libc_malloc]") != std::string::npos;
            });
        }
        case MemoryZone::C_BSS: {
            return GetAddrRange(pid, [](auto &str) { //
                return str.find("[anon:.bss]") != std::string::npos;
            });
        }
        case MemoryZone::C_DATA: {
            return GetAddrRange(pid, [](auto &str) { //
                return str.find("/data/") != std::string::npos;
            });
        }
        case MemoryZone::C_HEAP: {
            return GetAddrRange(pid, [](auto &str) { //
                return str.find("[heap]") != std::string::npos;
            });
        }
        case MemoryZone::JAVA_HEAP: {
            return GetAddrRange(pid, [](auto &str) { //
                return str.find("/dev/ashmem/") != std::string::npos;
            });
        }
        case MemoryZone::STACK: {
            return GetAddrRange(pid, [](auto &str) { //
                return str.find("[stack]") != std::string::npos;
            });
        }
        case MemoryZone::V: {
            return GetAddrRange(pid, [](auto &str) { //
                return str.find("/dev/kgsl-3d0") != std::string::npos;
            });
        }
        [[unlikely]] default: {
            logger.Error("Unexpected Case For MemoryZone: {}", static_cast<int>(zone));
            return std::nullopt;
        }
    }
};
