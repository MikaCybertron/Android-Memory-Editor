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

#include "ame_process.h"
#include "ame_logger.h"

#include <dirent.h>
#include <sys/types.h>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>

#include <format>
#include <fstream>
#include <functional>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <thread>

/**
 * @brief Find the PID of a process by its name.
 * @param [in] processName  Process name, which could be package name for Android app.
 * @return The std::optional with value of the PID, or std::nullopt if it could not be found.
 */
std::optional<pid_t> FindPidByProcessName(std::string_view processName) {
    DIR *procDir = opendir("/proc");
    if (procDir == nullptr) {
        logger.Error("Failed to open [/proc]: {}.", std::strerror(errno));
        return std::nullopt;
    }

    std::string cmdline;
    std::optional<pid_t> result;
    for (dirent *entry = nullptr; (entry = readdir(procDir)) != nullptr;) {
        if (entry->d_type != DT_DIR) {
            continue; // not a directory
        }
        std::string_view dirname(entry->d_name);
        if (dirname.find_first_not_of("0123456789") != std::string_view::npos) {
            continue; // not numeric name
        }

        std::string cmdlinePath = std::format("/proc/{}/cmdline", dirname);
        std::ifstream cmdlineFile(cmdlinePath);
        if (!cmdlineFile.is_open()) {
            logger.Error("Failed to open [{}].", cmdlinePath);
            continue;
        }
        std::getline(cmdlineFile, cmdline, '\0');
        if (cmdline == processName) {
            result = std::atoi(entry->d_name);
            break;
        }
    }
    closedir(procDir);
    return result;
}


std::optional<bool> IsProcessStopped(pid_t pid) {
    std::string statusPath = std::format("/proc/{}/status", pid);
    std::ifstream statusFile(statusPath);
    if (!statusFile.is_open()) {
        logger.Error("Failed to open [{}].", statusPath);
        return std::nullopt;
    }

    std::smatch matches;
    std::regex stateRegex(R"re(^State:\s+(\S+)\s+\((.+)\)$)re");
    for (std::string line; std::getline(statusFile, line);) {
        if (!std::regex_match(line, matches, stateRegex)) {
            continue;
        }
        std::string stateCode = matches.str(1);
        if (stateCode == "T") {
            logger.Debug("Process {} is in stopped state.", pid);
            return true;
        } else {
            std::string state = matches.str(2);
            logger.Debug("Process {} is not in stopped state: {} ({}).", pid, stateCode, state);
            return false;
        }
    }
    return std::nullopt;
}


bool FreezeProcessByPid(pid_t pid) {
    using namespace std::chrono_literals;

    if (kill(pid, SIGSTOP) == -1) {
        logger.Error("Failed to send SIGSTOP to process {}: {}.", pid, std::strerror(errno));
        return false;
    }
    logger.Debug("Succeeded in sending SIGSTOP to process {}.", pid);

    std::this_thread::sleep_for(10ms); // wait for process status
    auto isStopped = IsProcessStopped(pid);
    if (!isStopped.has_value()) {
        logger.Error("Failed to get state of process {}.", pid);
        return false;
    }
    if (!*isStopped) {
        logger.Error("Failed to freeze process {}.", pid);
        return false;
    }
    logger.Info("Succeeded in freezing process {}.", pid);
    return true;
}


bool ResumeProcessByPid(pid_t pid) {
    using namespace std::chrono_literals;

    if (kill(pid, SIGCONT) == -1) {
        logger.Error("Failed to send SIGCONT to process {}: {}.", pid, std::strerror(errno));
        return false;
    }
    logger.Debug("Succeeded in sending SIGCONT to process {}.", pid);

    std::this_thread::sleep_for(10ms); // wait for process status
    auto isStopped = IsProcessStopped(pid);
    if (!isStopped.has_value()) {
        logger.Error("Failed to get state of process {}.", pid);
        return false;
    }
    if (*isStopped) {
        logger.Error("Failed to resume process {}.", pid);
        return false;
    }
    logger.Info("Succeeded in resuming process {}.", pid);
    return true;
}


/**
 * @retval 0  The operation succeeded.
 * @retval -1  Could not find the process.
 * @retval -2  The operation failed.
 */
int DoWithProcessName(std::string_view processName, std::function<bool(pid_t)> operation) {
    auto pidOpt = FindPidByProcessName(processName);
    if (!pidOpt.has_value()) {
        logger.Error("Cannot find process [{}].", processName);
        return -1;
    }
    return operation(*pidOpt) ? 0 : -2;
}

int FreezeProcessByName(std::string_view processName) {
    return DoWithProcessName(processName, FreezeProcessByPid);
}

int ResumeProcessByName(std::string_view processName) {
    return DoWithProcessName(processName, ResumeProcessByPid);
}
