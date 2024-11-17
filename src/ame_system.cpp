/*
 * Copyright (C) 2024  Dicot0721
 *
 * This file is part of Android-Memory-Editor.
 *
 * Android-Memory-Editor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Android-Memory-Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Android-Memory-Editor.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ame_system.h"
#include "ame_logger.h"

#include <dirent.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>

#include <format>
#include <fstream>
#include <optional>

std::optional<pid_t> GetPidByPackageName(const char *packageName) {
    DIR *dp = opendir("/proc");
    if (dp == nullptr) {
        ShowError("Error Get pid: failed to opendir");
        return std::nullopt;
    }
    std::optional<pid_t> result = std::nullopt;
    dirent *filename = nullptr;
    while ((filename = readdir(dp)) != nullptr) {
        if (strcmp(filename->d_name, ".") == 0 || strcmp(filename->d_name, "..") == 0) {
            continue;
        }
        if (filename->d_type != DT_DIR) {
            continue;
        }
        std::string filepath = std::format("/proc/{}/cmdline", filename->d_name);
        std::fstream file(filepath, std::ios::in);
        if (!file.is_open()) {
            // ShowError("Error Get pid By Package Name.");
            continue;
        }
        std::string text;
        file >> text;
        file.close();
        if (text == packageName) {
            result = atoi(filename->d_name);
            break;
        }
    }
    closedir(dp);
    return result;
}

void StopProcess(pid_t pid) {
    if (getuid() == 0) {
        return; // Do nothing if root.
    }
    char command[64] = {0};
    snprintf(command, sizeof(command), "kill -STOP %d", pid);
    system(command);
    exit(1);
}

void ResumeProsess(pid_t pid) {
    if (getuid() == 0) {
        return; // Do nothing if root.
    }
    char command[64] = {0};
    snprintf(command, sizeof(command), "kill -CONT %d", pid);
    system(command);
    exit(1);
}
