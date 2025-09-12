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

#ifndef __AME_MEMORY_H__
#define __AME_MEMORY_H__

#include "ame_file.h"
#include "ame_logger.h"

#include <fcntl.h>

#include <cstdint>

#include <format>
#include <utility>
#include <vector>

namespace ame {

// Use uint64_t rather than uintptr_t/unsigned long.
using AddrRangeList = std::vector<std::pair<std::uint64_t, std::uint64_t>>;
using AddrList = std::vector<std::uint64_t>;

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

/**
 * @brief Memory partition.
 */
enum class MemPart {
    ALL,          // 所有内存
    ASHMEM,       // AS内存
    A_ANONMYOURS, // A内存
    B_BAD,        // B/v内存
    CODE_SYSTEM,  // Xs内存
    C_ALLOC,      // CA内存
    C_BSS,        // CB内存
    C_DATA,       // CD内存
    C_HEAP,       // CH内存
    JAVA_HEAP,    // JH内存
    STACK,        // S内存
    V,            // v内存
};

[[nodiscard]] bool IsAreaBelongToPart(MemPart memPart, const std::string &vmAreaStr);

[[nodiscard]] AddrRangeList GetAddrRange(pid_t pid, MemPart memPart);


/**
 * @brief Find addresses that *address == value.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <Arithmetic T>
[[nodiscard]] AddrList FindAddress(pid_t pid, MemPart memPart, T valueToFind) {
    AddrList result;

    const AddrRangeList addrRangeList = GetAddrRange(pid, memPart);
    if (addrRangeList.empty()) {
        LOG_ERROR("Failed to get address range.");
        return result;
    }

    const std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile{memPath, O_RDONLY};
    if (!memFile.IsOpen()) {
        LOG_ERROR("Failed to open [{}].", memPath);
        return result;
    }

    LOG_INFO("Find address by value of ({}) start.", valueToFind);
    for (const auto &[beginAddr, endAddr] : addrRangeList) {
        for (std::uint64_t address = beginAddr; address <= endAddr; address += sizeof(std::int32_t)) {
            T value;
            if ((memFile.PRead64(&value, sizeof(value), address) > 0) && (value == valueToFind)) {
                result.push_back(address);
            }
        }
    }
    LOG_INFO("Find address end.");

    return result;
}


/**
 * @brief Find addresses that minValue <= *address <= maxValue.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <Arithmetic T>
[[nodiscard]] AddrList FindAddressByRange(pid_t pid, MemPart memPart, T minValue, T maxValue) {
    AddrList result;

    if (minValue > maxValue) {
        LOG_ERROR("minValue ({}) > maxValue ({})", minValue, maxValue);
        return result;
    }

    const AddrRangeList addrRangeList = GetAddrRange(pid, memPart);
    if (addrRangeList.empty()) {
        LOG_ERROR("Failed to get address range.");
        return result;
    }

    const std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile{memPath, O_RDONLY};
    if (!memFile.IsOpen()) {
        LOG_ERROR("Failed to open [{}].", memPath);
        return result;
    }

    LOG_INFO("Find address by value in ({}, {}) start.", minValue, maxValue);
    for (const auto &[beginAddr, endAddr] : addrRangeList) {
        for (std::uint64_t address = beginAddr; address <= endAddr; address += sizeof(std::int32_t)) {
            T value;
            if ((memFile.PRead64(&value, sizeof(value), address) > 0) //
                && (minValue <= value) && (value <= maxValue)) {
                result.push_back(address);
            }
        }
    }
    LOG_INFO("Find address end.");

    return result;
}


/**
 * @brief Find addresses that *((T *)address) == items[0], *((T *)address + 1) == values[1], ...
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <Arithmetic T>
[[nodiscard]] AddrList FindArrayAddress(pid_t pid, MemPart memPart, const std::vector<T> &values) {
    AddrList result;

    if (values.empty()) {
        LOG_ERROR("values is empty.");
        return result;
    }

    const AddrRangeList addrRangeList = GetAddrRange(pid, memPart);
    if (addrRangeList.empty()) {
        LOG_ERROR("Failed to get address range.");
        return result;
    }

    const std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile{memPath, O_RDONLY};
    if (!memFile.IsOpen()) {
        LOG_ERROR("Failed to open [{}].", memPath);
        return result;
    }

    LOG_INFO("Find address with group of values start.");
    std::vector<T> buffer(values.size());
    for (const auto &[beginAddr, endAddr] : addrRangeList) {
        for (std::uint64_t address = beginAddr; address <= endAddr; address += sizeof(std::int32_t)) {
            if ((memFile.PRead64(buffer.data(), buffer.size() * sizeof(T), address) > 0) && (buffer == values)) {
                LOG_DEBUG("Find Address: 0x{:X}", address);
                result.push_back(address);
            }
        }
    }
    LOG_INFO("Find address end.");

    return result;
}


/**
 * @brief Find addresses in list that *(address + offset) == value.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <Arithmetic T>
[[nodiscard]] AddrList FilterAddrListByOffset(pid_t pid, const AddrList &listToFilter, T valueToFind, std::int64_t offset) {
    AddrList result;

    const std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile{memPath, O_RDONLY};
    if (!memFile.IsOpen()) {
        LOG_ERROR("Failed to open [{}].", memPath);
        return result;
    }

    LOG_INFO("Filter address by value of ({}) and offset of ({}) start.", valueToFind, offset);
    for (const auto &address : listToFilter) {
        T value;
        if ((memFile.PRead64(&value, sizeof(value), address + offset) > 0) && (value == valueToFind)) {
            LOG_DEBUG("Find Address: 0x{:X}", address);
            result.push_back(address);
        }
    }
    LOG_INFO("Filter address end.");

    return result;
}


/**
 * @brief Find addresses in list that *address == value.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <Arithmetic T>
[[nodiscard]] AddrList FilterAddrList(pid_t pid, const AddrList &listToFilter, T value) {
    return FilterAddrListByOffset(pid, listToFilter, value, 0);
}


/**
 * @brief Find addresses in list that minValue <= *address <= maxValue.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <Arithmetic T>
[[nodiscard]] AddrList FilterAddrListByRange(pid_t pid, const AddrList &listToFilter, T minValue, T maxValue) {
    AddrList result;

    if (minValue > maxValue) {
        LOG_ERROR("minValue ({}) > maxValue ({})", minValue, maxValue);
        return result;
    }

    const std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile{memPath, O_RDONLY};
    if (!memFile.IsOpen()) {
        LOG_ERROR("Failed to open [{}].", memPath);
        return result;
    }

    LOG_INFO("Filter address by value in ({}, {}) start.", minValue, maxValue);
    for (const auto &address : listToFilter) {
        T value;
        if ((memFile.PRead64(&value, sizeof(value), address) > 0) //
            && (minValue <= value) && (value <= maxValue)) {
            LOG_DEBUG("Find Address: 0x{:X}", address);
            result.push_back(address);
        }
    }
    LOG_INFO("Filter address end.");

    return result;
}


/**
 * @brief Write the value to the addresses in addrList.
 *
 * @tparam T  base data type, e.g. short, int, float, long.
 * @param [in] groupSize  The number of addresses to be written.
 * @return Count of successful writes.
 */
template <Arithmetic T>
int WriteAddressGroup(pid_t pid, const AddrList &addrList, T value, std::size_t groupSize = 1) {
    if (groupSize == 0) {
        LOG_ERROR("groupSize is zero.");
        return -1;
    }

    const std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile{memPath, O_WRONLY};
    if (!memFile.IsOpen()) {
        LOG_ERROR("Failed to open [{}].", memPath);
        return -1;
    }

    int successCount = 0;
    auto it = addrList.cbegin();
    for (std::size_t i = 0; (i < groupSize) && (it != addrList.cend()); ++i, ++it) {
        if (memFile.PWrite64(&value, sizeof(value), *it) != -1) {
            ++successCount;
        }
    }
    return successCount;
}


/**
 * @brief Copy values to each address in addrList.
 * @tparam T  base data type, e.g. short, int, float, long.
 * @return Count of successful writes.
 */
template <Arithmetic T>
int WriteArrayAddress(pid_t pid, const AddrList &addrList, const std::vector<T> &values) {
    if (values.empty()) {
        LOG_ERROR("values is empty.");
        return -1;
    }

    const std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile{memPath, O_WRONLY};
    if (!memFile.IsOpen()) {
        LOG_ERROR("Failed to open [{}].", memPath);
        return -1;
    }

    int successCount = 0;
    for (const auto &address : addrList) {
        if (memFile.PWrite64(values.data(), values.size() * sizeof(T), address) != -1) {
            ++successCount;
        }
    }
    return successCount;
}

} // namespace ame

#endif // __AME_MEMORY_H__
