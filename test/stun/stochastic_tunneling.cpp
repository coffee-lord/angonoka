#include "stun/stochastic_tunneling.h"
#include "stun/random_utils.h"
#include "stun/schedule_params.h"
#include "stun/temperature.h"
#include "stun/utils.h"
#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

namespace {
using namespace angonoka::stun;

struct RandomUtilsMock final : RandomUtilsStub {
    MAKE_MOCK0(uniform_01, float(), noexcept override);
    MAKE_MOCK1(
        uniform_int,
        int16(std::int16_t max),
        noexcept override);
};

struct MakespanMock final : MakespanStub {
    float operator()(State state) noexcept override
    {
        return call(state);
    }
    MAKE_MOCK1(call, float(State state), noexcept);
};

struct MutatorMock final : MutatorStub {
    void operator()(MutState state) const noexcept override
    {
        return call(state);
    }
    MAKE_MOCK1(call, void(MutState state), const noexcept);
};

struct TemperatureMock final : TemperatureStub {
    operator float() noexcept override { return to_float(); }
    MAKE_MOCK0(to_float, float(), noexcept);
    MAKE_MOCK1(update, void(float stun), noexcept override);
    MAKE_MOCK0(average_stun, float(), const noexcept override);
};
} // namespace

"StochasticTunneling type traits"_test = [] {
{
    using angonoka::stun::StochasticTunneling;
    expect(
        std::is_nothrow_destructible_v<StochasticTunneling>);
    expect(
        !std::is_default_constructible_v<StochasticTunneling>);
    expect(std::is_copy_constructible_v<StochasticTunneling>);
    expect(std::is_copy_assignable_v<StochasticTunneling>);
    expect(
        std::is_nothrow_move_constructible_v<StochasticTunneling>);
    expect(
        std::is_nothrow_move_assignable_v<StochasticTunneling>);
}

"Stochastic tunneling"_test = [] {
{
    using namespace angonoka::stun;
    using trompeloeil::_;

    RandomUtilsMock random_utils;
    MakespanMock makespan;
    TemperatureMock temperature;
    MutatorMock mutator;
    std::vector<StateItem> state(3);

    trompeloeil::sequence seq;

    expect_CALL(makespan, call(state)).RETURN(1.F).IN_SEQUENCE(seq);
    expect_CALL(mutator, call(_)).IN_SEQUENCE(seq);
    expect_CALL(makespan, call(_)).RETURN(1.F).IN_SEQUENCE(seq);
    expect_CALL(temperature, to_float())
        .RETURN(.5F)
        .IN_SEQUENCE(seq);
    expect_CALL(random_utils, uniform_01())
        .RETURN(.1F)
        .IN_SEQUENCE(seq);
    expect_CALL(temperature, update(_)).IN_SEQUENCE(seq);

    expect_CALL(mutator, call(_)).IN_SEQUENCE(seq);
    expect_CALL(makespan, call(_)).RETURN(.1F).IN_SEQUENCE(seq);

    "Simple"_test = [] {
    {
        StochasticTunneling stun{
            {.mutator{&mutator},
             .random{&random_utils},
             .makespan{&makespan},
             .temp{&temperature},
             .gamma{.5F}},
            state};
        for (int i{0}; i < 2; ++i) stun.update();

        expect(stun.energy() == .1_d);
        expect(stun.state().size() == 3);
    }

    "Two phase construction"_test = [] {
    {
        StochasticTunneling stun{
            {.mutator{&mutator},
             .random{&random_utils},
             .makespan{&makespan},
             .temp{&temperature},
             .gamma{.5F}}};
        stun.reset(state);
        for (int i{0}; i < 2; ++i) stun.update();

        expect(stun.energy() == .1_d);
        expect(stun.state().size() == 3);
    }
}

"StochasticTunneling options"_test = [] {
{
    using namespace angonoka::stun;
    using trompeloeil::_;

    RandomUtilsMock random_utils;
    MakespanMock makespan;
    TemperatureMock temperature;
    MutatorMock mutator;
    std::vector<StateItem> state{{0, 0}, {1, 1}, {2, 2}};

    ALLOW_CALL(makespan, call(_)).RETURN(1.F);

    StochasticTunneling::Options options{
        .mutator{&mutator},
        .random{&random_utils},
        .makespan{&makespan},
        .temp{&temperature},
        .gamma{.5F}};

    StochasticTunneling stun{options, state};

    expect(stun.options().makespan == &makespan);

    MakespanMock makespan2;

    options.makespan = &makespan2;
    stun.options(options);

    expect(stun.options().makespan == &makespan2);
}

"StochasticTunneling special member functions"_test = [] {
{
    using namespace angonoka::stun;
    using trompeloeil::_;

    RandomUtilsMock random_utils;
    MakespanMock makespan;
    TemperatureMock temperature;
    MutatorMock mutator;
    std::vector<StateItem> state{{0, 0}, {1, 1}, {2, 2}};
    std::vector<StateItem> state2{{3, 3}, {4, 4}, {5, 5}};

    ALLOW_CALL(makespan, call(_)).RETURN(1.F);

    StochasticTunneling::Options options{
        .mutator{&mutator},
        .random{&random_utils},
        .makespan{&makespan},
        .temp{&temperature},
        .gamma{.5F}};

    StochasticTunneling stun{options, state};

    "Copy assignment"_test = [] {
    {
        StochasticTunneling stun2{options, state2};
        stun = stun2;

        expect(stun.state()[0].agent_id == 3);
    }

    "Move assignment"_test = [] {
    {
        StochasticTunneling stun2{options, state2};
        stun = std::move(stun2);

        expect(stun.state()[0].agent_id == 3);
    }

    "Copy ctor"_test = [] {
    {
        StochasticTunneling stun2{stun};

        expect(stun2.state()[1].agent_id == 1);
    }

    "Move ctor"_test = [] {
    {
        StochasticTunneling stun2{std::move(stun)};

        expect(stun2.state()[1].agent_id == 1);
    }

    "Self copy"_test = [] {
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
        stun = stun;
#pragma clang diagnostic pop
        expect(stun.state()[1].agent_id == 1);
    }

    "Self move"_test = [] {
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
        stun = std::move(stun);
#pragma clang diagnostic pop
        expect(stun.state()[1].agent_id == 1);
    }
}
