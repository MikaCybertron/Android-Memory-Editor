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
#include <string>

namespace ame {

bool IsAreaBelongToPart(MemPart memPart, const std::string &vmAreaStr) {
    switch (memPart) {
        case MemPart::ALL:
            return true;
        case MemPart::ASHMEM:
            return vmAreaStr.contains("/dev/ashmem/") && !vmAreaStr.contains("dalvik");
        case MemPart::A_ANONMYOURS:
            return vmAreaStr.length() < 42;
        case MemPart::B_BAD:
            return vmAreaStr.contains("/system/fonts");
        case MemPart::CODE_SYSTEM:
            return vmAreaStr.contains("/system");
        case MemPart::C_ALLOC:
            return vmAreaStr.contains("[anon:libc_malloc]");
        case MemPart::C_BSS:
            return vmAreaStr.contains("[anon:.bss]");
        case MemPart::C_DATA:
            return vmAreaStr.contains("/data/");
        case MemPart::C_HEAP:
            return vmAreaStr.contains("[heap]");
        case MemPart::JAVA_HEAP:
            return vmAreaStr.contains("/dev/ashmem/");
        case MemPart::STACK:
            return vmAreaStr.contains("[stack]");
        case MemPart::V:
            return vmAreaStr.contains("/dev/kgsl-3d0");
        default: {
            assert(false && "invalid value for MemPart");
            return false;
        }
    }
}


AddrRangeList GetAddrRange(pid_t pid, MemPart memPart) {
    AddrRangeList result;

    const std::string mapsPath = std::format("/proc/{}/maps", pid);
    std::ifstream mapsFile{mapsPath};
    if (!mapsFile.is_open()) {
        LOG_ERROR("Failed to open [{}].", mapsPath);
        return result;
    }

    for (std::string line; std::getline(mapsFile, line);) {
        if (line.find("rw") > 27 || !IsAreaBelongToPart(memPart, line)) {
            continue; // 27 -> the max columns of vm_flags
        }
        std::size_t hyphenPos;
        const std::uint64_t startAddr = std::stoull(line, &hyphenPos, 16);
        const std::uint64_t endAddr = std::strtoull(&line[hyphenPos + 1], nullptr, 16);
        result.emplace_back(startAddr, endAddr);
    }
    return result;
}

} // namespace ame
