#include "stun_dag/stochastic_tunneling.h"
#include "stun_dag/random_utils.h"
#include "stun_dag/temperature.h"
#include "stun_dag/utils.h"
#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

namespace {
using namespace angonoka::stun_dag;

struct RandomUtilsMock final : RandomUtilsStub {
    MAKE_MOCK0(uniform_01, float(), noexcept override);
    MAKE_MOCK1(
        uniform_int,
        int16(std::int16_t max),
        noexcept override);
};
} // namespace

TEST_CASE("Stochastic tunneling")
{
    using namespace angonoka::stun_dag;

    RandomUtilsMock random_utils;
    // TODO: WIP
    // const auto r = stochastic_tunneling(State{},STUNOptions{});
}
