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

#include "ame_process.h"
#include "ame_logger.h"

#include <dirent.h>
#include <unistd.h>

#include <csignal>
#include <cstdlib>
#include <cstring>

#include <format>
#include <fstream>
#include <optional>
#include <regex>
#include <string>
#include <string_view>

std::optional<pid_t> FindPidByPackageName(std::string_view packageName) {
    DIR *dirp = opendir("/proc");
    if (dirp == nullptr) {
        logger.Error("pid not find: {}:", strerror(errno));
        return std::nullopt;
    }
    dirent *dp = nullptr;
    std::optional<pid_t> result = std::nullopt;
    while ((dp = readdir(dirp)) != nullptr) {
        if (dp->d_type != DT_DIR) {
            // not a directory
            continue;
        }
        std::string_view pidStr = dp->d_name;
        if (pidStr.find_first_not_of("0123456789") != std::string::npos) {
            // not numeric name
            continue;
        }
        std::string cmdlinePath = std::format("/proc/{}/cmdline", pidStr);
        std::ifstream cmdlineFile(cmdlinePath);
        if (!cmdlineFile.is_open()) {
            logger.Error("Failed to open: {}:", cmdlinePath);
            continue;
        }
        std::string cmdline;
        std::getline(cmdlineFile, cmdline, '\0');
        cmdlineFile.close();
        if (cmdline == packageName) {
            result = atoi(pidStr.data());
            break;
        }
    }
    closedir(dirp);
    return result;
}

std::optional<bool> IsProcessStopped(pid_t pid) {
    std::string statusPath = std::format("/proc/{}/status", pid);
    std::ifstream statusFile(statusPath);
    if (!statusFile.is_open()) {
        logger.Error("Failed to open: {}:", statusPath);
        return std::nullopt;
    }
    std::string line;
    std::smatch matches;
    std::regex stateRegex("^State:\\s+\\b(\\S+?)\\b\\s+\\((\\S+?)\\)");
    while (getline(statusFile, line)) {
        if (!std::regex_search(line, matches, stateRegex)) {
            continue;
        }
        std::string stateCode = matches[1].str();
        if (stateCode == "T") {
            logger.Debug("Process {} is in stopped state.", pid);
            return true;
        } else {
            std::string state = matches[2].str();
            logger.Debug("Process {} is not in stopped state: {} ({}).", pid, stateCode, state);
            return false;
        }
    }
    // statusFile.close();
    return std::nullopt;
}

bool FreezeProcessByPid(pid_t pid) {
    if (kill(pid, SIGSTOP) == -1) {
        logger.Error("Failed to send SIGSTOP to process {}.", pid);
        return false;
    }
    logger.Debug("Success to send SIGSTOP to process {}.", pid);
    usleep(1000 * 10); // sleep 10ms, wait for process status
    auto isStopped = IsProcessStopped(pid);
    if (!isStopped.has_value()) {
        logger.Error("Failed to get state of process {}.", pid);
        return false;
    }
    if (*isStopped) {
        logger.Debug("Process {} froze successfully.", pid);
        return true;
    } else {
        logger.Error("Failed to freeze process {}.", pid);
        return false;
    }
}

bool TryToResumeProsessByPid(pid_t pid, int attempts) {
    if (attempts < 1) {
        logger.Error("Failed to resume prosess {}: attempts={} less than one.", pid, attempts);
        return false;
    }
    for (int i = 0, j = 0; i < attempts; ++i) {
        if (kill(pid, SIGCONT) == -1) {
            logger.Error("Failed to send SIGCONT to process {} for {} time.", pid, ++j);
            continue;
        }
        usleep(1000); // sleep 1ms, wait for process status
        if (auto isStop = IsProcessStopped(pid); isStop.has_value() && !*isStop) {
            logger.Debug("Process {} resumed successfully.", pid);
            return true;
        }
        usleep(1000 * 100); // sleep 100ms, wait for another try
    }
    logger.Error("Failed to resume process {} after {} attempts.", pid, attempts);
    return false;
}
