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

#include "ame_memory.h"
#include "ame_process.h"

#include <format>
#include <iostream>

int main() {
    using namespace ame;

    // Init
    std::string packageName{"com.popcap.pvz_na"};
    std::cout << "Package name: " << packageName << std::endl;
    if (auto pidOpt = FindPidByProcessName(packageName); pidOpt.has_value()) {
        std::cout << std::format("PID of {}: {}", packageName, *pidOpt) << std::endl;
    } else {
        std::cout << std::format("PID of {} not find.", packageName) << std::endl;
    }

    // Test
    std::string option;
    std::cout << "[1] Reset Package Name" << std::endl;
    std::cout << "[2] Find PID" << std::endl;
    std::cout << "[3] Freeze Process" << std::endl;
    std::cout << "[4] Resume Process" << std::endl;
    std::cout << "[5] Exit" << std::endl;
    while (true) {
        std::cout << "Enter an option in [1, 2, 3, 4, 5]" << std::endl;
        std::cin >> option;

        if (option == "1") {
            std::cout << "Enter new package name:" << std::endl;
            std::cin >> packageName;
            std::cout << "New package name: " << packageName << std::endl;

        } else if (option == "2") {
            if (auto pidOpt = FindPidByProcessName(packageName); pidOpt.has_value()) {
                std::cout << std::format("PID of {}: {}", packageName, *pidOpt) << std::endl;
            } else {
                std::cout << std::format("PID of {} not find.", packageName) << std::endl;
            }

        } else if (option == "3") {
            FreezeProcessByName(packageName);

        } else if (option == "4") {
            ResumeProcessByName(packageName);

        } else if (option == "5") {
            break;

        } else {
            std::cout << "Wrong option!" << std::endl;
        }
    }

    return 0;
}
