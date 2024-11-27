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

#ifndef __INC_AME_MEMORY_H__
#define __INC_AME_MEMORY_H__

#include "ame_logger.h"

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>

#include <forward_list>
#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>
#include <vector>

// uint64_t rather than uintptr_t/unsigned long/unsigned long long.
using AddrRangeList = std::forward_list<std::pair<uint64_t, uint64_t>>;
using AddrList = std::forward_list<uint64_t>;

// 内存区域
enum class MemoryZone {
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

[[nodiscard]] std::optional<AddrRangeList> GetAddrRange(pid_t pid, std::function<bool(std::string_view)> predicate);

[[nodiscard]] std::optional<AddrRangeList> GetAddrRangeByZone(pid_t pid, MemoryZone zone);


/**
 * @brief Find addresses that *address == value.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FindAddress(pid_t pid, MemoryZone zone, const T &&valueToFind) {
    auto addrRangeList = GetAddrRangeByZone(pid, zone);
    if (!addrRangeList.has_value()) {
        logger.Error("Failed to find address: failed to get address range.");
        return std::nullopt;
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    int memFile = open(memPath.c_str(), O_RDONLY);
    if (memFile == -1) {
        logger.Error("Failed to find address: failed to open {}", memPath);
        return std::nullopt;
    }

    logger.Debug("Find address by value of {} start.", valueToFind);
    AddrList addrList;
    for (const auto &[beginAddr, endAddr] : *addrRangeList) {
        for (uint64_t address = beginAddr; address <= endAddr; address += sizeof(int32_t)) {
            T value = 0;
            if (pread64(memFile, &value, sizeof(T), address) <= 0) {
                // logger.Error("pread failed.");
                continue;
            }
            if (value == valueToFind) {
                // logger.Debug("Find Address: 0x{:X}", address);
                addrList.push_front(address);
            }
        }
    }
    logger.Debug("Find address end.");

    close(memFile);
    return addrList;
};


/**
 * @brief Find addresses that minValue <= *address <= maxValue.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FindAddressByRange(pid_t pid, MemoryZone zone, const T &&minValue, const T &&maxValue) {
    if (minValue > maxValue) {
        logger.Error("Failed to find address: minValue > maxValue. ({}, {})", minValue, maxValue);
        return std::nullopt;
    }

    auto addrRangeList = GetAddrRangeByZone(pid, zone);
    if (!addrRangeList.has_value()) {
        logger.Error("Failed to find address: failed to get address range.");
        return std::nullopt;
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    int memFile = open(memPath.c_str(), O_RDONLY);
    if (memFile == -1) {
        logger.Error("Failed to find address: failed to open {}", memPath);
        return std::nullopt;
    }

    logger.Debug("Find address by value in ({}, {}) start.", minValue, maxValue);
    AddrList addrList;
    for (const auto &[beginAddr, endAddr] : *addrRangeList) {
        for (uint64_t address = beginAddr; address <= endAddr; address += sizeof(int32_t)) {
            T value = 0;
            if (pread64(memFile, &value, sizeof(T), address) <= 0) {
                // logger.Error("pread failed.");
                continue;
            }
            if (minValue <= value && value <= maxValue) {
                // logger.Debug("Find Address: 0x{:X}", address);
                addrList.push_front(address);
            }
        }
    }
    logger.Debug("Find address end.");

    close(memFile);
    return addrList;
};


/**
 * @brief Find addresses that *((T *)address) == items[0], *((T *)address + 1) == items[1], ...
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FindArrayAddress(pid_t pid, MemoryZone zone, const std::vector<T> &&items) {
    if (items.empty()) {
        logger.Error("Failed to find address: items is empty.");
        return std::nullopt;
    }

    auto addrRangeList = GetAddrRangeByZone(pid, zone);
    if (!addrRangeList.has_value()) {
        logger.Error("Failed to find address: failed to get address range.");
        return std::nullopt;
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    int memFile = open(memPath.c_str(), O_RDONLY);
    if (memFile == -1) {
        logger.Error("Failed to find address: failed to open {}", memPath);
        return std::nullopt;
    }

    logger.Debug("Find address with group of values start.");
    AddrList addrList;
    for (const auto &[beginAddr, endAddr] : *addrRangeList) {
        for (uint64_t address = beginAddr; address <= endAddr; address += sizeof(int32_t)) {
            bool flag = true;
            for (size_t i = 0; i < items.size(); ++i) {
                T value = 0;
                if (pread64(memFile, &value, sizeof(T), address + sizeof(T) * i) <= 0 || value != items[i]) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                addrList.push_front(address);
            }
        }
    }
    logger.Debug("Find address end.");

    close(memFile);
    return addrList;
};


/**
 * @brief Find addresses in list that *(address + offset) == value.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FilterAddrListByOffset(pid_t pid, const AddrList &&listToFilter, const T &&valueToFind, int64_t offset) {
    std::string memPath = std::format("/proc/{}/mem", pid);
    int memFile = open(memPath.c_str(), O_RDONLY);
    if (memFile == -1) {
        logger.Error("Failed to filter address: failed to open {}", memPath);
        return std::nullopt;
    }

    logger.Debug("Filter address by value of {} and offset of {} start.", valueToFind, offset);
    AddrList listToReturn;
    for (const auto &address : listToFilter) {
        T value = 0;
        if (pread64(memFile, &value, sizeof(T), address + offset) <= 0) {
            // logger.Error("pread failed.");
            continue;
        }
        if (value == valueToFind) {
            // logger.Debug("Find Address: 0x{:X}", address);
            listToReturn.push_front(address);
        }
    }
    logger.Debug("Filter address end.");

    close(memFile);
    return listToReturn;
};


/**
 * @brief Find addresses in list that *address == value.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FilterAddrList(pid_t pid, const AddrList &&listToFilter, const T &&value) {
    return FilterAddrListByOffset(pid, listToFilter, value, 0);
};


/**
 * @brief Find addresses in list that minValue <= *address <= maxValue.
 * @tparam T  base data type, e.g. short, int, float, long.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] std::optional<AddrList> FilterAddrListByRange(pid_t pid, const AddrList &&listToFilter, const T &&minValue, const T &&maxValue) {
    if (minValue > maxValue) {
        logger.Error("Failed to filter address: minValue > maxValue. ({}, {})", minValue, maxValue);
        return std::nullopt;
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    int memFile = open(memPath.c_str(), O_RDONLY);
    if (memFile == -1) {
        logger.Error("Failed to filter address: failed to open {}", memPath);
        return std::nullopt;
    }

    logger.Debug("Filter address by value in ({}, {}) start.", minValue, maxValue);
    AddrList listToReturn;
    for (const auto &address : listToFilter) {
        T value = 0;
        if (pread64(memFile, &value, sizeof(T), address) <= 0) {
            // logger.Error("pread failed.");
            continue;
        }
        if (minValue <= value && value <= maxValue) {
            // logger.Debug("Find Address: 0x{:X}", address);
            listToReturn.push_front(address);
        }
    }
    logger.Debug("Filter address end.");

    close(memFile);
    return listToReturn;
};


/**
 * @brief Write the value to the addresses in addrList.
 *
 * @tparam T  base data type, e.g. short, int, float, long.
 * @param [in] groupSize  The number of addresses to be written.
 * @return Count of successful writes.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
int WriteAddressGroup(pid_t pid, const AddrList &&addrList, const T &&value, int groupSize = 1) {
    if (groupSize < 1) {
        logger.Error("Failed to write meory: groupSize={} less than one.", groupSize);
        return -1;
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    int memFile = open(memPath.c_str(), O_WRONLY);
    if (memFile == -1) {
        logger.Error("Failed to write meory: failed to open {}", memPath);
        return -1;
    }

    int i = 1, successCount = 0;
    for (const auto &address : addrList) {
        if (pwrite64(memFile, &value, sizeof(T), address) > 0) {
            ++successCount;
        } else {
            logger.Error("The {} times write failed.", i);
        }
        if (++i > groupSize) {
            break;
        }
    }
    close(memFile);
    return successCount;
}


/**
 * @brief Copy values of items to each "array" in addrList.
 * @tparam T  base data type, e.g. short, int, float, long.
 * @return Counts of successful writes for each address.
 */
template <typename T>
    requires std::is_arithmetic_v<T>
std::vector<int> WriteArrayAddress(pid_t pid, const AddrList &&addrList, const std::vector<T> &&items) {
    if (items.empty()) {
        logger.Error("Failed to write array address: items is empty.");
        return {};
    }

    std::string memPath = std::format("/proc/{}/mem", pid);
    int memFile = open(memPath.c_str(), O_WRONLY);
    if (memFile == -1) {
        logger.Error("Failed to write array address: failed to open {}", memPath);
        return {};
    }

    std::vector<int> successCountVec;
    for (const auto &address : addrList) {
        int successCount = 0;
        for (size_t i = 0; i < items.size(); ++i) {
            if (pwrite64(memFile, &items[i], sizeof(T), address + sizeof(T) * i) > 0) {
                ++successCount;
            }
        }
        successCountVec.push_back(successCount);
    }
    close(memFile);
    return successCountVec;
};


#endif // __INC_AME_MEMORY_H__
