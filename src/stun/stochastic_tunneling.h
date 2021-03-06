#pragma once

#include "common.h"
#include "detail.h"
#include <gsl/gsl-lite.hpp>
#include <vector>

namespace angonoka::stun {

struct ScheduleInfo;
#ifndef UNIT_TEST
using TemperatureT = class Temperature;
using MakespanT = class Makespan;
using RandomUtilsT = class RandomUtils;
using MutatorT = class Mutator;
#else // UNIT_TEST
using TemperatureT = struct TemperatureStub;
using MakespanT = struct MakespanStub;
using RandomUtilsT = struct RandomUtilsStub;
using MutatorT = struct MutatorStub;
#endif // UNIT_TEST

/**
    Stochastic tunneling algorithm.

    The internal state can be updated as many times as needed.
*/
class StochasticTunneling {
public:
    /**
        Result of a stochastic tunneling pass.

        @var state          State that had the lowest energy
        @var energy         Lowest energy achieved so far
        @var temperature    Final temperature
    */
    struct Result {
        std::vector<StateItem> state;
        float energy;
        float temperature;
    };

    /**
        STUN auxiliary data and utilities.

        @var mutator    Instance of Mutator
        @var random     Instance of RandomUtils
        @var makespan   Instance of Makespan
        @var temp       Instance of Temperature
        @var gamma      Tunneling parameter
    */
    struct Options {
        gsl::not_null<const MutatorT*> mutator;
        gsl::not_null<RandomUtilsT*> random;
        gsl::not_null<MakespanT*> makespan;
        gsl::not_null<TemperatureT*> temp;
        float gamma;
    };

    /**
        Default constructor.

        The object will be in an uninitialized state. User must call
        reset to set the initial schedule.

        @param options Instance of StochasticTunneling::Options
    */
    StochasticTunneling(const Options& options);

    /**
        Constructor.

        @param options  Instance of StochasticTunneling::Options
        @param state    Initial schedule
    */
    StochasticTunneling(const Options& options, State state);

    StochasticTunneling(const StochasticTunneling& other);
    StochasticTunneling(StochasticTunneling&& other) noexcept;
    StochasticTunneling& operator=(const StochasticTunneling& other);
    StochasticTunneling&
    operator=(StochasticTunneling&& other) noexcept;
    ~StochasticTunneling() noexcept;

    /**
        Reset stochastic tunneling algorithm to a new state.

        @param state Initial schedule
    */
    void reset(State state);

    /**
        Set stochastic tunneling options.

        @param options Instance of Options
    */
    void options(const Options& options);

    /**
        Get stochastic tunneling options.

        @return Stochastic tunneling options.
    */
    [[nodiscard]] Options options() const;

    /**
        Update the internal state according to stochastic
        tunneling algorithm.
    */
    void update() noexcept;

    /**
        The best schedule so far.

        @return A schedule.
    */
    [[nodiscard]] State state() const;

    /**
        The best makespan so far.

        @return Makespan.
    */
    [[nodiscard]] float energy() const;

private:
    using index = MutState::index_type;

    gsl::not_null<const MutatorT*> mutator;
    gsl::not_null<RandomUtilsT*> random;
    gsl::not_null<MakespanT*> makespan;
    gsl::not_null<TemperatureT*> temp;
    std::vector<StateItem> state_buffer;
    MutState best_state;
    MutState current_state;
    MutState target_state;

    float current_e;
    float lowest_e;
    float target_e;

    float gamma;
    float current_s;
    float target_s;

    /**
        Creates a new (mutated) state from the current state.
    */
    void get_new_neighbor() noexcept;

    /**
        Updates the lowest energy and best state if the
        target state is better.
    */
    bool neighbor_is_better() noexcept;

    /**
        Perform Monte Carlo sampling on the STUN-adjusted energy.
    */
    void perform_stun() noexcept;

    /**
        Init energies and STUN-adjusted energies.
    */
    void init_energies();

    /**
        Recreate state spans over the state buffer object.

        @param state_size Size of the state
    */
    void prepare_state_spans(index state_size);

    /**
        Init all states with the source state.

        @param source_state Source state
    */
    void init_states(State source_state) const;
};
} // namespace angonoka::stun
