#include "project.h"
#include "exceptions.h"
#include <gsl/gsl-lite.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/set_algorithm.hpp>

namespace angonoka {
bool is_universal(const Agent& agent) noexcept
{
    return agent.group_ids.empty();
}

bool can_work_on(const Agent& agent, GroupIndex id) noexcept
{
    Expects(id >= 0);

    if (is_universal(agent)) return true;
    return agent.group_ids.contains(id);
}

[[nodiscard]] bool
can_work_on(const Agent& agent, const Task& task) noexcept
{
    using ranges::includes;
    if (task.agent_id) return task.agent_id == agent.id;
    return is_universal(agent)
        || (!task.group_ids.empty()
            && includes(agent.group_ids, task.group_ids));
}

bool has_universal_agents(const Project& config) noexcept
{
    Expects(!config.agents.empty());

    return ranges::any_of(config.agents, is_universal);
}

float Agent::Performance::average() const
{
    Expects(min > 0.F);
    Expects(min <= max);
    // NOLINTNEXTLINE: cppcoreguidelines-avoid-magic-numbers
    return (min + max) / 2.F;
}

std::chrono::seconds Task::Duration::average() const
{
    Expects(min > std::chrono::seconds{0});
    Expects(min <= max);
    // NOLINTNEXTLINE: cppcoreguidelines-avoid-magic-numbers
    return (min + max) / 2;
}
} // namespace angonoka
