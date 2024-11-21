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
#include <memory>
#include <type_traits>

using AddrRangeList = std::forward_list<std::pair<uintptr_t, uintptr_t>>;
using AddrList = std::forward_list<uintptr_t>;

inline constexpr int BUFF_SIZE = 256;

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

[[nodiscard]] std::optional<AddrRangeList> RawGetAddrRange(pid_t pid, std::function<bool(std::string_view)> predicate);

[[nodiscard]] std::optional<AddrRangeList> GetAddrRange(pid_t pid, MemoryZone zone);

/**
 * @brief Search in zone for addresses that *addaess == value.
 */
template <typename T>
    requires std::is_arithmetic_v<T> // "T" must be the base data type, e.g. short, int, float.
[[nodiscard]] std::optional<AddrList> PreSearch(pid_t pid, MemoryZone zone, const T &&value) {
    auto addrRangeList = GetAddrRange(pid, zone);
    if (!addrRangeList.has_value()) {
        logger.Error("Get Address Range Failed.");
        return std::nullopt;
    }
    char filename[32];
    snprintf(filename, sizeof(filename), "/proc/%d/mem", pid);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        logger.Error("Search Faild.");
        return std::nullopt;
    }
    logger.Info("Search Value Begin: ()", value);
    AddrList addrList;
    auto buff = std::make_unique<T[]>(BUFF_SIZE);
    for (const auto &[beginAddr, endAddr] : *addrRangeList) {
        for (uintptr_t addr = beginAddr; addr <= endAddr; addr += 0x4) { // I don't know if "0x4" could be other value.
            memset(buff, 0, sizeof(T));
            ssize_t readResult = pread64(fd, buff, sizeof(T), addr);
            if (readResult == -1) {
                // logger.Error("Pread Failed.");
                continue;
            }
            if (buff[0] != value) {
                continue;
            }
            logger.Info("Find Address: 0x{:X}", addr);
            addrList.push_front(addr);
        }
    }
    close(fd);
    return addrList;
};

/**
 * @brief Search in zone for addresses that minValue <= *addaess <= maxValue.
 */
template <typename T>
    requires std::is_arithmetic_v<T> // "T" must be the base data type, e.g. short, int, float.
[[nodiscard]] std::optional<AddrList> PreSearch(pid_t pid, MemoryZone zone, const T &&minValue, const T &&maxValue) {
    if (minValue > maxValue) {
        logger.Error("Search No Start: minValue > maxValue. ({}, {})", minValue, maxValue);
        return std::nullopt;
    }
    auto addrRangeList = GetAddrRange(pid, zone);
    if (!addrRangeList.has_value()) {
        logger.Error("Get Address Range Failed.");
        return std::nullopt;
    }
    char filename[32];
    snprintf(filename, sizeof(filename), "/proc/%d/mem", pid);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        logger.Error("Search Faild.");
        return std::nullopt;
    }
    logger.Info("Search Value In Range Begin: ({}, {})", minValue, maxValue);
    AddrList addrList;
    auto buff = std::make_unique<T[]>(BUFF_SIZE);
    for (const auto &[beginAddr, endAddr] : *addrRangeList) {
        for (uintptr_t addr = beginAddr; addr <= endAddr; addr += 0x4) { // I don't know if "0x4" could be other value.
            memset(buff, 0, sizeof(T));
            ssize_t readResult = pread64(fd, buff, sizeof(T), addr);
            if (readResult == -1) {
                // logger.Error("Pread Failed.");
                continue;
            }
            if (buff[0] < minValue || maxValue < buff[0]) {
                continue;
            }
            logger.Info("Find Address: 0x{:X}", addr);
            addrList.push_front(addr);
        }
    }
    close(fd);
    return addrList;
};

/**
 * @brief Search the targetAddrList for addresses that *addaess == value.
 */
template <typename T>
    requires std::is_arithmetic_v<T> // "T" must be the base data type, e.g. short, int, float.
[[nodiscard]] std::optional<AddrList> Search(const AddrList &targetAddrList, const T &&value) {
    return SearchByOffset(targetAddrList, value, 0);
};

/**
 * @brief Search the targetAddrList for addresses that *(addaess + offset) == value.
 */
template <typename T>
    requires std::is_arithmetic_v<T> // "T" must be the base data type, e.g. short, int, float.
[[nodiscard]] std::optional<AddrList> SearchByOffset(pid_t pid, const AddrList &targetAddrList, const T &&value, const uintptr_t offset) {
    char filename[32];
    snprintf(filename, sizeof(filename), "/proc/%d/mem", pid);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        logger.Error("Search Faild.");
        return std::nullopt;
    }
    logger.Info("Search Value Begin: ()", value);
    AddrList resultAddrList;
    auto buff = std::make_unique<T[]>(BUFF_SIZE);
    for (const auto &addr : targetAddrList) {
        memset(buff, 0, sizeof(T));
        ssize_t readResult = pread64(fd, buff, sizeof(T), addr + offset);
        if (readResult == -1) {
            // logger.Error("Pread Failed.");
            continue;
        }
        if (buff[0] != value) {
            continue;
        }
        logger.Info("Find Address: 0x{:X}", addr);
        resultAddrList.push_front(addr);
    }
    close(fd);
    return resultAddrList;
};

/**
 * @brief Search the targetAddrList for addresses that minValue <= *addaess <= maxValue.
 */
template <typename T>
    requires std::is_arithmetic_v<T> // "T" must be the base data type, e.g. short, int, float.
[[nodiscard]] std::optional<AddrList> Search(pid_t pid, const AddrList &targetAddrList, const T &&minValue, const T &&maxValue) {
    if (minValue > maxValue) {
        logger.Error("Search No Start: minValue > maxValue. ({}, {})", minValue, maxValue);
        return std::nullopt;
    }
    char filename[32];
    snprintf(filename, sizeof(filename), "/proc/%d/mem", pid);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        logger.Error("Search Faild.");
        return std::nullopt;
    }
    logger.Info("Search Value In Range Begin: ({}, {})", minValue, maxValue);
    AddrList resultAddrList;
    auto buff = std::make_unique<T[]>(BUFF_SIZE);
    for (const auto &addr : targetAddrList) {
        memset(buff, 0, sizeof(T));
        ssize_t readResult = pread64(fd, buff, sizeof(T), addr);
        if (readResult == -1) {
            // logger.Error("Pread Failed.");
            continue;
        }
        if (buff[0] < minValue || maxValue < buff[0]) {
            continue;
        }
        logger.Info("Find Address: 0x{:X}", addr);
        resultAddrList.push_front(addr);
    }
    close(fd);
    return resultAddrList;
};

/**
 * @brief Write the value to the addresses in addrList.
 *
 * @param addrCount The number of addresses to be written.
 * @return The number of successful writes.
 */
template <typename T>
    requires std::is_arithmetic_v<T> // "T" must be the base data type, e.g. short, int, float.
int WriteMeory(pid_t pid, const AddrList &addrList, const T &&value, const int &addrCount) {
    if (addrCount <= 0) {
        logger.Error("Address Count Could Not Less Than One: {}", addrCount);
        return 0;
    }
    char filename[32];
    snprintf(filename, sizeof(filename), "/proc/%d/mem", pid);
    int fd = open(filename, O_WRONLY);
    if (fd == -1) {
        logger.Error("Search Faild.");
        return 0;
    }
    int successCount = 0, i = 1;
    for (const auto &addr : addrList) {
        if (pwrite64(fd, &value, sizeof(T), addr) != -1) {
            ++successCount;
        } else {
            logger.Error("The i-th Write Failed: {}", i);
        }
        if (++i > addrCount) {
            break;
        }
    }
    return successCount;
}

#endif // __INC_AME_MEMORY_H__
