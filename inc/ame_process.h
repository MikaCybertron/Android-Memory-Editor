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

#ifndef __AME_PROCESS_H__
#define __AME_PROCESS_H__

#include <sys/types.h>

#include <optional>
#include <string_view>

[[nodiscard]] std::optional<pid_t> FindPidByPackageName(std::string_view packageName);

[[nodiscard]] std::optional<bool> IsProcessStopped(pid_t pid);

bool FreezeProcessByPid(pid_t pid);

bool ResumeProsessByPid(pid_t pid, int attempts = 3);

int FreezeProcessByPackageName(std::string_view packageName);

int ResumeProsessByPackageName(std::string_view packageName, int attempts = 3);

#endif // __AME_PROCESS_H__
