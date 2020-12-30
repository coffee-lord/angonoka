#include "stochastic_tunneling.h"
#include "random_utils.h"
#include "schedule_info.h"
#include "temperature.h"
#include "utils.h"
#include <range/v3/algorithm/copy.hpp>

namespace {
using namespace angonoka::stun_dag;

#ifdef UNIT_TEST
constexpr std::uint_fast64_t max_iterations = 2;
#else // UNIT_TEST
constexpr std::uint_fast64_t max_iterations = 1'000'000;
#endif // UNIT_TEST

using index = MutState::index_type;

/**
    Adjusts the energy to be within 0.0 to 1.0 range.

    See https://arxiv.org/pdf/physics/9903008.pdf for more details.

    @param lowest_e Lowest energy
    @param energy   Target energy
    @param gamma    Tunneling parameter

    @return STUN-adjusted energy
*/
float stun(float lowest_e, float energy, float gamma) noexcept
{
    Expects(lowest_e >= 0.F);
    Expects(energy >= lowest_e);

    const auto result = 1.F - std::exp(-gamma * (energy - lowest_e));

    Ensures(result >= 0.F && result <= 1.F);
    return result;
}

/**
    Stochastic tunneling function object.
*/
struct StochasticTunnelingOp {
    gsl::not_null<const MutatorT*> mutator;
    gsl::not_null<RandomUtilsT*> random;
    gsl::not_null<MakespanT*> makespan;
    gsl::not_null<TemperatureT*> temp;
    std::uint_fast64_t iteration{0};
    std::vector<StateItem> state_buffer;
    MutState best_state;
    MutState current_state;
    MutState target_state;

    float current_e;
    float lowest_e;
    float target_e;

    float gamma; // TODO: Should be a constant
    float current_s;
    float target_s;

    /**
        Creates a new state from the current state.
    */
    void get_new_neighbor() noexcept
    {
        Expects(!current_state.empty());
        Expects(!target_state.empty());

        ranges::copy(current_state, target_state.begin());
        (*mutator)(target_state);
        target_e = (*makespan)(target_state);

        Ensures(target_e >= 0.F);
    }

    /**
        Updates the lowest energy and best state if the
        target state is better.
    */
    bool neighbor_is_better() noexcept
    {
        Expects(target_e >= 0.F);
        Expects(current_e >= 0.F);
        Expects(lowest_e >= 0.F);
        Expects(current_e >= lowest_e);
        Expects(!target_state.empty());
        Expects(!current_state.empty());
        Expects(!best_state.empty());

        if (target_e < current_e) {
            if (target_e < lowest_e) {
                lowest_e = target_e;
                ranges::copy(target_state, best_state.begin());
                current_s = stun(lowest_e, current_e, gamma);
            }
            std::swap(current_state, target_state);
            current_e = target_e;
            return true;
        }
        return false;
    }

    /**
        Perform Monte Carlo sampling on the STUN-adjusted energy.
    */
    void perform_stun() noexcept
    {
        Expects(target_e >= 0.F);
        Expects(lowest_e >= 0.F);
        Expects(current_s >= 0.F);
        Expects(!target_state.empty());
        Expects(!current_state.empty());

        target_s = stun(lowest_e, target_e, gamma);
        const auto delta_s = target_s - current_s;
        const auto pr = std::min(1.F, std::exp(-*temp * delta_s));
        if (pr >= random->uniform_01()) {
            std::swap(current_state, target_state);
            current_e = target_e;
            current_s = target_s;
            temp->update(
                target_s,
                static_cast<float>(iteration) / max_iterations);
        }

        Ensures(target_s >= 0.F);
        Ensures(current_s >= 0.F);
    }

    /**
        Run stochastic tunneling.
    */
    void run() noexcept
    {
        for (iteration = 0; iteration < max_iterations; ++iteration) {
            get_new_neighbor();
            if (neighbor_is_better()) continue;
            perform_stun();
        }
    }

    /**
        Init energies and STUN-adjusted energies.
    */
    void init_energies()
    {
        Expects(!current_state.empty());

        current_e = (*makespan)(current_state);
        lowest_e = current_e;
        current_s = stun(lowest_e, current_e, gamma);

        Ensures(current_e >= 0.F);
        Ensures(lowest_e >= 0.F);
        Ensures(current_s >= 0.F && current_s <= 1.F);
    }

    /**
        Recreate state spans over the state buffer object.

        @param state_size Size of the state
    */
    void prepare_state_spans(index state_size)
    {
        Expects(state_size > 0);

        state_buffer.resize(static_cast<gsl::index>(state_size) * 3);
        auto* data = state_buffer.data();
        const auto next = [&] {
            return std::exchange(data, std::next(data, state_size));
        };
        best_state = {next(), state_size};
        current_state = {next(), state_size};
        target_state = {next(), state_size};

        Ensures(current_state.size() == state_size);
        Ensures(target_state.size() == state_size);
        Ensures(best_state.size() == state_size);
    }

    /**
        Init all states with the source state.

        @param source_state Source state
    */
    void init_states(State source_state) const
    {
        Expects(source_state.size() == best_state.size());
        Expects(source_state.size() == current_state.size());
        Expects(!source_state.empty());

        ranges::copy(source_state, best_state.begin());
        ranges::copy(source_state, current_state.begin());
    }

    /**
        Init states and run stochastic tunneling optimization.

        @param state Initial state

        @return An instance of STUNResult
    */
    [[nodiscard]] STUNResult operator()(State state)
    {
        prepare_state_spans(state.size());
        init_states(state);
        init_energies();
        run();
        state_buffer.resize(static_cast<gsl::index>(state.size()));
        return {std::move(state_buffer), lowest_e, *temp};
    }
};
} // namespace

namespace angonoka::stun_dag {
STUNResult
stochastic_tunneling(State state, const STUNOptions& options) noexcept
{
    Expects(!state.empty());
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbraced-scalar-init"
    StochasticTunnelingOp op{
        .mutator{options.mutator},
        .random{options.random},
        .makespan{options.makespan},
        .temp{options.temp},
        .gamma{options.gamma}};
#pragma clang diagnostic pop
    return op(state);
}
} // namespace angonoka::stun_dag
