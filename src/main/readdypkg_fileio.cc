#include "simularium/util/math_util.h"
#define LOGURU_WITH_STREAMS 1
#include "loguru/loguru.hpp"
#include <json/json.h>

#include <csignal>
#include <math.h>

bool verboseOrientation = false;

std::unordered_map<std::string, Eigen::Matrix3d> default_orientation;

OrientationDataMap initOrientationData()
{

    Eigen::Matrix3d zero_rotation;
    zero_rotation << 1, 0, 0,
        0, 1, 0,
        0, 0, 1;

    Eigen::Matrix3d rotation_actin_to_prev_actin;
    rotation_actin_to_prev_actin << 0.58509899, -0.80874088, -0.05997798,
        -0.79675461, -0.55949104, -0.22836785,
        0.15113327, 0.18140554, -0.97172566;

    // TODO test this again once periodic boundary is accounted for in orientation calculation
    Eigen::Matrix3d rotation_barbed_from_actin_dimer_axis;
    rotation_barbed_from_actin_dimer_axis << 0.17508484, -0.52345361, -0.83387146,
        -0.3445482, 0.76082255, -0.54994144,
        0.92229704, 0.38359532, -0.0471465;

    Eigen::Matrix3d rotation_actin_to_next_actin;
    rotation_actin_to_next_actin << 0.5851038, -0.79675251, 0.15112575,
        -0.80873679, -0.55949491, 0.18141181,
        -0.05998622, -0.2283657, -0.97172566;

    // TODO test this again once periodic boundary is accounted for in orientation calculation
    Eigen::Matrix3d rotation_pointed_from_actin_dimer_axis;
    rotation_pointed_from_actin_dimer_axis << 0.17508484, -0.52345361, -0.83387146,
        -0.3445482, 0.76082255, -0.54994144,
        0.92229704, 0.38359532, -0.0471465;

    Eigen::Matrix3d rotation_branch_actin_to_arp3;
    rotation_branch_actin_to_arp3 << 0.19687615, -0.47932213, -0.85527193,
        0.80641304, -0.41697831, 0.41931742,
        -0.55761796, -0.77225604, 0.30443854;

    Eigen::Matrix3d rotation_arp2_from_arp_dimer_axis;
    rotation_arp2_from_arp_dimer_axis << 0.493391, -0.862657, 0.111303,
        -0.797446, -0.499725, -0.338164,
        0.34734, 0.0780887, -0.934482;

    Eigen::Matrix3d rotation_arp3_from_arp_dimer_axis;
    rotation_arp3_from_arp_dimer_axis << -0.599953, 0.795272, 0.0871714,
        0.598794, 0.374107, 0.70816,
        0.530568, 0.47706, -0.70065;

    Eigen::Matrix3d rotation_tub_from_tub_dimer_axis;
    rotation_tub_from_tub_dimer_axis << 1, 0, 0,
        0, 0, -1,
        0, 1, 0;

    default_orientation = {
        { "channel", zero_rotation },
        { "atpase", zero_rotation },
        { "membrane", zero_rotation }
    };

    OrientationDataMap data;
    data["actin"] = {
        { MonomerType("actin", { "any" }, -1),
            OrientationData(
                Eigen::Vector3d(1.453012, -3.27238, -2.330608),
                rotation_actin_to_prev_actin,
                zero_rotation // rotation_barbed_from_actin_dimer_axis
                ) },
        { MonomerType("actin", { "any" }, 1),
            OrientationData(
                Eigen::Vector3d(-3.809657, -1.078586, -1.60457),
                rotation_actin_to_next_actin,
                zero_rotation // rotation_pointed_from_actin_dimer_axis
                ) },
        { MonomerType("arp3", { "any" }, 101),
            OrientationData(
                Eigen::Vector3d(1.345954, -3.27222, 2.238695),
                rotation_branch_actin_to_arp3,
                zero_rotation) }
    };
    data["arp3"] = {
        { MonomerType("actin", { "branch" }, 101),
            OrientationData(
                Eigen::Vector3d(0.08125937, -3.388564, 2.458009),
                zero_rotation,
                zero_rotation) },
        { MonomerType("arp2", { "any" }, 101),
            OrientationData(
                Eigen::Vector3d(2.390059, 1.559484, 3.054445),
                zero_rotation,
                rotation_arp3_from_arp_dimer_axis) }
    };
    data["arp2"] = {
        { MonomerType("arp3", { "any" }, 101),
            OrientationData(
                Eigen::Vector3d(0, 0, 0),
                zero_rotation,
                rotation_arp2_from_arp_dimer_axis) }
    };
    data["cap"] = {
        { MonomerType("actin", { "any" }, 101),
            OrientationData(
                Eigen::Vector3d(0, 0, 0),
                zero_rotation,
                zero_rotation) }
    };
    data["tubulin"] = {
        { MonomerType("site-protofilament1", { "any" }, 101),
            OrientationData(
                Eigen::Vector3d(0, 0, -1),
                zero_rotation,
                zero_rotation) },
        { MonomerType("site-out", { "any" }, 101),
            OrientationData(
                Eigen::Vector3d(0, 1, 0),
                zero_rotation,
                zero_rotation) },
        { MonomerType("tubulin", { "any" }, 101),
            OrientationData(
                Eigen::Vector3d(0, 0, 0),
                zero_rotation,
                rotation_tub_from_tub_dimer_axis) }
    };
    return data;
}

void run_and_save_h5file(
    readdy::Simulation* sim,
    std::string file_name,
    float time_step,
    float n_time_steps,
    float write_stride);

void read_h5file(
    std::string file_name,
    std::shared_ptr<aics::simularium::ReaDDyFileInfo>& readdyFileInfo,
    IdParticleMapping& particleLookup);

bool is_child_particle(
    std::string particleType);

void copy_frame(
    readdy::Simulation* sim,
    const std::vector<ParticleData>& particle_data,
    RotationH5List& rotationInfo,
    std::vector<std::shared_ptr<aics::simularium::Agent>>& agents);

TimeTrajectoryH5Info readTrajectory(
    const std::shared_ptr<h5rd::File>& file,
    h5rd::Group& group,
    std::unordered_map<std::size_t, std::string>& typeMapping,
    IdParticleMapping& particleLookup);

TimeTopologyH5Info readTopologies(
    h5rd::Group& group,
    std::size_t from,
    std::size_t to,
    std::size_t stride);

std::vector<std::size_t> getNeighbors(
    std::size_t particleId,
    const TopologyH5List& topologies);

static MonomerType getMonomerType(
    std::string readdyParticleType);

static int getMonomerN(
    int actualMonomerNumber,
    MonomerType monomerType);

static bool monomerTypeIsSatisfied(
    MonomerType referenceMonomerType,
    int referenceActualMonomerNumber,
    MonomerType testMonomerType,
    int testActualMonomerNumber);

static std::string monomerTypeToString(
    MonomerType monomerType);

static std::vector<std::pair<MonomerType, OrientationData>> getOrientationDataForParticle(
    const OrientationDataMap& orientationDataLookup,
    MonomerType particleMonomerType);

static std::vector<RelativeOrientationData> getNeighborOrientationData(
    std::size_t particleID,
    MonomerType particleMonomerType,
    std::vector<std::pair<MonomerType, OrientationData>> particleOrientationData,
    const ParticleH5List& trajectoryFrame,
    const TopologyH5List& topologyFrame,
    const std::unordered_map<std::size_t, std::size_t>& idmappingFrame);

static Eigen::Vector3d getNeighborNonPeriodicBoundaryPosition(
    Eigen::Vector3d particlePosition,
    Eigen::Vector3d neighborParticlePosition,
    Eigen::Vector3d boxSize);

static Eigen::Matrix3d getCurrentRotation(
    ParticleData neighborParticle0,
    ParticleData currentParticle,
    ParticleData neighborParticle1,
    Eigen::Vector3d boxSize);

static Eigen::Matrix3d getInitialRotation(
    std::vector<RelativeOrientationData> neighborOrientationData);

static Eigen::Matrix3d getRotationUsingAxis(
    ParticleData particle,
    ParticleData neighborParticle,
    Eigen::Matrix3d axisRotation,
    Eigen::Vector3d boxSize);

static void calculateOrientations(
    const TopologyH5Info& topologyH5Info,
    const TrajectoryH5Info& trajectoryH5Info,
    std::shared_ptr<aics::simularium::ReaDDyFileInfo>& readdyFileInfo,
    const IdParticleMapping& particleLookup,
    const OrientationDataMap& orientationDataLookup);

namespace aics {
namespace simularium {

    std::size_t frame_no = 0;
    std::string last_loaded_file = "";
    IdParticleMapping ID_PARTICLE_CACHE;

    void ResetFileIO()
    {
        frame_no = 0;
        last_loaded_file = "";
    }

    void ReaDDyPkg::GetNextFrame(
        std::vector<std::shared_ptr<Agent>>& agents)
    {
        if (this->m_hasFinishedStreaming)
            return;

        if (agents.size() == 0) {
            for (std::size_t i = 0; i < 5000; ++i) {
                std::shared_ptr<Agent> agent;
                agent.reset(new Agent());
                agent->SetVisibility(false);
                agent->SetCollisionRadius(1);
                agents.push_back(agent);
            }
        }

        if (!this->m_hasLoadedRunFile) {
            TrajectoryFileProperties ignore;
            this->LoadTrajectoryFile("/tmp/test.h5", ignore);
        }

        auto& trajectoryInfo = std::get<1>(this->m_fileInfo->trajectoryInfo);

        if (frame_no >= trajectoryInfo.size()) {
            this->m_hasFinishedStreaming = true;
        } else {

            auto rotationList = this->m_fileInfo->rotationInfo.size() > 0 ? this->m_fileInfo->rotationInfo[frame_no] : RotationH5List();

            copy_frame(
                this->m_simulation,
                std::get<1>(this->m_fileInfo->trajectoryInfo).at(frame_no),
                rotationList,
                agents);

            frame_no++;
        }
    }

    bool ReaDDyPkg::IsFinished()
    {
        return this->m_hasFinishedStreaming;
    }

    void ReaDDyPkg::Run(float timeStep, std::size_t nTimeStep)
    {
        if (this->m_hasAlreadyRun)
            return;

        run_and_save_h5file(this->m_simulation, "/tmp/test.h5", timeStep, nTimeStep, 10);
        this->m_hasAlreadyRun = true;
        this->m_hasLoadedRunFile = false;
    }

    void ReaDDyPkg::LoadTrajectoryFile(
        std::string filePath,
        TrajectoryFileProperties& fileProps)
    {
        if (last_loaded_file != filePath) {
            this->m_hasLoadedRunFile = false;
        } else {
            LOG_F(INFO, "Using loaded file: %s", filePath.c_str());
            auto& time = std::get<0>(this->m_fileInfo->trajectoryInfo);
            auto& traj = std::get<1>(this->m_fileInfo->trajectoryInfo);

            fileProps.numberOfFrames = traj.size();
            fileProps.timeStepSize = time.size() >= 2 ? time[1] - time[0] : 0;

            return;
        }

        if (!this->m_hasLoadedRunFile) {
            LOG_F(INFO, "Loading trajectory file %s", filePath.c_str());
            frame_no = 0;

            read_h5file(filePath,
                this->m_fileInfo,
                ID_PARTICLE_CACHE);
            this->m_hasLoadedRunFile = true;
            last_loaded_file = filePath;
            LOG_F(INFO, "Finished loading trajectory file: %s", filePath.c_str());

            auto& time = std::get<0>(this->m_fileInfo->trajectoryInfo);
            auto& traj = std::get<1>(this->m_fileInfo->trajectoryInfo);

            fileProps.numberOfFrames = traj.size();
            fileProps.timeStepSize = time.size() >= 2 ? time[1] - time[0] : 0;

            for (auto entry : this->m_fileInfo->typeMapping) {
                TypeEntry newEntry;
                newEntry.name = entry.second;
                fileProps.typeMapping[entry.first] = newEntry;
            }

            fileProps.boxX = this->m_fileInfo->configInfo.boxX;
            fileProps.boxY = this->m_fileInfo->configInfo.boxY;
            fileProps.boxZ = this->m_fileInfo->configInfo.boxZ;
        }
    }

    double ReaDDyPkg::GetSimulationTimeAtFrame(std::size_t frameNumber)
    {
        auto times = std::get<0>(this->m_fileInfo->trajectoryInfo);
        if (times.size() > frameNumber) {
            return times.at(frameNumber);
        }

        return 0.0;
    }

    std::size_t ReaDDyPkg::GetClosestFrameNumberForTime(double timeNs)
    {
        auto times = std::get<0>(this->m_fileInfo->trajectoryInfo);
        auto lower = std::lower_bound(times.begin(), times.end(), timeNs);
        auto frame = std::distance(times.begin(), lower);

        if (frame > 0) {
            frame = frame - 1;
        }
        return frame;
    }

} // namespace simularium
} // namespace aics

/**
 *	File IO Functions
 **/
void run_and_save_h5file(
    readdy::Simulation* sim,
    std::string file_name,
    float time_step,
    float n_time_steps,
    float write_stride)
{
    auto file = readdy::File::create(file_name, h5rd::File::Flag::OVERWRITE);

    auto traj_ptr = sim->observe().flatTrajectory(1);
    traj_ptr->enableWriteToFile(*file.get(), "", write_stride);
    auto traj_handle = sim->registerObservable(std::move(traj_ptr));

    try {
        auto loop = sim->createLoop(time_step);
        loop.writeConfigToFile(*file);
        loop.run(n_time_steps);
    } catch (...) {
        std::raise(SIGABRT);
        LOG_F(ERROR, "Exception while running ReaDDy simulation");
    }

    file->close();
}

void read_h5file(
    std::string file_name,
    std::shared_ptr<aics::simularium::ReaDDyFileInfo>& readdyFileInfo,
    IdParticleMapping& particleLookup)
{
    TimeTrajectoryH5Info& trajectoryInfo = readdyFileInfo->trajectoryInfo;
    TimeTopologyH5Info& topologyInfo = readdyFileInfo->topologyInfo;
    std::unordered_map<std::size_t, std::string>& typeMapping = readdyFileInfo->typeMapping;

    // open the output file
    auto file = h5rd::File::open(file_name, h5rd::File::Flag::READ_ONLY);

    // read box extents
    try {
        auto config = file->getSubgroup("readdy/config");
        char test[512];
        H5LTread_dataset_string(config.id(), "general", test);

        Json::Value jsonMsg;
        Json::CharReaderBuilder jsonReadBuilder;
        std::unique_ptr<Json::CharReader> const jsonReader(jsonReadBuilder.newCharReader());
        std::string errs;
        std::string message = test;

        jsonReader->parse(message.c_str(), message.c_str() + message.length(), &jsonMsg, &errs);

        const Json::Value& boxSize = jsonMsg["box_size"];
        std::vector<float> boxSizeVec;
        for (Json::Value::const_iterator it = boxSize.begin(); it != boxSize.end(); ++it) {
            boxSizeVec.push_back(it->asFloat());
        }

        if (boxSizeVec.size() >= 3) {
            readdyFileInfo->configInfo.boxX = boxSizeVec[0];
            readdyFileInfo->configInfo.boxY = boxSizeVec[1];
            readdyFileInfo->configInfo.boxZ = boxSizeVec[2];
        }

        readdyFileInfo->configInfo.kbt = jsonMsg["kbt"].asFloat();
        readdyFileInfo->configInfo.boxVolume = jsonMsg["box_volume"].asFloat();
    } catch (...) {
        LOG_F(ERROR, "Config info not found in %s", file_name.c_str());
        return;
    }

    // read back trajectory
    try {
        auto traj = file->getSubgroup("readdy/trajectory");
        trajectoryInfo = readTrajectory(file, traj, typeMapping, particleLookup);
        traj.close();
    } catch (...) {
        LOG_F(ERROR, "No Trajectory information found in %s", file_name.c_str());
        return;
    }

    bool hasTopologies = false;
    try {
        auto topGroup = file->getSubgroup("readdy/observables/topologies");
        topologyInfo = readTopologies(topGroup, 0, std::numeric_limits<std::size_t>::max(), 1);
        topGroup.close();
        hasTopologies = true;
    } catch (...) {
        LOG_F(ERROR, "No Topology observable information found in %s", file_name.c_str());
    }

    if (hasTopologies) {
        auto orientationData = initOrientationData();
        calculateOrientations(
            std::get<1>(topologyInfo),
            std::get<1>(trajectoryInfo),
            readdyFileInfo,
            particleLookup,
            orientationData);
    }

    std::size_t nTrajFrames = std::get<0>(trajectoryInfo).size();
    std::size_t nTopFrames = std::get<0>(topologyInfo).size();

    LOG_F(INFO, "Found trajectory info for %zu frames", nTrajFrames);
    LOG_F(INFO, "Found topology info for %zu frames", nTopFrames);
    file->close();
}

static const std::vector<std::string> childTypes { "_CHILD", "site-" };

bool is_child_particle(
    std::string particleType)
{
    for (std::size_t i = 0; i < childTypes.size(); ++i) {
        if (particleType.find(childTypes.at(i)) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void copy_frame(
    readdy::Simulation* sim,
    const std::vector<ParticleData>& particle_data,
    RotationH5List& rotationInfo,
    std::vector<std::shared_ptr<aics::simularium::Agent>>& agents)
{
    std::size_t agentIndex = 0;
    std::size_t ignore_count = 0;
    std::size_t bad_topology_count = 0;
    std::size_t good_topology_count = 0;
    std::size_t particleCount = particle_data.size();
    bool useRotation = particle_data.size() == rotationInfo.size();

    auto& particles = sim->context().particleTypes();

    for (std::size_t particleIndex = 0; particleIndex < particleCount; ++particleIndex) {
        auto& p = particle_data.at(particleIndex);
        auto pos = p.position;

        // check if this particle has a valid readdy location
        if (isnan(pos[0]) || isnan(pos[1]) || isnan(pos[2])) {
            continue;
        }

        // if this particle is a child particle, don't stream its position
        //  to stream child positions, we would need to correlate the children
        //  the owning parent from the trajectory/H5 file info
        if (is_child_particle(p.type)) {
            continue;
        }

        // check if we have counted past the avaliable agents provided
        //	this would mean we used every agent in the 'agents' already
        if (agentIndex >= agents.size()) {
            LOG_F(WARNING, "%zu+ agents needed vs %zu passed to ReaDDy PKG", agentIndex, agents.size());
            LOG_F(ERROR, "Not enough agents to represent Readdy file run");

            readdy::log::info("\t type {}, id {}, pos ({}, {}, {})",
                p.type, p.id, p.position[0], p.position[1], p.position[2]);
            break;
        }

        // copy the position of the particle to an AgentViz agent
        auto currentAgent = agents[agentIndex].get();
        currentAgent->SetLocation(pos[0], pos[1], pos[2]);
        if (useRotation) {
            currentAgent->SetRotation(
                rotationInfo[particleIndex][0],
                rotationInfo[particleIndex][1],
                rotationInfo[particleIndex][2]);
        }
        currentAgent->SetTypeID(p.type_id);
        currentAgent->SetName(p.type);
        currentAgent->SetVisibility(true);
        currentAgent->SetUid(p.id);
        agentIndex++;
    }

    for (auto i = agentIndex; i < agents.size(); ++i) {
        if (!agents[i]->IsVisible()) {
            break;
        }

        agents[i]->SetVisibility(false);
    }
}

TimeTrajectoryH5Info readTrajectory(
    const std::shared_ptr<h5rd::File>& file,
    h5rd::Group& group,
    std::unordered_map<std::size_t, std::string>& typeMapping,
    IdParticleMapping& particleLookup)
{
    particleLookup.clear();
    TrajectoryH5Info results;

    // retrieve the h5 particle type info type
    auto particleInfoH5Type = readdy::model::ioutils::getParticleTypeInfoType(file->ref());

    // get particle types from config
    std::vector<readdy::model::ioutils::ParticleTypeInfo> types;
    {
        auto config = file->getSubgroup("readdy/config");
        config.read(
            "particle_types",
            types,
            &std::get<0>(particleInfoH5Type),
            &std::get<1>(particleInfoH5Type));
    }

    for (const auto& type : types) {
        typeMapping[type.type_id] = std::string(type.name);
        LOG_F(INFO, "Particle type found: %zu with name %s", type.type_id, type.name);
    }

    // limits of length 2T containing [start_ix, end_ix] for each time step
    std::vector<std::size_t> limits;
    group.read("limits", limits);

    // time of length T containing the time steps of recording the trajectory
    std::vector<readdy::TimeStep> time;
    group.read("time", time);

    // records containing all particles for all times, slicing according to limits
    // yield particles for a particular time step
    std::vector<readdy::model::observables::TrajectoryEntry> entries;
    auto trajectoryEntryTypes = readdy::model::observables::util::getTrajectoryEntryTypes(file->ref());
    group.read(
        "records", entries,
        &std::get<0>(trajectoryEntryTypes),
        &std::get<1>(trajectoryEntryTypes));

    auto n_frames = limits.size() / 2;

    // mapping the read-back data to the ParticleData struct
    results.reserve(n_frames);
    particleLookup.reserve(n_frames);

    auto timeIt = time.begin();
    for (std::size_t frame = 0; frame < limits.size(); frame += 2, ++timeIt) {
        auto begin = limits[frame];
        auto end = limits[frame + 1];

        results.emplace_back();
        auto& currentFrame = results.back();
        currentFrame.reserve(end - begin);

        particleLookup.emplace_back();
        auto& idparticleCache = particleLookup.back();

        for (auto it = entries.begin() + begin; it != entries.begin() + end; ++it) {
            currentFrame.emplace_back(
                typeMapping[it->typeId],
                readdy::model::particleflavor::particleFlavorToString(it->flavor),
                it->pos.data, it->id, it->typeId, *timeIt);

            idparticleCache[currentFrame.back().id] = currentFrame.size() - 1;
        }
    }

    return std::make_tuple(time, results);
}

TimeTopologyH5Info readTopologies(
    h5rd::Group& group,
    std::size_t from,
    std::size_t to,
    std::size_t stride)
{
    readdy::io::BloscFilter bloscFilter;
    bloscFilter.registerFilter();

    std::size_t nFrames { 0 };
    // limits
    std::vector<std::size_t> limitsParticles;
    std::vector<std::size_t> limitsEdges;
    {
        if (stride > 1) {
            group.read("limitsParticles", limitsParticles, { stride, 1 });
        } else {
            group.read("limitsParticles", limitsParticles);
        }
        if (stride > 1) {
            group.read("limitsEdges", limitsEdges, { stride, 1 });
        } else {
            group.read("limitsEdges", limitsEdges);
        }

        if (limitsParticles.size() != limitsEdges.size()) {
            throw std::logic_error(fmt::format("readTopologies: Incompatible limit sizes, "
                                               "limitsParticles.size={}, limitsEdges.size={}",
                limitsParticles.size(), limitsEdges.size()));
        }

        nFrames = limitsParticles.size() / 2;
        from = std::min(nFrames, from);
        to = std::min(nFrames, to);

        if (from == to) {
            throw std::invalid_argument(fmt::format("readTopologies: not enough frames to cover range ({}, {}]",
                from, to));
        } else {
            limitsParticles = std::vector<std::size_t>(limitsParticles.begin() + 2 * from,
                limitsParticles.begin() + 2 * to);
            limitsEdges = std::vector<std::size_t>(limitsEdges.begin() + 2 * from,
                limitsEdges.begin() + 2 * to);
        }
    }

    from = std::min(nFrames, from);
    to = std::min(nFrames, to);

    // time
    std::vector<readdy::TimeStep> time;
    if (stride > 1) {
        group.readSelection("time", time, { from }, { stride }, { to - from });
    } else {
        group.readSelection("time", time, { from }, { 1 }, { to - from });
    }

    if (limitsParticles.size() % 2 != 0 || limitsEdges.size() % 2 != 0) {
        throw std::logic_error(fmt::format(
            "limitsParticles size was {} and limitsEdges size was {}, they should be divisible by 2; from={}, to={}",
            limitsParticles.size(), limitsEdges.size(), from, to));
    }
    // now check that nFrames(particles) == nFrames(edges) == nFrames(time)...
    if (to - from != limitsEdges.size() / 2 || (to - from) != time.size()) {
        throw std::logic_error(fmt::format(
            "(to-from) should be equal to limitsEdges/2 and equal to the number of time steps in the recording, "
            "but was: (to-from) = {}, limitsEdges/2 = {}, nTimeSteps={}; from={}, to={}",
            to - from, limitsEdges.size() / 2, time.size(), from, to));
    }

    std::vector<std::vector<readdy::TopologyTypeId>> types;
    group.readVLENSelection("types", types, { from }, { stride }, { to - from });

    std::vector<std::vector<TopologyRecord>> result;

    std::size_t ix = 0;
    for (std::size_t frame = from; frame < to; ++frame, ++ix) {
        result.emplace_back();
        // this frame's records
        auto& records = result.back();

        const auto& particlesLimitBegin = limitsParticles.at(2 * ix);
        const auto& particlesLimitEnd = limitsParticles.at(2 * ix + 1);
        // since the edges are flattened, we actually have to multiply this by 2
        const auto& edgesLimitBegin = limitsEdges.at(2 * ix);
        const auto& edgesLimitEnd = limitsEdges.at(2 * ix + 1);

        std::vector<std::size_t> flatParticles;
        group.readSelection("particles", flatParticles, { particlesLimitBegin }, { stride }, { particlesLimitEnd - particlesLimitBegin });
        std::vector<std::size_t> flatEdges;
        // readdy::log::critical("edges {} - {}", edgesLimitBegin, edgesLimitEnd);
        group.readSelection("edges", flatEdges, { edgesLimitBegin, 0 }, { stride, 1 }, { edgesLimitEnd - edgesLimitBegin, 2 });

        const auto& currentTypes = types.at(ix);
        auto typesIt = currentTypes.begin();
        for (auto particlesIt = flatParticles.begin();
             particlesIt != flatParticles.end(); ++particlesIt) {

            records.emplace_back();
            auto nParticles = *particlesIt;
            for (std::size_t i = 0; i < nParticles; ++i) {
                ++particlesIt;
                records.back().particleIndices.push_back(*particlesIt);
            }
            records.back().type = *typesIt;
            ++typesIt;
        }

        if (currentTypes.size() != records.size()) {
            throw std::logic_error(fmt::format("for frame {} had {} topology types but {} topologies",
                frame, currentTypes.size(), records.size()));
        }

        std::size_t recordIx = 0;
        for (auto edgesIt = flatEdges.begin();
             edgesIt != flatEdges.end(); ++recordIx) {
            auto& currentRecord = records.at(recordIx);

            auto nEdges = *edgesIt;
            edgesIt += 2;

            for (std::size_t i = 0; i < nEdges; ++i) {
                currentRecord.edges.emplace_back(*edgesIt, *(edgesIt + 1));
                edgesIt += 2;
            }
        }
    }

    if (time.size() != result.size()) {
        throw std::logic_error(fmt::format("readTopologies: size mismatch, time size is {}, result size is {}",
            time.size(), result.size()));
    }
    return std::make_tuple(time, result);
}

std::vector<std::size_t> getNeighbors(
    std::size_t particleId,
    const TopologyH5List& topologies)
{
    std::vector<std::size_t> neighbors;
    bool done = false;

    std::size_t topologyCount = topologies.size();

    for (std::size_t topologyIndex = 0; topologyIndex < topologyCount; ++topologyIndex) {
        auto topology = topologies.at(topologyIndex);
        for (std::size_t edgeIndex = 0; edgeIndex < topology.edges.size(); ++edgeIndex) {
            auto edge = topology.edges[edgeIndex];
            auto p1 = topology.particleIndices.at(std::get<0>(edge));
            auto p2 = topology.particleIndices.at(std::get<1>(edge));

            if (p1 == particleId) {
                neighbors.push_back(p2);
            } else if (p2 == particleId) {
                neighbors.push_back(p1);
            }
        }

        // Assuming each particle is only part of one topology
        //  this means that if nieghbors are found, this function is done
        if (neighbors.size() > 0) {
            return neighbors;
        }
    }

    return neighbors;
}

static MonomerType getMonomerType(
    std::string readdyParticleType)
{
    // ReaDDy particle types contain the main type as well as state info
    // i.e. ReaDDy particle type = [main type]#[state flags (delimited by "_")]

    // get the main type
    char delimiter = '#';
    auto pos = readdyParticleType.find(delimiter);
    std::string mainType = readdyParticleType.substr(0, pos);
    std::string flagStr;
    if (pos != std::string::npos) {
        flagStr = readdyParticleType.substr(pos + 1, readdyParticleType.size() - pos);
    } else {
        return MonomerType(mainType, {}, 101);
    }

    // get the state flags
    delimiter = '_';
    std::vector<std::string> flags;
    auto start = 0U;
    auto end = flagStr.find(delimiter);
    while (end != std::string::npos) {
        flags.push_back(flagStr.substr(start, end - start));
        start = end + 1;
        end = flagStr.find(delimiter, start);
    }
    flags.push_back(flagStr.substr(start, flagStr.length() - start));

    // if one of the flags is a number, set it as the monomer number
    int number = 101;
    for (std::size_t i = 0; i < flags.size(); ++i) {
        if (!flags[i].empty() && std::all_of(flags[i].begin(), flags[i].end(), ::isdigit)) {
            number = std::stoi(flags[i]);
            flags.erase(flags.begin() + i);
            break;
        }
    }

    return MonomerType(mainType, flags, number);
}

static int getMonomerN(
    int actualMonomerNumber,
    MonomerType monomerType)
{
    auto n = actualMonomerNumber + monomerType.number;

    if (n > 3)
        n -= 3;
    if (n < 1)
        n += 3;

    return n;
}

static bool monomerTypeIsSatisfied(
    MonomerType referenceMonomerType,
    int referenceActualMonomerNumber,
    MonomerType testMonomerType,
    int testActualMonomerNumber)
{
    // check that the main types are the same
    if (testMonomerType.name != referenceMonomerType.name) {
        return false;
    }

    // check that the monomer number is correct
    if (referenceMonomerType.number <= 100) {
        if (getMonomerN(testActualMonomerNumber, testMonomerType) != getMonomerN(referenceActualMonomerNumber, referenceMonomerType)) {
            return false;
        }
    }

    // check that all the required flags are present
    auto referenceFlags = referenceMonomerType.flags;
    auto testFlags = testMonomerType.flags;
    for (std::size_t i = 0; i < referenceFlags.size(); ++i) {
        if (referenceFlags[i] == "any") {
            return true;
        }
        if (std::find(testFlags.begin(), testFlags.end(), referenceFlags[i]) == testFlags.end()) {
            return false;
        }
    }
    return true;
}

static std::string monomerTypeToString(
    MonomerType monomerType)
{
    std::string s = "[" + monomerType.name + " (";
    auto flags = monomerType.flags;
    for (std::size_t i = 0; i < flags.size(); ++i) // numberOfFrames
    {
        s += flags[i] + ", ";
    }
    return s + ") " + std::to_string(monomerType.number) + "]";
}

static std::vector<std::pair<MonomerType, OrientationData>> getOrientationDataForParticle(
    const OrientationDataMap& orientationDataLookup,
    MonomerType particleMonomerType)
{
    // get the relevant orientation data for this particle
    auto particleOrientationData = orientationDataLookup.find(particleMonomerType.name);
    if (particleOrientationData == orientationDataLookup.end()) {
        if (verboseOrientation) {
            LOG_F(ERROR, "Failed to find orientation data for %s",
                monomerTypeToString(particleMonomerType).c_str());
        }
        return {};
    }
    return particleOrientationData->second;
}

static std::vector<RelativeOrientationData> getNeighborOrientationData(
    std::size_t particleID,
    MonomerType particleMonomerType,
    std::vector<std::pair<MonomerType, OrientationData>> particleOrientationData,
    const ParticleH5List& trajectoryFrame,
    const TopologyH5List& topologyFrame,
    const std::unordered_map<std::size_t, std::size_t>& idmappingFrame)
{
    // match actual neighbors to orientation data
    std::vector<RelativeOrientationData> neighborOrientationData {};
    auto neighborIds = getNeighbors(particleID, topologyFrame);
    for (std::size_t i = 0; i < particleOrientationData.size(); ++i) {
        for (std::size_t j = 0; j < neighborIds.size(); ++j) {
            if (idmappingFrame.find(neighborIds.at(j)) == idmappingFrame.end()) {
                if (verboseOrientation) {
                    LOG_F(ERROR, "Neighbor ID not found in id mapping! %zu", neighborIds.at(j));
                }
                continue;
            }
            auto neighborID = idmappingFrame.at(neighborIds.at(j));
            auto neighborParticle = trajectoryFrame.at(neighborID);
            auto neighborMonomerType = getMonomerType(neighborParticle.type);

            if (monomerTypeIsSatisfied(
                    particleOrientationData.at(i).first,
                    particleMonomerType.number,
                    neighborMonomerType, 0)) {
                neighborOrientationData.push_back(RelativeOrientationData(
                    neighborID,
                    particleOrientationData.at(i).second));
            }
        }
    }
    return neighborOrientationData;
}

static Eigen::Vector3d getNeighborNonPeriodicBoundaryPosition(
    Eigen::Vector3d particlePosition,
    Eigen::Vector3d neighborParticlePosition,
    Eigen::Vector3d boxSize)
{
    std::vector<float> result {};
    for (std::size_t dim = 0; dim < 3; ++dim) {
        if (abs(neighborParticlePosition[dim] - particlePosition[dim]) > boxSize[dim] / 2.) {
            if (verboseOrientation) {
                LOG_F(INFO, "neighbor crossed periodic boundary in dimension %zu", dim);
            }
            result.push_back(neighborParticlePosition[dim] - neighborParticlePosition[dim] / abs(neighborParticlePosition[dim]) * boxSize[dim]);
        } else {
            result.push_back(neighborParticlePosition[dim]);
        }
    }
    return Eigen::Vector3d(result[0], result[1], result[2]);
}

static Eigen::Matrix3d getCurrentRotation(
    ParticleData neighborParticle0,
    ParticleData currentParticle,
    ParticleData neighborParticle1,
    Eigen::Vector3d boxSize)
{
    std::vector<Eigen::Vector3d> basisPositions {};
    auto rpos1 = currentParticle.position;
    auto pos1 = Eigen::Vector3d(rpos1[0], rpos1[1], rpos1[2]);

    auto rpos0 = neighborParticle0.position;
    auto pos0 = Eigen::Vector3d(rpos0[0], rpos0[1], rpos0[2]);
    auto neighbor0Position = getNeighborNonPeriodicBoundaryPosition(pos1, pos0, boxSize);
    basisPositions.push_back(neighbor0Position);

    basisPositions.push_back(pos1);

    auto rpos2 = neighborParticle1.position;
    auto pos2 = Eigen::Vector3d(rpos2[0], rpos2[1], rpos2[2]);
    auto neighbor2Position = getNeighborNonPeriodicBoundaryPosition(pos1, pos2, boxSize);
    basisPositions.push_back(neighbor2Position);

    return aics::simularium::mathutil::GetRotationMatrix(basisPositions);
}

static Eigen::Matrix3d getInitialRotation(
    std::vector<RelativeOrientationData> neighborOrientationData)
{
    std::vector<Eigen::Vector3d> basisPositions {};
    basisPositions.push_back(neighborOrientationData.at(0).data.localPosition);
    basisPositions.push_back(Eigen::Vector3d(0, 0, 0));
    basisPositions.push_back(neighborOrientationData.at(1).data.localPosition);

    return aics::simularium::mathutil::GetRotationMatrix(basisPositions);
}

static Eigen::Matrix3d getRotationUsingAxis(
    ParticleData particle,
    ParticleData neighborParticle,
    Eigen::Matrix3d axisRotation,
    Eigen::Vector3d boxSize)
{
    auto rpos1 = particle.position;
    auto pos1 = Eigen::Vector3d(rpos1[0], rpos1[1], rpos1[2]);

    auto rpos0 = neighborParticle.position;
    auto pos0 = Eigen::Vector3d(rpos0[0], rpos0[1], rpos0[2]);
    auto neighbor0Position = getNeighborNonPeriodicBoundaryPosition(pos1, pos0, boxSize);

    Eigen::Vector3d axis = neighbor0Position - pos1;
    auto normal = aics::simularium::mathutil::getRandomPerpendicularVector(axis);
    auto pos2 = pos1 + normal;

    std::vector<Eigen::Vector3d> basisPositions {};
    basisPositions.push_back(neighbor0Position);
    basisPositions.push_back(pos1);
    basisPositions.push_back(pos2);

    Eigen::Matrix3d rotation = aics::simularium::mathutil::GetRotationMatrix(basisPositions);
    return rotation * axisRotation;
}

static void calculateOrientations(
    const TopologyH5Info& topologyH5Info,
    const TrajectoryH5Info& trajectoryH5Info,
    std::shared_ptr<aics::simularium::ReaDDyFileInfo>& readdyFileInfo,
    const IdParticleMapping& particleLookup,
    const OrientationDataMap& orientationDataLookup)
{
    auto numberOfFrames = trajectoryH5Info.size();
    auto topologyStride = (trajectoryH5Info.size() / topologyH5Info.size()) + 1;
    RotationH5Info& outRotations = readdyFileInfo->rotationInfo;
    outRotations.resize(numberOfFrames);

    auto boxSize = Eigen::Vector3d(
        readdyFileInfo->configInfo.boxX,
        readdyFileInfo->configInfo.boxY,
        readdyFileInfo->configInfo.boxZ);

    for (std::size_t frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex) {
        auto trajectoryFrame = trajectoryH5Info.at(frameIndex);
        auto topologyFrame = topologyH5Info.at(frameIndex / topologyStride);
        auto& rotationFrame = outRotations.at(frameIndex);
        auto& idmappingFrame = particleLookup.at(frameIndex);

        std::vector<Eigen::Matrix3d> orientationFrame {};
        std::unordered_map<std::size_t, RelativeOrientationData> particlesToOrientRelativeToNeighbor;

        // match particles to orientation data and calculate orientation for particles with 0 or 2 neighbors
        for (std::size_t particleIndex = 0; particleIndex < trajectoryFrame.size(); ++particleIndex) {
            auto currentParticle = trajectoryFrame.at(particleIndex);
            if (is_child_particle(currentParticle.type)) {
                orientationFrame.push_back(aics::simularium::mathutil::getErrorOrientation(0));
                continue;
            }

            auto particleMonomerType = getMonomerType(currentParticle.type);
            auto particleOrientationData = getOrientationDataForParticle(orientationDataLookup, particleMonomerType);
            auto neighborOrientationData = getNeighborOrientationData(
                currentParticle.id, particleMonomerType, particleOrientationData,
                trajectoryFrame, topologyFrame, idmappingFrame);

            if (neighborOrientationData.size() < 1) {
                if (default_orientation.find(particleMonomerType.name) != default_orientation.end()) {
                    // no neighbors, use default orientation
                    if (verboseOrientation) {
                        LOG_F(INFO, "%zu: Use default orientation for %s %zu",
                            frameIndex, currentParticle.type.c_str(), currentParticle.id);
                    }
                    orientationFrame.push_back(default_orientation[particleMonomerType.name]);
                    continue;
                }
                // no neighbors, use random rotation
                if (verboseOrientation) {
                    LOG_F(INFO, "%zu: Use random orientation for %s %zu",
                        frameIndex, currentParticle.type.c_str(), currentParticle.id);
                }
                orientationFrame.push_back(aics::simularium::mathutil::getRandomOrientation());
                continue;
            }

            if (neighborOrientationData.size() < 2) {
                // one neighbor, revisit after neighbor's orientation is calculated
                if (verboseOrientation) {
                    LOG_F(INFO, "%zu: Use one neighbor orientation of %s %zu",
                        frameIndex, currentParticle.type.c_str(), currentParticle.id);
                }
                orientationFrame.push_back(aics::simularium::mathutil::getErrorOrientation(0));
                particlesToOrientRelativeToNeighbor[particleIndex] = RelativeOrientationData(
                    neighborOrientationData.at(0));
                continue;
            }

            // two+ neighbors, use vectors to neighbors to construct current rotation matrix
            // and subtract the initial rotation for the particle constructed from the ideal
            // relative positions of the neighbors
            Eigen::Matrix3d currentRotation = getCurrentRotation(
                trajectoryFrame.at(neighborOrientationData.at(0).neighborID),
                currentParticle,
                trajectoryFrame.at(neighborOrientationData.at(1).neighborID),
                boxSize);
            Eigen::Matrix3d initialRotation = getInitialRotation(neighborOrientationData);
            orientationFrame.push_back(currentRotation.inverse() * initialRotation);
            if (verboseOrientation) {
                LOG_F(INFO, "%zu: Successfully oriented %s %zu",
                    frameIndex, currentParticle.type.c_str(), currentParticle.id);
            }
        }

        // go back and calculate orientation for particles with one neighbor
        // since their orientation is dependent on the neighbor's orientation
        for (auto it : particlesToOrientRelativeToNeighbor) {
            auto particleID = it.first;

            if (orientationFrame.at(particleID) != aics::simularium::mathutil::getErrorOrientation(0)) {
                continue;
            }

            auto neighborID = it.second.neighborID;
            auto neighborOrientation = orientationFrame.at(neighborID);
            if (neighborOrientation == aics::simularium::mathutil::getErrorOrientation(0)) {
                // neighbor hasn't been oriented, so use axis rotation instead
                orientationFrame.at(particleID) = getRotationUsingAxis(
                    trajectoryFrame.at(particleID),
                    trajectoryFrame.at(neighborID),
                    it.second.data.axisRotation,
                    boxSize);
                if (verboseOrientation) {
                    LOG_F(INFO, "%zu: Successfully oriented %zu with axis rotation from %zu",
                        frameIndex, particleID, neighborID);
                }
                continue;
            }

            auto offsetRotation = it.second.data.localRotation.inverse();

            orientationFrame.at(particleID) = neighborOrientation * offsetRotation;
            if (verboseOrientation) {
                LOG_F(INFO, "%zu: Successfully oriented %zu with one neighbor",
                    frameIndex, particleID);
            }
        }

        // save all the orientations as euler angles
        for (std::size_t i = 0; i < orientationFrame.size(); ++i) {
            rotationFrame.push_back(orientationFrame[i].eulerAngles(0, 1, 2));
        }
    }
}
