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

#include "ame_system.h"
#include "ame_logger.h"

#include <dirent.h>
#include <unistd.h>

#include <csignal>
#include <cstdlib>
#include <cstring>

#include <format>
#include <fstream>
#include <optional>
#include <string>

std::optional<pid_t> FindPidByPackageName(std::string_view packageName) {
    DIR *dirp = opendir("/proc");
    if (dirp == nullptr) {
        logger.Error("Pid not find: {}:", strerror(errno));
        return std::nullopt;
    }
    dirent *dp = nullptr;
    std::optional<pid_t> result = std::nullopt;
    while ((dp = readdir(dirp)) != nullptr) {
        if (dp->d_type != DT_DIR) {
            // not a directory
            continue;
        }
        std::string pidStr = dp->d_name;
        if (pidStr.find_first_not_of("0123456789") != std::string::npos) {
            continue;
        }
        std::string cmdlinePath = std::format("/proc/{}/cmdline", pidStr);
        std::fstream cmdlineFile(cmdlinePath, std::ios::in);
        if (!cmdlineFile.is_open()) {
            logger.Debug("Failed to open: {}:", cmdlinePath);
            continue;
        }
        std::string cmdline;
        std::getline(cmdlineFile, cmdline);
        cmdlineFile.close();
        if (cmdline.find(packageName) != std::string::npos) {
            result = std::stoi(pidStr);
            break;
        }
    }
    closedir(dirp);
    return result;
}

bool IsProcessStopped(pid_t pid) {
    return true;
}

bool FreezeProcess(pid_t pid) {
    if (kill(pid, SIGSTOP) == -1) {
        logger.Error("Failed to send SIGSTOP to process {}", pid);
        return false;
    }
    logger.Info("Success to send SIGSTOP to process {}", pid);
    /* usleep(1000); // sleep 1ms, wait for process status
    if (!IsProcessStopped(pid)) {
        return false;
    } */
    logger.Info("Process {} freeze success.", pid);
    return true;
}

bool TryToResumeProsess(pid_t pid, int attempts) {
    for (int i = 0; i < attempts; ++i) {
        if (kill(pid, SIGCONT) == -1) {
            continue;
        }
        usleep(1000); // sleep 1ms, wait for process status
        /* if (IsProcessStopped(pid)) {
            usleep(1000 * 100); // sleep 100ms, wait for another try
            continue;
        } */
        logger.Info("Process {} resumed successfully.", pid);
        return true;
    }
    logger.Error("Failed to resume process {} after {} attempts.", pid, attempts);
    return false;
}
