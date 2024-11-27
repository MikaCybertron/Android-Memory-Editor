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
    std::string packageName = "com.popcap.pvz_na";
    auto pidOp = FindPidByPackageName(packageName);
    if (!pidOp.has_value()) {
        logger.Info("pid of {} not find.", packageName);
        return 0;
    }
    pid_t pid = *pidOp;
    logger.Info("pid of {}: {}", packageName, pid);

    // Test
    logger.Info("[1] Freeze Process");
    logger.Info("[2] Resume Prosess");
    logger.Info("[3] Exit");
    std::string input;
    while (true) {
        logger.Info("Enter an option in [1, 2, 3]");
        std::cin >> input;
        if (input == "1") {
            FreezeProcessByPid(pid);
        } else if (input == "2") {
            TryToResumeProsessByPid(pid);
        } else if (input == "3") {
            break;
        } else {
            logger.Info("Wrong option!");
        }
    }

    // FreezeProcessByPid(pid);
    // auto listOp = FindArrayAddress<int32_t>(pid,
    //                                         MemoryZone::A_ANONMYOURS,
    //                                         // the density of fog in each cell
    //                                         {
    //                                             200, 200, 200, 200, 200, 200, 200, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    //                                         });
    // if (listOp.has_value() && !listOp->empty()) {
    //     for (const auto &address : *listOp) {
    //         logger.Info("Fog address: 0x{:X}", address);
    //     }
    // } else {
    //     logger.Warning("Fog address not find.");
    // }
    // TryToResumeProsessByPid(pid);

    return 0;
}
