#pragma once

#include "common.h"
#include <range/v3/view/span.hpp>
#include <vector>

namespace angonoka::stun_dag {
using ranges::span;

/**
    TODO: doc

    @var agent_performance
    @var task_duration
    @var available_agents_data
    @var available_agents
    @var dependencies_data
    @var dependencies
*/
struct ScheduleInfo {
    std::vector<float> agent_performance;
    std::vector<float> task_duration;

    std::vector<int16> available_agents_data;
    std::vector<span<int16>> available_agents;

    std::vector<int16> dependencies_data;
    std::vector<span<int16>> dependencies;
};

} // namespace angonoka::stun_dag
