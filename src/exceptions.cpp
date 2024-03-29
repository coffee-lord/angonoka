#include "exceptions.h"
#include <fmt/format.h>

namespace angonoka {
using namespace fmt::literals;
DurationParseError::DurationParseError(std::string_view text)
    : text{text}
{
}
InvalidDuration::InvalidDuration(
    std::string_view where,
    std::string_view what)
    : ValidationError{
        R"(Task "{}" has invalid duration "{}".)"_format(where, what)}
{
}
InvalidAgentPerformance::InvalidAgentPerformance(std::string_view who)
    : ValidationError{
        R"(Agent "{}" has invalid performance.)"_format(who)}
{
}
AgentPerformanceMinMax::AgentPerformanceMinMax(std::string_view who)
    : ValidationError{
        R"(The minimum performance of the agent "{}" is greater than maximum.)"_format(
            who)}
{
}
DuplicateAgentDefinition::DuplicateAgentDefinition(
    std::string_view who)
    : ValidationError{
        R"(Agent "{}" is specified more than once.)"_format(who)}
{
}
TaskDurationMinMax::TaskDurationMinMax(std::string_view where)
    : ValidationError{
        R"(Task "{}" has min duration that is greater than max duration.)"_format(
            where)}
{
}
NoSuitableAgent::NoSuitableAgent(std::string_view task)
    : ValidationError{
        R"_(No suitable agent for task "{}".)_"_format(task)}
{
}
DuplicateTaskDefinition::DuplicateTaskDefinition(
    std::string_view task_id)
    : ValidationError{R"(Duplicate task id "{}".)"_format(task_id)}
{
}
NegativePerformance::NegativePerformance(std::string_view who)
    : ValidationError{
        R"(Agent "{}" can't have a negative performance value.)"_format(
            who)}
{
}

CantBeEmpty::CantBeEmpty(std::string_view what)
    : ValidationError{"{} can't be empty."_format(what)}
{
}

InvalidTaskAssignment::InvalidTaskAssignment(std::string_view task)
    : ValidationError{
        R"(Task "{}" must have at most one of: agent, group, groups.)"_format(
            task)}
{
}

TaskNotFound::TaskNotFound(std::string_view task_id)
    : ValidationError{
        R"_(Task with id "{}" doesn't exist.)_"_format(task_id)}
{
}

AgentNotFound::AgentNotFound(std::string_view name)
    : ValidationError{R"_(Agent "{}" doesn't exist.)_"_format(name)}
{
}

DependencyCycle::DependencyCycle()
    : ValidationError{"Dependency cycle detected."}
{
}
} // namespace angonoka
