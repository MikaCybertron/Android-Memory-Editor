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

#include "ame_logger.h"
#include "ame_memory.h"
#include "ame_system.h"

int main() {
    auto pid = GetPidByPackageName("com.popcap.pvz_na");
    ShowInfo("test: {}", pid.has_value());
    return 0;
}
