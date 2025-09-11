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

#include <iostream>
#include <print>

int main(int argc, char* argv[]) {
    using namespace ame;

    // init
    std::string packageName = (argc > 1) ? argv[1] : "";
    std::println("Package name: '{}'", packageName);
    if (const auto pidOpt = FindPidByProcessName(packageName); pidOpt.has_value()) {
        std::println("PID of '{}': {}", packageName, *pidOpt);
    } else {
        std::println("PID of '{}' not find.", packageName);
    }

    // test
    std::string option;
    std::println("[1] Reset Package Name");
    std::println("[2] Find PID");
    std::println("[3] Freeze Process");
    std::println("[4] Resume Process");
    std::println("[5] Exit");
    while (true) {
        std::println("Enter an option in [1, 2, 3, 4, 5]");
        std::cin >> option;

        if (option == "1") {
            std::println("Enter new package name:");
            std::cin >> packageName;
            std::println("New package name: '{}'", packageName);

        } else if (option == "2") {
            if (auto pidOpt = FindPidByProcessName(packageName); pidOpt.has_value()) {
                std::println("PID of '{}': {}", packageName, *pidOpt);
            } else {
                std::println("PID of '{}' not find.", packageName);
            }

        } else if (option == "3") {
            FreezeProcessByName(packageName);

        } else if (option == "4") {
            ResumeProcessByName(packageName);

        } else if (option == "5") {
            break;

        } else {
            std::println("Wrong option!");
        }
    }

    return 0;
}
