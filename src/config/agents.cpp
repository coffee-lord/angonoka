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

    and inserts "A", "B", "C" into Project.groups.
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
    Makes sure the performance is greater than 0.

    @param performance  Performance value
    @param agent        Agent

    @return Performance value
*/
float validate_performance(float performance)
{
    if (performance <= 0.F)
        throw std::domain_error{"Negative performance"};
    return performance;
}

/**
    Parses agent performance.

    Parses blocks such as these:

    performance:
      min: 1.0
      max: 2.0

    @param performance  Map with performance data
    @param agent        An instance of Agent
*/
void parse_agent_performance(
    const YAML::Node& performance,
    Agent& agent)
{
    try {
        if (performance.IsScalar()) {
            const auto performance_value
                = validate_performance(performance.as<float>());
            agent.performance.min = performance_value;
            agent.performance.max = performance_value;
        } else {
            agent.performance.min = validate_performance(
                performance["min"].as<float>());
            agent.performance.max = validate_performance(
                performance["max"].as<float>());
        }
    } catch (const std::domain_error&) {
        throw NegativePerformance{agent.name};
    } catch (const YAML::Exception&) {
        throw InvalidAgentPerformance{agent.name};
    }
    if (agent.performance.min > agent.performance.max)
        throw AgentPerformanceMinMax{agent.name};
}

/**
    Check for duplicate agents.

    @param agents An array of Agents
    @param name   Agent's name
*/
void check_for_duplicates(const Agents& agents, std::string_view name)
{
    Expects(!name.empty());

    if (const auto a = ranges::find(agents, name, &Agent::name);
        a != agents.end())
        throw DuplicateAgentDefinition{name};
}

namespace detail {
    /**
        Strong-typed alias for YAML:Node.
    */
    struct OpaqueNode {
        explicit OpaqueNode(const YAML::Node& node)
            : node{&node}
        {
        }
        gsl::not_null<const YAML::Node*> node;

        decltype(auto) operator->() const
        {
            return node.operator->();
        }

        auto operator[](auto&& arg) const
        {
            return (*node)[std::forward<decltype(arg)>(arg)];
        }
    };
} // namespace detail

/**
    Opaque types to distinguish consecutive
    arguments in functions.
*/
struct AgentNode : detail::OpaqueNode {
    using OpaqueNode::OpaqueNode;
};
struct AgentData : detail::OpaqueNode {
    using OpaqueNode::OpaqueNode;
};

/**
    Parses agent blocks.

    Parses blocks such as these:

    agent 1:
      performance:
        min: 0.5
        max: 1.5
      groups:
        - A
        - B

    @param agent_node Scalar holding the name of the agent
    @param agent_data   Map with agent data
    @param config       An instance of Project
*/
void parse_agent(
    const AgentNode& agent_node,
    const AgentData& agent_data,
    Project& config)
{
    const auto& agent_name = agent_node->Scalar();
    Expects(!agent_name.empty());

    check_for_duplicates(config.agents, agent_name);
    auto& agent = config.agents.emplace_back();
    agent.id = config.agents.size() - 1;

    // Parse agent.name
    agent.name = agent_name;

    // Parse agent.groups
    if (const auto groups = agent_data["groups"])
        parse_agent_groups(groups, agent, config.groups);

    // Parse agent.perf
    if (const auto performance = agent_data["performance"])
        parse_agent_performance(performance, agent);

    Ensures(agent.id >= 0);
    Ensures(!agent.name.empty());
}
} // namespace

namespace angonoka::detail {
void parse_agents(const YAML::Node& node, Project& config)
{
    Expects(config.agents.empty());

    if (node.size() == 0) throw CantBeEmpty{R"_("agents")_"};
    for (auto&& agent : node) {
        parse_agent(
            AgentNode{agent.first},
            AgentData{agent.second},
            config);
    }

    Ensures(!config.agents.empty());
}
} // namespace angonoka::detail
