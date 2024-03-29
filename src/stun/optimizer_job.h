#pragma once

#include "random_utils.h"
#include "schedule.h"
#include "schedule_params.h"
#include "stochastic_tunneling.h"
#include "temperature.h"
#include "utils.h"
#include <gsl/gsl-lite.hpp>

namespace angonoka::stun {
/**
    A single optimization job, meant to be launched in a thread pool.

    Optimizer starts many OptimizerJobs in parallel,
    pruning unseccessful jobs as needed.
*/
class OptimizerJob {
public:
    /**
        OptimizerJob options.

        @var params         Schedule parameters
        @var random         Random utils.
        @var batch_size     Number of STUN iterations in each update
        @var beta_scale     Temperature parameter's inertia
        @var stun_window    Temperature adjustment window
        @var gamma          Domain-specific parameter for STUN
        @var restart_period Temperature volatility period
    */
    struct Options {
        gsl::not_null<const ScheduleParams*> params;
        gsl::not_null<RandomUtils*> random;
        int32 batch_size;
        float beta_scale;
        int32 stun_window;
        float gamma;
        int32 restart_period;
    };

    /**
        OptimizerJob parameters.

        @var params Schedule parameters
        @var random Random utils.
    */
    struct Params {
        gsl::not_null<const ScheduleParams*> params;
        gsl::not_null<RandomUtils*> random;
    };

    /**
        Constructor.

        @param options Job tunables
    */
    explicit OptimizerJob(const Options& options);

    /**
        Run stochastic tunneling optimization batch.

        Does batch_size number of iterations.
    */
    void update() noexcept;

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

    /**
        Get current parameters.

        @return Parameters.
    */
    [[nodiscard]] Params params() const;

    /**
        Set parameters.

        @param params Parameters.
    */
    void params(const Params& params);

    OptimizerJob(const OptimizerJob& other);
    OptimizerJob(OptimizerJob&& other) noexcept;
    OptimizerJob& operator=(const OptimizerJob& other);
    OptimizerJob& operator=(OptimizerJob&& other) noexcept;
    ~OptimizerJob() noexcept;

private:
    int32 batch_size;
    Mutator mutator;
    Makespan makespan;
    Temperature temperature;
    StochasticTunneling stun;
};
} // namespace angonoka::stun
