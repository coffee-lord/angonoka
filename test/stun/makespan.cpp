#include "stun/schedule_params.h"
#include "stun/utils.h"
#include <catch2/catch.hpp>
#include <range/v3/action/push_back.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/chunk.hpp>

namespace {
using namespace angonoka::stun;
ScheduleParams make_test_schedule_params()
{
    using ranges::to;
    using ranges::actions::push_back;
    using ranges::views::chunk;

    ScheduleParams params;
    params.agent_performance = {1.F, 2.F, 3.F};
    params.task_duration = {1.F, 2.F, 3.F};
    std::vector<int16> available_agents_data = {0, 1, 0, 1, 0, 1};
    std::vector<span<int16>> available_agents = available_agents_data
        | chunk(2) | to<decltype(available_agents)>();
    params.available_agents
        = {std::move(available_agents_data),
           std::move(available_agents)};

    std::vector<int16> dependencies_data = {0, 1};
    std::vector<span<int16>> dependencies{{}};
    push_back(dependencies, dependencies_data | chunk(1));
    params.dependencies
        = {std::move(dependencies_data), std::move(dependencies)};
    return params;
}
} // namespace

"Makespan type traits"_test = [] {
{
    using angonoka::stun::Makespan;
    expect(std::is_nothrow_destructible_v<Makespan>);
    expect(!std::is_default_constructible_v<Makespan>);
    expect(std::is_copy_constructible_v<Makespan>);
    expect(std::is_copy_assignable_v<Makespan>);
    expect(std::is_nothrow_move_constructible_v<Makespan>);
    expect(std::is_nothrow_move_assignable_v<Makespan>);
}

"Makespan special member functions"_test = [] {
{
    using namespace angonoka::stun;

    const auto params = make_test_schedule_params();
    const std::vector<StateItem> state{{0, 0}, {1, 1}, {2, 2}};
    Makespan makespan{params};

    expect(makespan(state) == 3._d);

    auto params2 = make_test_schedule_params();
    params2.agent_performance.resize(2);
    params2.task_duration.resize(2);
    const std::vector<StateItem> state2{{0, 0}, {1, 1}};
    Makespan makespan2{params2};

    expect(makespan2(state2) == 2._d);

    "Copy assignment"_test = [] {
    {
        makespan2 = makespan;
        expect(makespan(state) == 3._d);
        expect(makespan2(state) == 3._d);
    }

    "Move assignment"_test = [] {
    {
        makespan2 = std::move(makespan);
        expect(makespan2(state) == 3._d);
    }

    "Copy ctor"_test = [] {
    {
        Makespan makespan3{makespan2};
        expect(makespan2(state2) == 2._d);
        expect(makespan3(state2) == 2._d);
    }

    "Move ctor"_test = [] {
    {
        Makespan makespan4{std::move(makespan2)};
        expect(makespan4(state2) == 2._d);
    }

    "Self copy"_test = [] {
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
        makespan = makespan;
#pragma clang diagnostic pop

        expect(makespan(state) == 3._d);
    }

    "Self move"_test = [] {
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
        makespan = std::move(makespan);
#pragma clang diagnostic pop

        expect(makespan(state) == 3._d);
    }
}

"Makespan estimation"_test = [] {
{
    const auto params = make_test_schedule_params();
    Makespan makespan{params};
    const std::vector<StateItem> state{{0, 0}, {1, 1}, {2, 2}};
    expect(makespan(state) == 3._d);
}
