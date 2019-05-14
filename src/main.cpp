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

#ifdef DEBUG
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

    Dispatcher mulle_mech("MulleMech");
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
    Options(): GamePort(0), StartPort(0), ComputerOpponent(false) {
    }

    int32_t GamePort;
    int32_t StartPort;
    std::string ServerAddress;
    std::string OpponentId;
    bool ComputerOpponent;
    std::string PlayLocalMap;
    sc2::Difficulty ComputerDifficulty;
    sc2::Race ComputerRace;
};

void ParseArguments(int argc, char* argv[], Options* options_) {
    sc2::ArgParser arg_parser(argv[0]);
    arg_parser.AddOptions({
            {"-g", "--GamePort", "Port of client to connect to", false},
            {"-o", "--StartPort", "Starting server port", false},
            {"-l", "--LadderServer", "Ladder server address", false},
            {"-x", "--OpponentId", "PlayerId of opponent", false},
            {"-p", "--PlayLocalMap", "If we play only on this PC using specified map", false},
            {"-c", "--ComputerOpponent", "If we set up a computer opponent", false},
            {"-a", "--ComputerRace", "Race of computer oppent", false},
            {"-d", "--ComputerDifficulty", "Difficulty of computer opponent", false}
        });

    arg_parser.Parse(argc, argv);

    std::string GamePortStr;
    if (arg_parser.Get("GamePort", GamePortStr))
        options_->GamePort = atoi(GamePortStr.c_str());

    std::string StartPortStr;
    if (arg_parser.Get("StartPort", StartPortStr))
        options_->StartPort = atoi(StartPortStr.c_str());

    std::string OpponentId;
    if (arg_parser.Get("OpponentId", OpponentId))
        options_->OpponentId = OpponentId;

    arg_parser.Get("LadderServer", options_->ServerAddress);

    std::string dummy;
    if (arg_parser.Get("ComputerOpponent", dummy)) {
        options_->ComputerOpponent = true;
        std::string CompRace;

        if (arg_parser.Get("ComputerRace", CompRace))
            options_->ComputerRace = convert::StringToRace(CompRace);

        std::string CompDiff;
        if (arg_parser.Get("ComputerDifficulty", CompDiff))
            options_->ComputerDifficulty = convert::StringToDifficulty(CompDiff);
    } else
        options_->ComputerOpponent = false;

    arg_parser.Get("PlayLocalMap", options_->PlayLocalMap);
}

}  // namespace

int main(int argc, char* argv[]) {
    Options options;
    ParseArguments(argc, argv, &options);

    gHistory.Init("history.log");
    gHistory.SetSeverity(LogSeverity::info);

    sc2::Coordinator coordinator;
    Dispatcher bot(options.OpponentId);

    size_t num_agents;
    if (options.ComputerOpponent) {
        num_agents = 1;
        coordinator.SetParticipants({
            CreateParticipant(sc2::Race::Terran, &bot),
            CreateComputer(options.ComputerRace, options.ComputerDifficulty)
            });
    } else {
        num_agents = 2;
        coordinator.SetParticipants({CreateParticipant(sc2::Race::Terran, &bot)});
    }

    if (options.PlayLocalMap.empty()) {
        std::cout << "Connecting to port " << options.GamePort << std::endl;
        coordinator.Connect(options.GamePort);
        coordinator.SetupPorts(num_agents, options.StartPort, false);
        coordinator.JoinGame();
        coordinator.SetTimeoutMS(10000);
        std::cout << " Successfully joined game" << std::endl;
    } else {
        coordinator.LaunchStarcraft();
        coordinator.StartGame(options.PlayLocalMap);
    }

    while (coordinator.Update()) {
    }

    return 0;
}

#endif
