#pragma once

#include "exp_curve_fitter.h"
#include "random_utils.h"
#include "schedule.h"
#include "schedule_params.h"
#include "stochastic_tunneling.h"
#include "temperature.h"
#include "utils.h"
#include <gsl/gsl-lite.hpp>

namespace angonoka::stun {
/**
    How many stochastic tunneling iterations to go through
    during each update.
*/
enum class BatchSize : std::int_fast32_t;

/**
    How many iterations without improvement before
    considering optimization complete.
*/
enum class MaxIdleIters : std::int_fast32_t;

/**
    Optimization algorithm based on stochastic tunneling.

    This is the primary facade for doing stochastic tunneling
    optimization.
*/
class Optimizer {
public:
    /**
        Constructor.

        @param params           Scheduling parameters
        @param batch_size       Number of iterations per update
        @param max_idle_iters   Stopping condition
    */
    Optimizer(
        const ScheduleParams& params,
        BatchSize batch_size,
        MaxIdleIters max_idle_iters);

    /**
        Run stochastic tunneling optimization batch.

        Does batch_size number of iterations and adjusts the estimated
        progress accordingly.
    */
    void update() noexcept;

    /**
        Checks if the stopping condition has been met.

        @return True when further improvements are unlikely
    */
    [[nodiscard]] bool has_converged() const noexcept;

    /**
        Estimated optimization progress from 0.0 to 1.0.

        @return Progress from 0.0 to 1.0
    */
    [[nodiscard]] float estimated_progress() const noexcept;

    /**
        The best schedule so far.

        @return A schedule.
    */
    [[nodiscard]] Schedule schedule() const;

    /**
        The best makespan so far.

        @return Makespan.
    */
    [[nodiscard]] float normalized_makespan() const;

    /**
        Reset the optimization to initial state.
    */
    void reset();

    Optimizer(const Optimizer& other);
    Optimizer(Optimizer&& other) noexcept;
    Optimizer& operator=(const Optimizer& other);
    Optimizer& operator=(Optimizer&& other) noexcept;
    ~Optimizer() noexcept;

private:
    struct Impl;

    static constexpr auto beta_scale = 1e-4F;
    static constexpr auto stun_window = 10000;
    static constexpr auto gamma = .5F;
    static constexpr auto restart_period = 1 << 20;
    static constexpr auto initial_beta = 1.0F;

    gsl::not_null<const ScheduleParams*> params;
    int16 batch_size;
    int16 max_idle_iters;
    int16 idle_iters{0};
    int16 epochs{0};
    float last_progress{0.F};
    float last_makespan{0.F};
    RandomUtils random_utils;
    Mutator mutator{*params, random_utils};
    Makespan makespan{*params};
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wbraced-scalar-init"
    Temperature temperature{
        Beta{initial_beta},
        BetaScale{beta_scale},
        StunWindow{stun_window},
        RestartPeriod{restart_period}};
    StochasticTunneling stun{
        {.mutator{&mutator},
         .random{&random_utils},
         .makespan{&makespan},
         .temp{&temperature},
         .gamma{gamma}},
        initial_schedule(*params)};
#pragma clang diagnostic pop
    ExpCurveFitter exp_curve;
};
} // namespace angonoka::stun
