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

#include "ame_memory.h"
#include "ame_logger.h"

#include <sys/types.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <format>
#include <fstream>
#include <optional>
#include <string>

bool IsAreaBelongToPart(MemPart memPart, const std::string &vmAreaStr) {
    switch (memPart) {
        case MemPart::ALL: {
            return true;
        }
        case MemPart::ASHMEM: {
            return vmAreaStr.find("/dev/ashmem/") != std::string::npos //
                && vmAreaStr.find("dalvik") == std::string::npos;
        }
        case MemPart::A_ANONMYOURS: {
            return vmAreaStr.length() < 42;
        }
        case MemPart::B_BAD: {
            return vmAreaStr.find("/system/fonts") != std::string::npos;
        }
        case MemPart::CODE_SYSTEM: {
            return vmAreaStr.find("/system") != std::string::npos;
        }
        case MemPart::C_ALLOC: {
            return vmAreaStr.find("[anon:libc_malloc]") != std::string::npos;
        }
        case MemPart::C_BSS: {
            return vmAreaStr.find("[anon:.bss]") != std::string::npos;
        }
        case MemPart::C_DATA: {
            return vmAreaStr.find("/data/") != std::string::npos;
        }
        case MemPart::C_HEAP: {
            return vmAreaStr.find("[heap]") != std::string::npos;
        }
        case MemPart::JAVA_HEAP: {
            return vmAreaStr.find("/dev/ashmem/") != std::string::npos;
        }
        case MemPart::STACK: {
            return vmAreaStr.find("[stack]") != std::string::npos;
        }
        case MemPart::V: {
            return vmAreaStr.find("/dev/kgsl-3d0") != std::string::npos;
        }
        [[unlikely]] default: {
            assert(false && "Invalid value of MemPart: " && int(memPart));
            return false;
        }
    }
}


std::optional<AddrRangeList> GetAddrRange(pid_t pid, MemPart memPart) {
    std::string mapsPath = std::format("/proc/{}/maps", pid);
    std::ifstream mapsFile(mapsPath);
    if (!mapsFile.is_open()) {
        logger.Error("Failed to open [{}].", mapsPath);
        return std::nullopt;
    }

    AddrRangeList addrRangeList;
    for (std::string line; std::getline(mapsFile, line);) {
        if (line.find("rw") > 27 || !IsAreaBelongToPart(memPart, line)) {
            continue; // 27 -> the max columns of vm_flags
        }
        size_t hyphenPos;
        uint64_t startAddr = std::stoull(line, &hyphenPos, 16);
        uint64_t endAddr = std::strtoull(&line[hyphenPos + 1], nullptr, 16);
        addrRangeList.push_back({startAddr, endAddr});
    }
    return addrRangeList;
}
