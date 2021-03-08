#include "utils.h"
#include "random_utils.h"
#include "schedule_params.h"
#include <range/v3/algorithm/binary_search.hpp>
#include <range/v3/algorithm/fill.hpp>
#include <range/v3/algorithm/max.hpp>
#include <range/v3/view/transform.hpp>

namespace angonoka::stun {
Makespan::Makespan(const ScheduleParams& params)
    : params{&params}
    , sum_buffer(
          params.task_duration.size() + params.agent_performance.size())
    , task_done{sum_buffer.data(), static_cast<int>(params.task_duration.size())}
    , work_done{
          task_done.end(),
          static_cast<int>(params.agent_performance.size())}
{
    Expects(!params.agent_performance.empty());
    Expects(!params.task_duration.empty());
    Ensures(!sum_buffer.empty());
    Ensures(
        task_done.size()
        == static_cast<int>(params.task_duration.size()));
    Ensures(
        work_done.size()
        == static_cast<int>(params.agent_performance.size()));
}

Makespan::Makespan(const Makespan& other)
    : Makespan{*other.params}
{
    Ensures(sum_buffer.size() == other.sum_buffer.size());
}

Makespan& Makespan::operator=(const Makespan& other) noexcept
{
    *this = Makespan{other};
    Ensures(sum_buffer.size() == other.sum_buffer.size());
    return *this;
}

Makespan::Makespan(Makespan&& other) noexcept = default;
Makespan& Makespan::operator=(Makespan&& other) noexcept
{
    if (&other == this) return *this;

    params = std::move(other.params);
    sum_buffer = std::move(other.sum_buffer);
    task_done = other.task_done;
    work_done = other.work_done;

    Ensures(!sum_buffer.empty());
    return *this;
}

Makespan::~Makespan() noexcept = default;

float Makespan::operator()(State state) noexcept
{
    Expects(!state.empty());
    Expects(state.size() == task_done.size());
    Expects(!sum_buffer.empty());
    Expects(task_done.data() == sum_buffer.data());

    ranges::fill(sum_buffer, 0.F);
    for (auto [task_id, agent_id] : state) {
        const auto done = std::max(
                              dependencies_done(task_id),
                              work_done[agent_id])
            + task_duration(task_id, agent_id);
        work_done[agent_id] = task_done[task_id] = done;
    }
    return ranges::max(work_done);
}

[[nodiscard]] float
Makespan::dependencies_done(int16 task_id) const noexcept
{
    Expects(task_id >= 0);

    using ranges::views::transform;
    const auto deps
        = params->dependencies[static_cast<gsl::index>(task_id)];
    if (deps.empty()) return 0.F;
    return ranges::max(deps | transform([&](const auto& dep_id) {
                           return task_done[dep_id];
                       }));
}

[[nodiscard]] float
Makespan::task_duration(int16 task_id, int16 agent_id) const noexcept
{
    Expects(task_id >= 0);
    Expects(agent_id >= 0);
    return params->task_duration[static_cast<gsl::index>(task_id)]
        / params
              ->agent_performance[static_cast<gsl::index>(agent_id)];
}

void Mutator::try_swap(MutState state) const noexcept
{
    Expects(!state.empty());
    if (state.size() == 1) return;
    const auto swap_index = 1 + random->uniform_int(state.size() - 2);
    auto& task_a = state[swap_index].task_id;
    auto& task_b = state[swap_index - 1].task_id;
    if (!is_swappable(task_a, task_b)) return;
    std::swap(task_a, task_b);
}

[[nodiscard]] bool
Mutator::is_swappable(int16 task, int16 predecessor) const noexcept
{
    Expects(task >= 0);
    Expects(
        static_cast<gsl::index>(task) < params->dependencies.size());
    Expects(predecessor >= 0);
    Expects(
        static_cast<gsl::index>(predecessor)
        < params->dependencies.size());
    Expects(task != predecessor);
    return !ranges::binary_search(
        params->dependencies[static_cast<gsl::index>(task)],
        predecessor);
}

Mutator::Mutator(const ScheduleParams& params, RandomUtils& random)
    : params{&params}
    , random{&random}
{
}

void Mutator::operator()(MutState state) const noexcept
{
    Expects(!state.empty());
    try_swap(state);
    update_agent(state);
}

void Mutator::update_agent(MutState state) const noexcept
{
    Expects(!state.empty());
    Expects(
        static_cast<gsl::index>(state.size())
        == params->available_agents.size());
    const auto task_index = random->uniform_int(state.size() - 1);
    const auto task_id
        = static_cast<gsl::index>(state[task_index].task_id);
    const auto new_agent_id = random->uniform_int(
        params->available_agents[task_id].size() - 1);
    state[task_index].agent_id = new_agent_id;
}
} // namespace angonoka::stun
