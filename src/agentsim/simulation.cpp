#include "agentsim/simulation.h"
#include "agentsim/simpkg/simpkg.h"

namespace aics {
namespace agentsim {

Simulation::Simulation(
	std::vector<std::shared_ptr<SimPkg>> simPkgs,
	std::vector<std::shared_ptr<Agent>> agents)
{
	for(std::size_t i = 0; i < simPkgs.size(); ++i)
	{
			this->m_SimPkgs.push_back(simPkgs[i]);
			this->m_SimPkgs[i]->Setup();
	}

	this->m_agents = agents;
}

Simulation::~Simulation()
{
	for(std::size_t i = 0; i < this->m_SimPkgs.size(); ++i)
	{
			this->m_SimPkgs[i]->Shutdown();
	}
}

void Simulation::RunTimeStep(float timeStep)
{
	for(std::size_t i = 0; i < this->m_SimPkgs.size(); ++i)
	{
			this->m_SimPkgs[i]->RunTimeStep(timeStep, this->m_agents);
	}
}

} // namespace agentsim
} // namespace aics
