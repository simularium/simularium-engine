#include "simul_prop.h"

#include "agentsim/simpkg/cytosimpkg.h"
#include "agentsim/agents/agent.h"
#include "loguru/loguru.hpp"

#include "exceptions.h"
#include "frame_reader.h"
#include "glossary.h"
#include "messages.h"
#include "parser.h"
#include "simul.h"
#include "stream_func.h"
#include "tictoc.h"
#include <csignal>
#include <stdlib.h>

using std::endl;

void killed_handler(int sig)
{
    LOG_F(WARNING, "Cytosim Killed");
    exit(sig);
}

Simul simul;
Glossary glos;
FrameReader reader;

std::string input_file = "./dep/cytosim/cym/aster.cym";
std::string output_file = "./objects.cmo";

void GetFiberPositionsFromFrame(
    std::vector<std::shared_ptr<aics::agentsim::Agent>>& agents);

/**
*	Simulation API
**/
namespace aics {
namespace agentsim {

    void CytosimPkg::Setup()
    {
        if (this->m_hasAlreadySetup)
            return;

        LOG_F(INFO,"Cytosim PKG Setup started");

        // Register a function to be called for Floating point exceptions:
        if (signal(SIGINT, killed_handler) == SIG_ERR)
            LOG_F(ERROR,"Could not register SIGINT handler");

        if (signal(SIGTERM, killed_handler) == SIG_ERR)
            LOG_F(ERROR,"Could not register SIGTERM handler");

        glos.clear();
        glos.define("config", input_file);

        try {
            simul.initialize(glos);
            Parser(simul, 1, 1, 1, 0, 0).readConfig(input_file);
        } catch (Exception& e) {
            LOG_F(FATAL,e.what().c_str());
            return;
        } catch (...) {
            LOG_F(FATAL,"Unkown exception occured during Cytosim PKG Initialization");
            return;
        }

        glos.warnings(std::cerr);

        this->m_hasAlreadySetup = true;
        LOG_F(INFO,"Cytosim PKG Setup ended");
    }

    void CytosimPkg::Shutdown()
    {
        this->m_hasAlreadyRun = false;
        this->m_hasAlreadySetup = false;
        this->m_hasFinishedStreaming = false;
        this->m_hasLoadedFrameReader = false;

        simul.erase();
        glos.clear();
        reader.clearPositions();
    }

    void CytosimPkg::InitAgents(std::vector<std::shared_ptr<Agent>>& agents, Model& model)
    {
        for (std::size_t i = 0; i < 1000; ++i) {
            std::shared_ptr<Agent> agent;
            agent.reset(new Agent());
            agent->SetVisibility(false);
            agents.push_back(agent);
        }
    }

    void CytosimPkg::InitReactions(Model& model)
    {
    }

    void CytosimPkg::RunTimeStep(
        float timeStep, std::vector<std::shared_ptr<Agent>>& agents)
    {
        float max_time_step = 0.1f;

        if (timeStep > max_time_step) {
            unsigned n_iterations = timeStep / max_time_step;

            simul.prop->time_step = max_time_step;
            Parser(simul, 0, 0, 0, 1, 0).execute_run(n_iterations, glos);
        } else {
            simul.prop->time_step = timeStep;
            Parser(simul, 0, 0, 0, 1, 0).execute_run(1, glos);
        }

        GetFiberPositionsFromFrame(agents);
    }

    void CytosimPkg::UpdateParameter(std::string paramName, float paramValue)
    {
    }

    void CytosimPkg::Run(float timeStep, std::size_t nTimeStep)
    {
        if (this->m_hasAlreadyRun)
            return;

        LOG_F(INFO,"Cytosim PKG Run Started");
        try {
            Parser(simul, 0, 0, 0, 1, 1).readConfig(input_file);
        } catch (Exception& e) {
            LOG_F(FATAL,e.what().c_str());
            return;
        } catch (...) {
            LOG_F(FATAL,"Unkown exception occured during Cytosim PKG Initialization");
            return;
        }

        LOG_F(INFO,"Cytosim PKG Run ended");
        this->m_hasAlreadyRun = true;
    }

    bool CytosimPkg::IsFinished()
    {
        return this->m_hasAlreadyRun && this->m_hasFinishedStreaming;
    }

    void CytosimPkg::GetNextFrame(std::vector<std::shared_ptr<Agent>>& agents)
    {
        if (this->m_hasFinishedStreaming)
            return;

        if (!this->m_hasLoadedFrameReader) {
            try {
                reader.openFile(output_file);
            } catch (Exception& e) {
                std::cerr << "Aborted: " << e.what() << std::endl;
                return;
            }

            if (reader.good()) {
                this->m_hasLoadedFrameReader = true;
            } else {
                printf("File could not be opened\n");
                return;
            }
        }

        if (0 == reader.loadNextFrame(simul)) {
            GetFiberPositionsFromFrame(agents);
        } else {
            LOG_F(INFO,"Finished Streaming Cytosim Run from File");
            this->m_hasFinishedStreaming = true;
        }
    }

} // namespace agentsim
} // namespace aics

void GetFiberPositionsFromFrame(
    std::vector<std::shared_ptr<aics::agentsim::Agent>>& agents)
{
    Fiber* fib = static_cast<Fiber*>(simul.fibers.inventory.first());

    std::size_t agent_index = 0;
    std::size_t fib_index = 0;
    while (fib) {
        if (agent_index >= agents.size()) {
            std::cerr << std::endl
                      << "Error: Not enough agents to represent fibers" << std::endl;
            return;
        }

        std::vector<Eigen::Vector3d> fiber_positions;
        for (std::size_t p = 0; p < fib->nbPoints(); ++p) {
            Vector cytosim_pos = fib->posPoint(p);
            Eigen::Vector3d agentviz_pos(
                cytosim_pos[0],
                cytosim_pos[1],
                cytosim_pos[2]);

            fiber_positions.push_back(agentviz_pos);
        }

        agents[agent_index]->SetName("Fiber");
        agents[agent_index]->SetTypeID(0);
        agents[agent_index]->SetVisType(kVisType::vis_type_fiber);
        agents[agent_index]->SetCollisionRadius(0.3);
        agents[agent_index]->SetVisibility(true);
        agents[agent_index]->SetLocation(fiber_positions[fiber_positions.size() - 1]);

        for (std::size_t i = 0; i < fiber_positions.size(); ++i) {
            agents[agent_index]->UpdateSubPoint(i, fiber_positions[i]);
        }

        agent_index++;
        fib_index++;
        fib = static_cast<Fiber*>(simul.fibers.inventory.next(fib));
    }
}
