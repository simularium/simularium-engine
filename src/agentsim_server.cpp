#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#define ASIO_STANDALONE
#include <asio/asio.hpp>
#include <json/json.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "agentsim/agentsim.h"

using namespace aics::agentsim;

// Arg List:
//  --no-exit  don't use the no client timeout
void ParseArguments(
    int argc,
    char* argv[],
    ConnectionManager& connectionManager);

int main(int argc, char* argv[])
{
    ConnectionManager connectionManager;
    ParseArguments(argc, argv, connectionManager);

    // A synchronized variable that tells all the threads to exit
    std::atomic<bool> isServerRunning { true };

    float timeStep = 1e-12; // seconds

    // @TODO: A more robust way to select simulation packages
    bool use_readdy = true;
    bool use_cytosim = !use_readdy;

    // Simulation setup
    std::vector<std::shared_ptr<SimPkg>> simulators;

    if (use_readdy) {
        ReaDDyPkg* readdySimPkg = new ReaDDyPkg();
        std::shared_ptr<SimPkg> readdyPkg;
        readdyPkg.reset(readdySimPkg);
        simulators.push_back(readdyPkg);
    }

    if (use_cytosim) {
        CytosimPkg* cytosimSimPkg = new CytosimPkg();
        std::shared_ptr<SimPkg> cytosimPkg;
        cytosimPkg.reset(cytosimSimPkg);
        simulators.push_back(cytosimPkg);
    }

    std::vector<std::shared_ptr<Agent>> agents;
    Simulation simulation(simulators, agents);

    connectionManager.ListenAsync();
    connectionManager.StartSimAsync(simulation, timeStep);
    connectionManager.StartHeartbeatAsync();

    auto ioThread = std::thread([&] {
        std::string input;
        std::cout << "Enter 'quit' to exit\n";
        while (isServerRunning && std::getline(std::cin, input, '\n')) {
            if (input == "quit") {
                isServerRunning = false;
            } else {
                input = "";
            }
        }
    });

    while (isServerRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Exiting Server...\n";
    connectionManager.CloseServer();

    // The following thread(s) are detached since they block for IO
    //  under the assumption that these threads will be terminated
    //  when the process terminates
    ioThread.detach();
}

void ParseArguments(int argc, char* argv[], ConnectionManager& connectionManager)
{
    // The first argument is the program running
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg.compare("--no-exit") == 0) {
            std::cout << "Argument : --no-exit; ignoring no-client timeout" << std::endl;
            connectionManager.SetNoTimeoutArg(true);
        } else {
            std::cout << "Unrecognized argument " << arg << " ignored" << std::endl;
        }
    }
}
