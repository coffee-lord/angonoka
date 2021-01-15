#include "../common.h"
#include "../exceptions.h"
#include "load.h"
#include <fmt/format.h>
#include <gsl/gsl-lite.hpp>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/view/zip.hpp>

namespace {
using namespace angonoka;
using Dependencies = std::vector<std::vector<std::string_view>>;
/**
    Parses task duration.

    Parses blocks such as these:

    duration:
      min: 1 day
      max: 3 days

    @param duration  A map with min/max values or a scalar
    @param task      Task object
*/
void parse_duration(const YAML::Node& duration, Task::Duration& dur)
{
    using detail::parse_duration;
    if (duration.IsScalar()) {
        const auto value = parse_duration(duration.Scalar());
        dur.min = value;
        dur.max = value;
    } else {
        dur.min = parse_duration(duration["min"].Scalar());
        dur.max = parse_duration(duration["max"].Scalar());
    }
    if (dur.min > dur.max) throw TaskDurationMinMax{};
}

/**
    Parses task group.

    Parses blocks such as these:

    group: A

    @param group_node Scalar holding the name of the group
    @param task       An instance of Task
    @param system     An instance of System
*/
void parse_task_group(
    const YAML::Node& group_node,
    Task& task,
    System& system)
{
    const auto& group_name = group_node.Scalar();
    Expects(!group_name.empty());
    const auto [gid, is_inserted]
        = detail::find_or_insert_group(system.groups, group_name);
    if (is_inserted && !has_universal_agents(system))
        throw NoSuitableAgent{group_name};
    task.group_id = gid;
}

/**
    Check for duplicate tasks.
    TODO: Legacy, remove

    @param agents An array of Tasks
    @param name   Agent's name
*/
void check_for_duplicates(const Tasks& tasks, std::string_view name)
{
    Expects(!name.empty());
    if (const auto a = ranges::find(tasks, name, &Task::name);
        a != tasks.end())
        throw DuplicateTaskDefinition{};
}

/**
    TODO: doc
*/
void check_for_duplicates_new(const Tasks& tasks, std::string_view id)
{
    Expects(!id.empty());
    if (const auto a = ranges::find(tasks, id, &Task::id);
        a != tasks.end())
        throw DuplicateTaskDefinition{};
}

/**
    TODO: doc
*/
TaskId find_task_index_by_id(const Tasks& tasks, std::string_view id)
{
    Expects(!id.empty());
    if (const auto a = ranges::find(tasks, id, &Task::id);
        a != tasks.end())
        return std::distance(tasks.begin(), a);
    throw TaskNotFound{id};
}

/**
    Parses task blocks.

    Parses blocks such as these:

    task 1:
      group: A
      days:
        min: 2
        max: 2

    @param task_node  Scalar holding the name of the task
    @param task_node  Map with task data
    @param sys        An instance of System
*/
void parse_task(
    const YAML::Node& task_node,
    const YAML::Node& task_data,
    System& sys)
{
    const auto& task_name = task_node.Scalar();
    Expects(!task_name.empty());
    check_for_duplicates(sys.tasks, task_name);
    auto& task = sys.tasks.emplace_back();
    task.name = task_name;
    parse_duration(task_data["duration"], task.duration);

    // Parse task.group
    if (const auto group = task_data["group"]) {
        parse_task_group(group, task, sys);
    }
}

/**
    TODO: doc
*/
void parse_task_id(
    const YAML::Node& id_node,
    const Tasks& tasks,
    Task& task)
{
    const auto& id = id_node.Scalar();
    if (id.empty()) throw CantBeEmpty{"Task id"};
    check_for_duplicates_new(tasks, id);
    task.id = id;
}

/**
    TODO: doc
*/
std::string_view validate_task_id(std::string_view task_id)
{
    if (task_id.empty()) throw CantBeEmpty{"Dependency id"};
    return task_id;
}

/**
    TODO: doc
*/
void parse_dependencies(
    const YAML::Node& depends_on,
    std::vector<std::string_view>& task_deps)
{
    if (depends_on.IsSequence()) {
        for (const auto& d : depends_on)
            task_deps.emplace_back(validate_task_id(d.Scalar()));
    } else {
        task_deps.emplace_back(validate_task_id(depends_on.Scalar()));
    }
}

/**
    TODO: doc, rename
*/
void parse_task_new(
    const YAML::Node& task_node,
    System& sys,
    Dependencies& deps)
{
    const auto& name = task_node["name"].Scalar();
    if (name.empty()) throw CantBeEmpty{"Task name"};

    auto& task = sys.tasks.emplace_back();
    auto& task_deps = deps.emplace_back();

    task.name = name;

    // Parse task.id
    if (const auto& id_node = task_node["id"]) {
        parse_task_id(id_node, sys.tasks, task);
    }

    parse_duration(task_node["duration"], task.duration);

    // Parse task.group
    if (const auto& group = task_node["group"]) {
        parse_task_group(group, task, sys);
    }

    // Parse task.depends_on
    if (const auto& depends_on = task_node["depends_on"]) {
        parse_dependencies(depends_on, task_deps);
    }
}

/**
    TODO: doc
*/
void parse_dependencies_2nd_phase(
    Tasks& tasks,
    const Dependencies& deps)
{
    Expects(!tasks.empty());
    Expects(tasks.size() == deps.size());

    using ranges::views::zip;
    for (auto&& [task, deps] : zip(tasks, deps)) {
        for (auto dep : deps) {
            const auto dep_index = find_task_index_by_id(tasks, dep);
            task.dependencies.emplace(dep_index);
        }
    }
}

/**
    TODO: doc
*/
enum class VertexState : std::uint8_t {
    not_visited,
    visited,
    complete
};

/**
    TODO: doc
*/
void depth_first_search(
    const Tasks& tasks,
    std::vector<VertexState>& state,
    int8 task_id)
{
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
    TODO: doc
*/
void check_for_cycles(const Tasks& tasks)
{
    std::vector<VertexState> state(tasks.size());
    for (int8 task_id{0}; task_id < tasks.size(); ++task_id) {
        if (state[task_id] != VertexState::not_visited) continue;
        depth_first_search(tasks, state, task_id);
    }
}
} // namespace

namespace angonoka::detail {
void parse_tasks(const YAML::Node& node, System& sys)
{
    // new
    if (node.IsSequence()) {
        Dependencies deps;
        for (auto&& task : node) { parse_task_new(task, sys, deps); }
        parse_dependencies_2nd_phase(sys.tasks, deps);
        check_for_cycles(sys.tasks);
        return;
    }
    // TODO: Legacy
    for (auto&& task : node) {
        parse_task(task.first, task.second, sys);
    }
}
} // namespace angonoka::detail
