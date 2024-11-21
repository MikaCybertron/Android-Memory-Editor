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

#ifndef __INC_AME_SYSTEM_H__
#define __INC_AME_SYSTEM_H__

#include <sys/types.h>

#include <optional>
#include <string_view>

[[nodiscard]] std::optional<pid_t> FindPidByPackageName(std::string_view packageName);

bool StopProcess(pid_t pid);

bool TryToResumeProsess(pid_t pid, int attempts);

#endif // __INC_AME_SYSTEM_H__
