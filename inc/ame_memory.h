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
#include <optional>
#include <utility>
#include <vector>

// Use uint64_t rather than uintptr_t/unsigned long.
using AddrRangeList = std::vector<std::pair<uint64_t, uint64_t>>;
using AddrList = std::vector<uint64_t>;

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

[[nodiscard]] std::optional<AddrRangeList> GetAddrRange(pid_t pid, MemPart memPart);


/**
 * @brief Find addresses that *address == value.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FindAddress(pid_t pid, MemPart memPart, T valueToFind) {
    auto addrRangeList = GetAddrRange(pid, memPart);
    if (!addrRangeList.has_value()) {
        logger.Error("Failed to find address: failed to get address range.");
        return std::nullopt;
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile(memPath, O_RDONLY);
    if (!memFile.IsOpen()) {
        logger.Error("Failed to find address: failed to open [{}].", memPath);
        return std::nullopt;
    }

    logger.Info("Find address by value of ({}) start.", valueToFind);
    AddrList addrList;
    for (const auto &[beginAddr, endAddr] : *addrRangeList) {
        for (uint64_t address = beginAddr; address <= endAddr; address += sizeof(int32_t)) {
            T value{};
            if (memFile.Pread64(&value, sizeof(value), address) > 0 && value == valueToFind) {
                addrList.push_back(address);
            }
        }
    }
    logger.Info("Find address end.");

    return addrList;
}


/**
 * @brief Find addresses that minValue <= *address <= maxValue.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FindAddressByRange(pid_t pid, MemPart memPart, T minValue, T maxValue) {
    if (minValue > maxValue) {
        logger.Error("Failed to find address: minValue > maxValue. ({} > {})", minValue, maxValue);
        return std::nullopt;
    }

    auto addrRangeList = GetAddrRange(pid, memPart);
    if (!addrRangeList.has_value()) {
        logger.Error("Failed to find address: failed to get address range.");
        return std::nullopt;
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile(memPath, O_RDONLY);
    if (!memFile.IsOpen()) {
        logger.Error("Failed to find address: failed to open [{}].", memPath);
        return std::nullopt;
    }

    logger.Info("Find address by value in ({}, {}) start.", minValue, maxValue);
    AddrList addrList;
    for (const auto &[beginAddr, endAddr] : *addrRangeList) {
        for (uint64_t address = beginAddr; address <= endAddr; address += sizeof(int32_t)) {
            T value{};
            if (memFile.Pread64(&value, sizeof(value), address) > 0 //
                && minValue <= value && value <= maxValue) {
                addrList.push_back(address);
            }
        }
    }
    logger.Info("Find address end.");

    return addrList;
}


/**
 * @brief Find addresses that *((T *)address) == items[0], *((T *)address + 1) == items[1], ...
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FindArrayAddress(pid_t pid, MemPart memPart, const std::vector<T> &items) {
    if (items.empty()) {
        logger.Error("Failed to find address: items is empty.");
        return std::nullopt;
    }

    auto addrRangeList = GetAddrRange(pid, memPart);
    if (!addrRangeList.has_value()) {
        logger.Error("Failed to find address: failed to get address range.");
        return std::nullopt;
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile(memPath, O_RDONLY);
    if (!memFile.IsOpen()) {
        logger.Error("Failed to find address: failed to open [{}].", memPath);
        return std::nullopt;
    }

    logger.Info("Find address with group of values start.");
    std::vector<T> buffer(items.size());
    AddrList addrList;
    for (const auto &[beginAddr, endAddr] : *addrRangeList) {
        for (uint64_t address = beginAddr; address <= endAddr; address += sizeof(int32_t)) {
            if (memFile.Pread64(buffer.data(), buffer.size() * sizeof(T), address) > 0 && buffer == items) {
                logger.Debug("Find Address: 0x{:X}", address);
                addrList.push_back(address);
            }
        }
    }
    logger.Info("Find address end.");

    return addrList;
}


/**
 * @brief Find addresses in list that *(address + offset) == value.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FilterAddrListByOffset(pid_t pid, const AddrList &listToFilter, T valueToFind, int64_t offset) {
    std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile(memPath, O_RDONLY);
    if (!memFile.IsOpen()) {
        logger.Error("Failed to filter address: failed to open [{}].", memPath);
        return std::nullopt;
    }

    logger.Info("Filter address by value of ({}) and offset of ({}) start.", valueToFind, offset);
    AddrList listToReturn;
    for (const auto &address : listToFilter) {
        T value{};
        if (memFile.Pread64(&value, sizeof(value), address + offset) > 0 && value == valueToFind) {
            logger.Debug("Find Address: 0x{:X}", address);
            listToReturn.push_back(address);
        }
    }
    logger.Info("Filter address end.");

    return listToReturn;
}


/**
 * @brief Find addresses in list that *address == value.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FilterAddrList(pid_t pid, const AddrList &listToFilter, T value) {
    return FilterAddrListByOffset(pid, listToFilter, value, 0);
}


/**
 * @brief Find addresses in list that minValue <= *address <= maxValue.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FilterAddrListByRange(pid_t pid, const AddrList &listToFilter, T minValue, T maxValue) {
    if (minValue > maxValue) {
        logger.Error("Failed to filter address: minValue > maxValue. ({} > {})", minValue, maxValue);
        return std::nullopt;
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile(memPath, O_RDONLY);
    if (!memFile.IsOpen()) {
        logger.Error("Failed to filter address: failed to open [{}].", memPath);
        return std::nullopt;
    }

    logger.Info("Filter address by value in ({}, {}) start.", minValue, maxValue);
    AddrList listToReturn;
    for (const auto &address : listToFilter) {
        T value{};
        if (memFile.Pread64(&value, sizeof(value), address) > 0 //
            && minValue <= value && value <= maxValue) {
            logger.Debug("Find Address: 0x{:X}", address);
            listToReturn.push_back(address);
        }
    }
    logger.Info("Filter address end.");

    return listToReturn;
}


/**
 * @brief Write the value to the addresses in addrList.
 *
 * @tparam T  base data type, e.g. short, int, float, long.
 * @param [in] groupSize  The number of addresses to be written.
 * @return Count of successful writes.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
int WriteAddressGroup(pid_t pid, const AddrList &addrList, T value, int groupSize = 1) {
    if (groupSize < 1) {
        logger.Error("Failed to write meory: groupSize ({}) less than one.", groupSize);
        return -1;
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile(memPath, O_WRONLY);
    if (!memFile.IsOpen()) {
        logger.Error("Failed to write meory: failed to open [{}].", memPath);
        return -1;
    }

    int successCount = 0;
    for (int i = 1; const auto &address : addrList) {
        if (memFile.Pwrite64(&value, sizeof(value), address) != -1) {
            ++successCount;
        }
        if (++i > groupSize) {
            break;
        }
    }
    return successCount;
}


/**
 * @brief Copy values of items to each "array" in addrList.
 * @tparam T  base data type, e.g. short, int, float, long.
 * @return Count of successful writes.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
int WriteArrayAddress(pid_t pid, const AddrList &addrList, const std::vector<T> &items) {
    if (items.empty()) {
        logger.Error("Failed to write array address: items is empty.");
        return -1;
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    FileWrapper memFile(memPath, O_WRONLY);
    if (!memFile.IsOpen()) {
        logger.Error("Failed to write array address: failed to open [{}].", memPath);
        return -1;
    }

    int successCount = 0;
    for (const auto &address : addrList) {
        if (memFile.Pwrite64(items.data(), items.size() * sizeof(T), address) != -1) {
            ++successCount;
        }
    }
    return successCount;
}

#endif // __AME_MEMORY_H__
