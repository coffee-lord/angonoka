#include "run.h"
#include "config/load.h"
#include "stun/common.h"
#include "stun/random_utils.h"
#include "stun/schedule_params.h"
#include "stun/stochastic_tunneling.h"
#include "stun/temperature.h"
#include "stun/utils.h"
#include <fmt/ranges.h>
#include <gsl/gsl-lite.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/terminal_size.hpp>
#include <range/v3/action/insert.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/span.hpp>
#include <range/v3/view/transform.hpp>
#include <utility>
#include <vector>

namespace angonoka {
using namespace angonoka::stun;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wbraced-scalar-init"

/**
    Optimization algorithm based on stochastic tunneling.
*/
class Optimizer {
public:
    /**
        Constructor.

        @param params Scheduling parameters
    */
    Optimizer(const ScheduleParams& params)
        : params{&params}
    {
    }

    // TODO: doc, test, expects
    void update() noexcept { stun.update(); }

    // TODO: doc, test, expects
    // bool done() noexcept { return true; }

    /**
        The best schedule so far.

        TODO: doc, test, expects

        @return A schedule.
    */
    [[nodiscard]] State state() const { return stun.state(); }

    /**
        The best makespan so far.

        TODO: doc, test, expects

        @return Makespan.
    */
    [[nodiscard]] float energy() const { return stun.energy(); }

    /**
        TODO: doc, test, expects
    */
    void reset()
    {
        stun.reset(initial_state(*params));
        temperature
            = {Beta{initial_beta},
               BetaScale{beta_scale},
               StunWindow{stun_window},
               RestartPeriod{restart_period}};
    }

    Optimizer(const Optimizer& other)
        : params{other.params}
        , random_utils{other.random_utils}
        , mutator{other.mutator}
        , makespan{other.makespan}
        , temperature{other.temperature}
        , stun{other.stun}
    {
        stun.options(
            {.mutator{&mutator},
             .random{&random_utils},
             .makespan{&makespan},
             .temp{&temperature},
             .gamma{gamma}});
    }
    Optimizer(Optimizer&& other) noexcept = default;
    Optimizer& operator=(const Optimizer& other)
    {
        *this = Optimizer{other};
        return *this;
    }
    Optimizer& operator=(Optimizer&& other) noexcept = default;
    ~Optimizer() noexcept = default;

private:
    static constexpr auto beta_scale = 1e-4F;
    static constexpr auto stun_window = 10000;
    static constexpr auto gamma = .5F;
    static constexpr auto restart_period = 1 << 20;
    static constexpr auto initial_beta = 1.0F;

    gsl::not_null<const ScheduleParams*> params;
    RandomUtils random_utils;
    Mutator mutator{*params, random_utils};
    Makespan makespan{*params};
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
        initial_state(*params)};
};

/**
    Find the optimal schedule.

    TODO: test, expects

    @param params Instance of ScheduleParams

    @return Optimal schedule.
*/
std::vector<stun::StateItem>
optimize(const stun::ScheduleParams& params)
{
    Expects(!params.agent_performance.empty());
    Expects(!params.task_duration.empty());
    Expects(!params.available_agents.empty());
    Expects(
        params.available_agents.size()
        == params.task_duration.size());

    using namespace angonoka::stun;

    constexpr auto number_of_epochs = 10 * 10000;

    Optimizer optimizer{params};
    for (int i{0}; i < number_of_epochs; ++i) optimizer.update();
    // state = stun.state();

    // TODO: track progress via progress bar
    // TODO: return the Result, not just the state
    // TODO: How do we track progress? atomic int from 0 to 100?
    // Add a new class that encapsulates stun and adds
    // stopping condition and has an update method.
    return ranges::to<std::vector<StateItem>>(optimizer.state());
}
#pragma clang diagnostic pop

void run(std::string_view tasks_yml)
{
    using namespace angonoka::stun;

    // TODO: Handle YAML exceptions
    const auto config = load_file(tasks_yml);
    const auto schedule_params = to_schedule_params(config);
    const auto state = optimize(schedule_params);
    fmt::print("{}\n", state);
}
} // namespace angonoka
