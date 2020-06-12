#include "../exceptions.h"
#include "load.h"
#include <range/v3/algorithm/find.hpp>
#include <string_view>

namespace {
using namespace angonoka;
/**
    Parses agent groups section.

    Parses blocks such as these:

    groups:
      - A
      - B
      - C

    and inserts "A", "B", "C" into System.groups.
    Then places group ids into agent.groups_ids.

    @param group_nodes    Sequence with group names
    @param agent          An instance of Agent
    @param groups         An array of Groups
*/
void parse_agent_groups(
    const YAML::Node& group_nodes,
    Agent& agent,
    Groups& groups)
{
    for (auto&& g : group_nodes) {
        const auto gid
            = detail::find_or_insert_group(groups, g.Scalar());
        agent.group_ids.emplace(gid.first);
    }
}

/**
    Parses agent perf.

    Parses blocks such as these:

    perf:
      min: 1.0
      max: 2.0

    @param perf   Map with perf data
    @param agent  An instance of Agent
*/
void parse_agent_perf(const YAML::Node& perf, Agent& agent)
{
    try {
        if (perf.IsScalar()) {
            const auto perf_value = perf.as<float>();
            agent.perf.min = perf_value;
            agent.perf.max = perf_value;
        } else {
            agent.perf.min = perf["min"].as<float>();
            agent.perf.max = perf["max"].as<float>();
        }
    } catch (const YAML::Exception&) {
        throw ValidationError{"Invalid agent performance."};
    }
    if (agent.perf.min > agent.perf.max) {
        constexpr auto text = "Agent's performance minimum can't be "
                              "greater than maximum.";
        throw ValidationError{text};
    }
}

/**
    Check for duplicate agents.

    @param agents An array of Agents
    @param name   Agent's name
*/
// NOLINTNEXTLINE(misc-unused-parameters)
void check_for_duplicates(const Agents& agents, std::string_view name)
{
    Expects(!name.empty());
    if (const auto a = ranges::find(agents, name, &Agent::name);
        a != agents.end()) {
        constexpr auto text = "Duplicate agent definition";
        throw ValidationError{text};
    }
}

/**
    Parses agent blocks.

    Parses blocks such as these:

    agent 1:
      perf:
        min: 0.5
        max: 1.5
      groups:
        - A
        - B

    @param agent_node Scalar holding the name of the agent
    @param agent_data Map with agent data
    @param sys        An instance of System
*/
void parse_agent(
    const YAML::Node& agent_node,
    const YAML::Node& agent_data,
    System& sys)
{
    const auto& agent_name = agent_node.Scalar();
    Expects(!agent_name.empty());
    check_for_duplicates(sys.agents, agent_name);
    auto& agent = sys.agents.emplace_back();

    // Parse agent.name
    agent.name = agent_name;

    // Parse agent.groups
    if (const auto groups = agent_data["groups"]) {
        parse_agent_groups(groups, agent, sys.groups);
    }

    // Parse agent.perf
    if (const auto perf = agent_data["performance"]) {
        parse_agent_perf(perf, agent);
    }
}
} // namespace

namespace angonoka::detail {
void parse_agents(const YAML::Node& node, System& sys)
{
    for (auto&& agent : node) {
        parse_agent(agent.first, agent.second, sys);
    }
}
} // namespace angonoka::detail
