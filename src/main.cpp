// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Dispatcher.h"
#include "Historican.h"

#include <sc2api/sc2_coordinator.h>
#include <sc2api/sc2_gametypes.h>
#include <sc2utils/sc2_manage_process.h>

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Provide either name of the map file or path to it!" << std::endl;
        return -1;
    }

#ifdef DEBUG
    gHistory.Init("bin/history.log");
#endif

    sc2::Coordinator coordinator;
    coordinator.LoadSettings(1, argv);

    Dispatcher bot;
    coordinator.SetParticipants({
        CreateParticipant(sc2::Race::Terran, &bot),
        CreateComputer(sc2::Race::Random, sc2::Difficulty::CheatInsane)
    });

    coordinator.LaunchStarcraft();
    coordinator.StartGame(argv[1]);

    while (coordinator.Update()) {
        #ifdef DEBUG
            // Slow down game speed for better look & feel while making experiments.
            sc2::SleepFor(15);
        #endif
    }

    return 0;
}
