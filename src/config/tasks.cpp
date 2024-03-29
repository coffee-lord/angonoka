#include "../common.h"
#include "../exceptions.h"
#include "load.h"
#include <concepts>
#include <fmt/format.h>
#include <gsl/gsl-lite.hpp>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/view/zip.hpp>

namespace {
using namespace angonoka;
using namespace fmt::literals;
using Dependencies = std::vector<std::vector<std::string_view>>;
/**
    Parses task duration.

    Parses blocks such as these:

    duration:
      min: 1 day
      max: 3 days

    @param duration A map with min/max values or a scalar
    @param task     Task object
*/
void parse_duration(const YAML::Node& duration, Task& task)
{
    using detail::parse_duration;
    auto& dur = task.duration;
    try {
        if (duration.IsScalar()) {
            const auto value = parse_duration(duration.Scalar());
            dur.min = value;
            dur.max = value;
        } else {
            dur.min = parse_duration(duration["min"].Scalar());
            dur.max = parse_duration(duration["max"].Scalar());
        }
    } catch (const DurationParseError& e) {
        throw InvalidDuration(task.name, e.text);
    }
    if (dur.min > dur.max) throw TaskDurationMinMax{task.name};
}

/**
    Parses task group.

    Parses blocks such as these:

    group: A

    @param group_node Scalar holding the name of the group
    @param task       An instance of Task
    @param config     An instance of Project
*/
void parse_task_group(
    const YAML::Node& group_node,
    Task& task,
    Project& config)
{
    const auto& group_name = group_node.Scalar();
    Expects(!group_name.empty());
    const auto [gid, is_inserted]
        = detail::find_or_insert_group(config.groups, group_name);
    if (is_inserted && !has_universal_agents(config))
        throw NoSuitableAgent{group_name};
    task.group_ids.emplace(gid);

    Ensures(!task.group_ids.empty());
}

/**
    Parse task groups.

    Parses blocks such as these:

    groups:
      - A
      - B

    @param groups_node  YAML node of groups
    @param task         An instance of Task
    @param confg        An instance of Project
*/
void parse_task_groups(
    const YAML::Node& groups_node,
    Task& task,
    Project& config)
{
    for (auto&& group : groups_node)
        parse_task_group(group, task, config);
}

/**
    Checks if a given id has already been used.

    @param tasks    Array of Tasks
    @param id       Task id to check
*/
void check_for_duplicates(const Tasks& tasks, std::string_view id)
{
    Expects(!id.empty());
    if (const auto a = ranges::find(tasks, id, &Task::id);
        a != tasks.end())
        throw DuplicateTaskDefinition{id};
}

/**
    Find the task index by id.

    @param tasks    An array of Tasks
    @param id       Task id

    @return Task index
*/
TaskIndex
find_task_index_by_id(const Tasks& tasks, std::string_view id)
{
    Expects(!id.empty());
    if (const auto a = ranges::find(tasks, id, &Task::id);
        a != tasks.end())
        return std::distance(tasks.begin(), a);
    throw TaskNotFound{id};
}

/**
    Find agent's id by name.

    @param agents   Array of agents
    @param name     Agents name

    @return Agent's id
*/
AgentIndex
find_agent_index_by_name(const Agents& agents, std::string_view name)
{
    Expects(!agents.empty());
    Expects(!name.empty());

    if (const auto a = ranges::find(agents, name, &Agent::name);
        a != agents.end())
        return a->id;
    throw AgentNotFound{name};
}

/**
    Parses Task ids.

    Parses blocks such as these:

    id: X

    @param id_node  Scalar containing the id
    @param tasks    Array of Tasks
    @param task     An instance of Task
*/
void parse_task_id(
    const YAML::Node& id_node,
    const Tasks& tasks,
    Task& task)
{
    const auto& id = id_node.Scalar();
    if (id.empty()) {
        throw CantBeEmpty{
            R"(Task id for the task "{}")"_format(task.name)};
    }
    check_for_duplicates(tasks, id);
    task.id = id;

    Ensures(!task.id.empty());
}

/**
    Validate a Task id.

    @param task_id  Task id
    @param task     Task object

    @return A Task id, if validation succeeds
*/
std::string_view
validate_task_id(std::string_view task_id, const Task& task)
{
    if (task_id.empty()) {
        throw CantBeEmpty{
            R"(Dependency id of the task "{}")"_format(task.name)};
    }
    return task_id;
}

/**
    Parse Task dependencies.

    Parses blocks such as these:

    depends_on:
      - A
      - B

    This is the first phase of dependency resolution. See
    parse_dependencies_2nd_phase for the second phase.

    @param depends_on   A sequence or a scalar of dependencies
    @param task_deps    Array of Task dependencies
    @param task         Task object
*/
void parse_dependencies(
    const YAML::Node& depends_on,
    std::vector<std::string_view>& task_deps,
    const Task& task)
{
    Expects(depends_on.IsSequence() || depends_on.IsScalar());

    if (depends_on.IsSequence()) {
        for (const auto& d : depends_on) {
            task_deps.emplace_back(
                validate_task_id(d.Scalar(), task));
        }
    } else {
        task_deps.emplace_back(
            validate_task_id(depends_on.Scalar(), task));
    }

    Ensures(!task_deps.empty());
}

// Forward decl
void parse_task(
    const YAML::Node& task_node,
    Project& config,
    Dependencies& deps);

/**
    Parse subtasks definition.

    Parses blocks such as these:

    subtasks:
      - ...

    @param subtasks     YAML sequence of subtasks
    @param config       An instance of Project
    @param deps         Array of dependencies
    @param task_index   Current task's index
*/
void parse_subtasks(
    const YAML::Node& subtasks,
    Project& config,
    Dependencies& deps,
    int8 task_index)
{
    Expects(!config.tasks.empty());
    Expects(task_index < config.tasks.size());

    for (auto&& sub : subtasks) {
        // The next task will be a depencency
        config.tasks[task_index].dependencies.emplace(
            config.tasks.size());
        parse_task(sub, config, deps);
    }
}

/**
    Parse task's dedicated agent.

    Parses blocks such as these:

    agent: Bob
           ^ this

    @param agent    YAML scalar
    @param task     Task for the agent
    @param agents   Array of agents
*/
void parse_task_agent(
    const YAML::Node& agent,
    Task& task,
    const Agents& agents)
{
    Expects(!agents.empty());
    Expects(agent.IsScalar());

    const auto& name = agent.Scalar();
    if (name.empty()) throw CantBeEmpty{"Assigned agents name"};
    task.agent_id = find_agent_index_by_name(agents, name);

    Ensures(task.agent_id);
}

/**
    Count how many YAML nodes are defined.

    @param vars YAML nodes

    @return Number of defined nodes.
*/
int count_defined(
    concepts::explicitly_convertible_to<bool> auto const&... vars)
{
    return (static_cast<int>(static_cast<bool>(vars)) + ...);
}

/**
    Parses task's groups and dedicated agents definition.

    Parses blocks such as these:

    agent: Bob

    and

    groups:
     - group 1

    @param task_node    YAML node containing task data
    @param task         Instance of Task
    @param config       Instance of Project
*/
void parse_groups_and_agents(
    const YAML::Node& task_node,
    Task& task,
    Project& config)
{
    Expects(!task.name.empty());

    if (count_defined(
            task_node["group"],
            task_node["groups"],
            task_node["agent"])
        > 1)
        throw InvalidTaskAssignment(task.name);

    // Parse task.group
    if (const auto& group = task_node["group"]) {
        parse_task_group(group, task, config);
        return;
    }

    // Parse task.group
    if (const auto& group = task_node["groups"]) {
        parse_task_groups(group, task, config);
        return;
    }

    // Parse task.agent
    if (const auto& agent = task_node["agent"]) {
        parse_task_agent(agent, task, config.agents);
        return;
    }
}

/**
    Parses task blocks.

    Parses blocks such as these:

    name: task 1
    group: A
    days:
      min: 2
      max: 2

    @param task_node    Scalar holding the name of the task
    @param config       An instance of Project
    @param deps         Array of dependencies
*/
void parse_task(
    const YAML::Node& task_node,
    Project& config,
    Dependencies& deps)
{
    Expects(!task_node.IsNull());
    Expects(config.tasks.size() == deps.size());

    const auto& name = task_node["name"].Scalar();
    if (name.empty()) throw CantBeEmpty{"Task name"};

    auto& task = config.tasks.emplace_back();
    auto& task_deps = deps.emplace_back();

    task.name = name;

    // Parse task.id
    if (const auto& id_node = task_node["id"])
        parse_task_id(id_node, config.tasks, task);

    parse_duration(task_node["duration"], task);
    parse_groups_and_agents(task_node, task, config);

    // Parse task.depends_on
    if (const auto& depends_on = task_node["depends_on"])
        parse_dependencies(depends_on, task_deps, task);

    // Parse task.subtasks
    if (const auto& subtasks = task_node["subtasks"]) {
        const auto task_index = config.tasks.size() - 1;
        parse_subtasks(subtasks, config, deps, task_index);
    }

    Ensures(config.tasks.size() == deps.size());
}

/**
    Second pass of parsing task dependencies.

    Resolves Task ids and pupulates task dependencies.
    Two phase lookup is needed because some task ids might
    not be resolved during the first pass.

    @param tasks   Array of Tasks
    @param deps    2D array of Task dependencies
*/
void parse_dependencies_2nd_phase(
    Tasks& tasks,
    const Dependencies& deps)
{
    Expects(!tasks.empty());
    Expects(tasks.size() == deps.size());

    using ranges::views::zip;
    for (auto&& [task, tdeps] : zip(tasks, deps)) {
        for (auto dep : tdeps) {
            const auto dep_index = find_task_index_by_id(tasks, dep);
            task.dependencies.emplace(dep_index);
        }
    }
}

/**
    State of the vertex for the dependency graph cycle detection.
*/
enum class VertexState : std::uint8_t {
    not_visited,
    visited,
    complete
};

/**
    Depth first search algorithm for dependency cycle detection.

    @param tasks    Array of Tasks
    @param state    Array of vertex states
    @param task_id  Current task id
*/
void depth_first_search(
    const Tasks& tasks,
    std::vector<VertexState>& state,
    int8 task_id)
{
    Expects(!tasks.empty());
    Expects(tasks.size() == state.size());
    Expects(state[task_id] == VertexState::not_visited);
    Expects(task_id < tasks.size());

    state[task_id] = VertexState::visited;
    for (const auto& dep_id : tasks[task_id].dependencies) {
        switch (state[dep_id]) {
        case VertexState::visited: throw DependencyCycle();
        case VertexState::not_visited:
            depth_first_search(tasks, state, dep_id);
        default: continue;
        }
    }
    state[task_id] = VertexState::complete;
}

/**
    Makes sure task dependencies have no cycles.

    @param tasks Array of tasks
*/
void check_for_cycles(const Tasks& tasks)
{
    Expects(!tasks.empty());

    std::vector<VertexState> state(tasks.size());
    for (int8 task_id{0}; task_id < tasks.size(); ++task_id) {
        if (state[task_id] != VertexState::not_visited) continue;
        depth_first_search(tasks, state, task_id);
    }
}
} // namespace

namespace angonoka::detail {
void parse_tasks(const YAML::Node& node, Project& config)
{
    Expects(config.tasks.empty());

    if (node.size() == 0) throw CantBeEmpty{R"_("tasks")_"};
    Dependencies deps;
    for (auto&& task : node) parse_task(task, config, deps);
    parse_dependencies_2nd_phase(config.tasks, deps);
    check_for_cycles(config.tasks);

    Ensures(!config.tasks.empty());
}
} // namespace angonoka::detail
