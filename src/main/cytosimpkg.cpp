#include "simul_prop.h"
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

#include "agentsim/simpkg/cytosimpkg.h"
#include "agentsim/agents/agent.h"
#include "loguru/loguru.hpp"

using std::endl;

void killed_handler(int sig)
{
    LOG_F(WARNING, "Cytosim Killed");
    exit(sig);
}

std::string input_file = "./dep/cytosim/cym/aster.cym";
std::string output_file = "./objects.cmo";

/**
*	Simulation API
**/
namespace aics {
namespace agentsim {

    void CytosimPkg::Setup()
    {
        if(!this->m_reader.get()) {
            this->m_reader.reset(new FrameReader());
        }

        //Cytosim::all_silent();
    }
    void CytosimPkg::Shutdown()
    {
        if(this->m_reader.get() && this->m_reader->good()) {
            this->m_reader->clear();
        }

        this->m_hasFinishedStreaming = false;
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

    void CytosimPkg::InitReactions(Model& model) { }
    void CytosimPkg::RunTimeStep(
        float timeStep, std::vector<std::shared_ptr<Agent>>& agents)
    {

    }

    void CytosimPkg::UpdateParameter(std::string paramName, float paramValue) { }
    void CytosimPkg::Run(float timeStep, std::size_t nTimeSteps)
    {
        Simul simul;
        simul.prop->trajectory_file = "./cpkg_objects.cmo";
        simul.prop->property_file = "./cpkg_properties.cmo";
        simul.prop->config_file = "./dep/cytosim/cym/aster.cym";
        if (Parser(simul, 1, 1, 1, 0, 0).readConfig()) {
                std::cerr << "You must specify a config file\n";
        }

        simul.prepare();

        unsigned     nb_steps = nTimeSteps;
        unsigned     nb_frames  = 0;
        int          solve      = 1;
        bool         prune      = true;
        bool         binary     = true;

        unsigned sss = 0;

        unsigned int  frame = 1;
        real          delta = nb_steps;
        unsigned long check = nb_steps;

        void (Simul::* solveFunc)() = &Simul::solve_not;
        switch ( solve )
        {
            case 1: solveFunc = &Simul::solve;      break;
            case 2: solveFunc = &Simul::solve_auto; break;
            case 3: solveFunc = &Simul::solveX;     break;
        }

        simul.writeProperties(nullptr, prune);
        if ( simul.prop->clear_trajectory )
        {
            simul.writeObjects(simul.prop->trajectory_file, false, binary);
            simul.prop->clear_trajectory = false;
        }
        delta = real(nb_steps) / real(nb_frames);
        check = delta;

        simul.prop->time_step = real(timeStep);
        while ( 1 )
        {
            if ( sss >= check )
            {
                simul.relax();
                simul.writeObjects(simul.prop->trajectory_file, true, binary);
                simul.unrelax();
                if ( sss >= nb_steps )
                    break;
                check = ( ++frame * delta );
            }

            //hold();
            fprintf(stderr, "> step %6i\n", sss);
            (simul.*solveFunc)();
            simul.step();
            ++sss;
        }

        simul.erase();
        simul.prop->clear();
    }

    bool CytosimPkg::IsFinished() { return this->m_hasFinishedStreaming; }

    void CytosimPkg::GetNextFrame(std::vector<std::shared_ptr<Agent>>& agents)
    {
        if(this->m_hasFinishedStreaming)
            return;

        Simul simul;
        simul.prop->property_file = "./cpkg_properties.cmo";
        simul.loadProperties();

        if(!this->m_reader->hasFile()) {
            try {
                this->m_reader->openFile("./cpkg_objects.cmo");
            } catch (Exception& e) {
                std::cerr << "Aborted: " << e.what() << std::endl;
                simul.erase();
                simul.prop->clear();
                return;
            }
        }

        if(this->m_reader->eof()) {
            LOG_F(INFO, "Finished processing Cytosim Trajectory");
            this->m_hasFinishedStreaming = true;
        }

        if(this->m_reader->good()) {
            int errCode = this->m_reader->loadNextFrame(simul);
            std::size_t currentFrame = this->m_reader->currentFrame();
            if(errCode != 0) {
                LOG_F(ERROR, "Error loading Cytosim Reader: err-no %i", errCode);
                this->m_hasFinishedStreaming = true;
                simul.erase();
                simul.prop->clear();
                return;
            } else {
                LOG_F(INFO, "Copying fibers from frame %i", static_cast<int>(currentFrame));
                this->CopyFibers(
                    agents,
                    this->m_reader.get(),
                    &simul
                );
            }
        } else {
            LOG_F(ERROR, "File could not be opened");
            this->m_hasFinishedStreaming = true;
            simul.erase();
            simul.prop->clear();
            return;
        }

        simul.erase();
        simul.prop->clear();
    }

    void CytosimPkg::CopyFibers(
        std::vector<std::shared_ptr<Agent>>& agents,
        FrameReader* reader,
        Simul* simul
    )
    {
        //LOG_F(INFO, "Cytosim Inventory Report below...");
        //simul->reportInventory(std::cout);

        int fiberIndex = 0;
        for (Fiber * fib = simul->fibers.first(); fib; fib = fib->next(), fiberIndex++)
        {
            if (fiberIndex >= agents.size()) {
                LOG_F(ERROR, "Not enough agents to represent fibers");
                return;
            }

            auto agent = agents[fiberIndex];
            agents[fiberIndex]->SetLocation(0,0,0);

            agents[fiberIndex]->SetVisType(vis_type_fiber);
            agents[fiberIndex]->SetVisibility(true);
            agents[fiberIndex]->SetCollisionRadius(0.5);
            for(std::size_t p=0; p < fib->nbPoints(); ++p) {
                Vector cytosimPos = fib->posPoint(p);
                float x = cytosimPos[0];
                float y = cytosimPos[1];
                float z = cytosimPos[2];
                agents[fiberIndex]->UpdateSubPoint(p,x,y,z);
            }
        }
    }

} // namespace agentsim
} // namespace aics
