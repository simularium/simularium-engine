#include "mockpkg.cc"
#include "simularium/simularium.h"
#include "gtest/gtest.h"
#include <cstdio>

namespace aics {
namespace simularium {
    namespace test {
        class SimulationTimeTests : public ::testing::Test {
        };

        TEST_F(SimulationTimeTests, FrameTimeCalculation)
        {
            std::string simFileName = "testSim"; // not a real simulation
            std::string simFilePath = "trajectory/" + simFileName;

            std::ofstream outfile(simFilePath.c_str());
            outfile << "TRAJECTORY MOCK FILE";
            outfile.close();

            TrajectoryFileProperties tfp;
            tfp.timeStepSize = 1.0;
            tfp.numberOfFrames = 100;
            std::shared_ptr<SimPkg> mockPkg(new MockSimPkg(tfp));

            std::vector<std::shared_ptr<SimPkg>> simulators;
            std::vector<std::shared_ptr<Agent>> agents;
            simulators.push_back(mockPkg);

            Simulation simulation(simulators, agents);
            simulation.SetPlaybackMode(SimulationMode::id_traj_file_playback);
            simulation.LoadTrajectoryFile(simFileName);

            std::size_t frameNum = 0;

            // A time in the middle
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 50);
            EXPECT_EQ(frameNum, 50);

            // A time at the beginning
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 0);
            EXPECT_EQ(frameNum, 0);

            // A time at the end
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 100);
            EXPECT_EQ(frameNum, 99);

            // A time past the end
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 5000);
            EXPECT_EQ(frameNum, 99);

            // A time before the beginning
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, -1);
            EXPECT_EQ(frameNum, 0);

            // A non integer time request, round up
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 4.6);
            EXPECT_EQ(frameNum, 5);

            // A non integer time request, round down
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 4.4);
            EXPECT_EQ(frameNum, 4);

            // A non integer time request, round up w/ epsilon
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 4.5);
            EXPECT_EQ(frameNum, 5);

            // Beginning plus epsilon, floating point
            float epsilon = 1e-15;
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 0. + epsilon);
            EXPECT_EQ(frameNum, 0);

            // Beginning minus epsilon, floating point
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 0. - epsilon);
            EXPECT_EQ(frameNum, 0);

            // End plus epsilon, floating point
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 99. + epsilon);
            EXPECT_EQ(frameNum, 99);

            // End minus epsilon, floating point
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 99. - epsilon);
            EXPECT_EQ(frameNum, 99);

            std::remove(simFilePath.c_str());
        }

        TEST_F(SimulationTimeTests, LiveModeScrubbing)
        {
            TrajectoryFileProperties tfp;
            tfp.timeStepSize = 1.0;
            tfp.numberOfFrames = 100;
            std::shared_ptr<SimPkg> mockPkg(new MockSimPkg(tfp));

            std::vector<std::shared_ptr<SimPkg>> simulators;
            std::vector<std::shared_ptr<Agent>> agents;
            simulators.push_back(mockPkg);

            std::size_t frameNum = 0;
            std::string simFileName = "testSim";
            std::size_t numFrames = 100;

            Simulation simulation(simulators, agents);
            simulation.SetPlaybackMode(SimulationMode::id_live_simulation);
            simulation.SetSimId(simFileName);

            for (std::size_t i = 0; i < numFrames; ++i) {
                simulation.RunTimeStep(1.0);
            }

            // A time in the middle
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, numFrames / 2);
            EXPECT_EQ(frameNum, numFrames / 2);

            // A time at the beginning
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, 0);
            EXPECT_EQ(frameNum, 0);

            // A time at the end
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, numFrames);
            EXPECT_EQ(frameNum, numFrames - 1);

            // A time past the end
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, numFrames * 10);
            EXPECT_EQ(frameNum, numFrames - 1);

            // A time before the beginning
            frameNum = simulation.GetClosestFrameNumberForTime(simFileName, -1);
            EXPECT_EQ(frameNum, 0);
        }

    } // namespace test
} // namespace simularium
} // namespace aics
