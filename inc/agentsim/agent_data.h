#ifndef AICS_AGENT_DATA_H
#define AICS_AGENT_DATA_H

namespace aics {
namespace agentsim {

struct AgentData
{
	float type = 0;
	float x = 0, y = 0, z = 0;
	float xrot = 0, yrot = 0, zrot = 0;
	float collision_radius = 0;
	std::vector<float> subpoints;
	float vis_type = 0;
};

}
}

#endif // AICS_AGENT_DATA_H