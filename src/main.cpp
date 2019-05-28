// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Dispatcher.h"
#include "Historican.h"
#include "core/Converter.h"

#include <sc2api/sc2_coordinator.h>
#include <sc2api/sc2_gametypes.h>
#include <sc2utils/sc2_arg_parser.h>
#include <sc2utils/sc2_manage_process.h>

class Human : public sc2::Agent {
public:
    void OnGameStart() final {
        Debug()->DebugTextOut("Human");
        Debug()->SendDebug();

    }
    void OnStep()
    {
        Control()->GetObservation();
    }
};

#ifdef DEBUG
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Provide either name of the map file or path to it!" << std::endl;
        return -1;
    }

    gHistory.Init("history.log");
    gHistory.SetSeverity(LogSeverity::debug);
    // gHistory.AddFilter(LogChannel::general);
    // gHistory.AddFilter(LogChannel::reasoning);
    // gHistory.AddFilter(LogChannel::scouting);
    // gHistory.AddFilter(LogChannel::economy);
    // gHistory.AddFilter(LogChannel::combat);

    sc2::Coordinator coordinator;
    coordinator.LoadSettings(1, argv);

    Dispatcher mulle_mech("SC2 Default AI");
    Human human;
    coordinator.SetParticipants({
        //CreateParticipant(sc2::Race::Terran, &human), // Uncomment this and comment out CreateComputer to play vs the AI
        CreateParticipant(sc2::Race::Terran, &mulle_mech),
        CreateComputer(sc2::Race::Random, sc2::Difficulty::CheatInsane)
    });

    // Uncomment these lines to play vs the AI in realtime (without realtime it seems buggy)
    // The AI is not really tested when playing in realtime when if stepsize > 1 though
    //coordinator.SetStepSize(1);
    //coordinator.SetRealtime(true);

    coordinator.LaunchStarcraft();
    coordinator.StartGame(argv[1]);

    while (coordinator.Update()) {
        // NOTE (alkurbatov): Slow down game speed for better look & feel
        // while making experiments. Uncomment this if needed.
        // sc2::SleepFor(15);
    }

    return 0;
}
#else

namespace {

struct Options {
    int32_t GamePort = 0;
    int32_t StartPort = 0;
    std::string ServerAddress;
    std::string OpponentId;
    std::string PlayLocalMap;
    bool ComputerOpponent = false;
    bool HumanOpponent = false;
    sc2::Race LocalOpponentRace = sc2::Race::Random;
    sc2::Difficulty ComputerDifficulty = sc2::Difficulty::VeryHard;
    bool RealTime = false;
    int StepSize = 1;
    std::string ExePath;
};

void ParseArguments(int argc, char* argv[], Options* options_) {
    sc2::ArgParser arg_parser(argv[0]);
    arg_parser.AddOptions({
            {"-g", "--GamePort", "Port of client to connect to", false},
            {"-o", "--StartPort", "Starting server port", false},
            {"-l", "--LadderServer", "Ladder server address", false},
            {"-x", "--OpponentId", "PlayerId of opponent", false},
            {"-p", "--PlayLocalMap", "If we play only on this PC using specified map", false}, // NOTE: If this is set ComputerOpponent or HumanOpponent HAS to be set to
            {"-c", "--ComputerOpponent", "If we set up a computer opponent", false},
            {"-h", "--HumanOpponent", "If we set up a human opponent", false}, // This will be ignored if ComputerOpponent is set
            {"-a", "--LocalOpponentRace", "Race of computer/human opponent", false},
            {"-d", "--ComputerDifficulty", "Difficulty of computer opponent", false},
            {"-t", "--RealTime", "Race of human opponent", false},
            {"-s", "--StepSize", "StepSize", false},
            {"-e", "--ExePath", "Path to the 4.8.4 exe", false}
        });

    arg_parser.Parse(argc, argv);

    std::string dummy;

    if (arg_parser.Get("ComputerOpponent", dummy)) {
        options_->ComputerOpponent = true;
        std::string OpponentId = "SC2 Default AI";

        std::string local_opponent_race;
        if (arg_parser.Get("LocalOpponentRace", local_opponent_race))
            options_->LocalOpponentRace = convert::StringToRace(local_opponent_race);

        std::string computer_difficulty;
        if (arg_parser.Get("ComputerDifficulty", computer_difficulty))
            options_->ComputerDifficulty = convert::StringToDifficulty(computer_difficulty);

        arg_parser.Get("PlayLocalMap", options_->PlayLocalMap);

    } else if (arg_parser.Get("HumanOpponent", dummy)) {
        options_->HumanOpponent = true;
        std::string OpponentId = "Human";

        std::string local_opponent_race;
        if (arg_parser.Get("LocalOpponentRace", local_opponent_race))
            options_->LocalOpponentRace = convert::StringToRace(local_opponent_race);

        arg_parser.Get("PlayLocalMap", options_->PlayLocalMap);

    } else {
        std::string game_port_str;
        if (arg_parser.Get("GamePort", game_port_str))
            options_->GamePort = std::stoi(game_port_str);

        std::string start_port_str;
        if (arg_parser.Get("StartPort", start_port_str))
            options_->StartPort = std::stoi(start_port_str);

        std::string opponent_id;
        if (arg_parser.Get("OpponentId", opponent_id))
            options_->OpponentId = opponent_id;

        arg_parser.Get("LadderServer", options_->ServerAddress);
    }

    if (arg_parser.Get("RealTime", dummy))
        options_->RealTime = true;

    std::string step_size;
    if (arg_parser.Get("StepSize", step_size))
        options_->StepSize = std::stoi(step_size);

    arg_parser.Get("ExePath", options_->ExePath);
}

}  // namespace

int main(int argc, char* argv[]) {
    Options options;
    ParseArguments(argc, argv, &options);

    gHistory.Init("history.log");
    gHistory.SetSeverity(LogSeverity::info);

    sc2::Coordinator coordinator;
    Dispatcher mulle_mech(options.OpponentId);
    Human human;

    coordinator.SetRealtime(options.RealTime);
    coordinator.SetStepSize(options.StepSize);

    // 4.8.4 EXE
    if (!options.ExePath.empty()) {
        coordinator.SetDataVersion("CD040C0675FD986ED37A4CA3C88C8EB5");
        coordinator.SetProcessPath(options.ExePath);
    }

    if (!options.PlayLocalMap.empty()) {
        if (options.ComputerOpponent) {
            coordinator.SetParticipants({
                CreateParticipant(sc2::Race::Terran, &mulle_mech),
                CreateComputer(options.LocalOpponentRace, options.ComputerDifficulty)
            });

        } else if (options.HumanOpponent) {
            coordinator.SetParticipants({
                CreateParticipant(options.LocalOpponentRace, &human),
                CreateParticipant(sc2::Race::Terran, &mulle_mech)
            });
        }

        coordinator.LaunchStarcraft();
        coordinator.StartGame(options.PlayLocalMap);

    // Playing on ladder manager
    } else {
        coordinator.SetParticipants({CreateParticipant(sc2::Race::Terran, &mulle_mech)});
        std::cout << "Connecting to port " << options.GamePort << std::endl;
        coordinator.Connect(options.GamePort);
        coordinator.SetupPorts(2, options.StartPort, false);
        coordinator.JoinGame();
        coordinator.SetTimeoutMS(10000);
        std::cout << " Successfully joined game" << std::endl;
    }

    while (coordinator.Update()) {
    }

    return 0;
}

#endif
