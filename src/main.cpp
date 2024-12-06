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

#include "ame_logger.h"
#include "ame_memory.h"
#include "ame_process.h"


int main() {
    // Init
    std::string packageName = "com.popcap.pvz_na";
    logger.Info("Package name: {}", packageName);
    if (auto pidOpt = FindPidByPackageName(packageName); pidOpt.has_value()) {
        logger.Info("PID of {}: {}", packageName, *pidOpt);
    } else {
        logger.Info("PID of {} not find.", packageName);
    }

    // Interact (test)
    std::string option;
    logger.Info("[1] Reset Package Name");
    logger.Info("[2] Find PID");
    logger.Info("[3] Freeze Process");
    logger.Info("[4] Resume Prosess");
    logger.Info("[5] Exit");
    while (true) {
        logger.Info("Enter an option in [1, 2, 3, 4, 5]");
        std::cin >> option;

        if (option == "1") {
            logger.Info("Enter new package name:");
            std::cin >> packageName;
            logger.Info("New package name: {}", packageName);

        } else if (option == "2") {
            if (auto pidOpt = FindPidByPackageName(packageName); pidOpt.has_value()) {
                logger.Info("PID of {}: {}", packageName, *pidOpt);
            } else {
                logger.Info("PID of {} not find.", packageName);
            }

        } else if (option == "3") {
            FreezeProcessByPackageName(packageName);

        } else if (option == "4") {
            ResumeProsessByPackageName(packageName);

        } else if (option == "5") {
            break;

        } else {
            logger.Info("Wrong option!");
        }
    }

    return 0;
}
