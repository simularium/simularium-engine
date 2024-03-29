#include "test/network/test_net_commands.h"
#include "simularium/network/cli_client.h"
#include "simularium/network/net_message_ids.h"
#include "simularium/simularium.h"
#include <memory>

namespace aics {
namespace simularium {
    namespace test {

        TEST_F(NetCommandTests, PlayPause)
        {
            // Disabling STD OUT until a logging library is setup
            //  otherwise, the cli clients would be noisy for this test
            std::cout.setstate(std::ios_base::failbit);

            // Wait specified amount of time
            // send play/pause/resume command
            std::size_t numCommands = 100;

            std::atomic<bool> isRunning = true;
            float timeStep = 1e-12;
            std::string uri = "wss://localhost:9002";
            std::string simFileName = "actin19.h5";

            std::vector<std::shared_ptr<SimPkg>> simulators;
            std::vector<std::shared_ptr<Agent>> agents;
            Simulation simulation(simulators, agents);

            ConnectionManager connectionManager;
            connectionManager.ListenAsync();
            connectionManager.StartFileIOAsync(isRunning, simulation);
            connectionManager.StartSimAsync(isRunning, simulation, timeStep);

            CliClient controller(uri);
            controller.Parse("start trajectory " + simFileName);

            std::vector<std::string> commands = {
                "pause",
                "resume",
                "stop",
            };

            for (std::size_t i = 0; i < numCommands; i++) {
                controller.Parse(commands.at(rand() % commands.size()));
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

            controller.Parse("quit");
            isRunning = false;
            connectionManager.CloseServer();
            std::cout.clear();
        }

    } // namespace test
} // namespace simularium
} // namespace aics
