#include "stun/optimizer.h"
#include "config/load.h"
#include <boost/ut.hpp>

using namespace boost::ut;

suite optimizer = [] {
    "Optimizer type traits"_test = [] {
        using angonoka::stun::Optimizer;
        expect(std::is_nothrow_destructible_v<Optimizer>);
        expect(!std::is_default_constructible_v<Optimizer>);
        expect(std::is_copy_constructible_v<Optimizer>);
        expect(std::is_copy_assignable_v<Optimizer>);
        expect(std::is_nothrow_move_constructible_v<Optimizer>);
        expect(std::is_nothrow_move_assignable_v<Optimizer>);
    };

    "basic Optimizer operations"_test = [] {
        using namespace angonoka::stun;

        // clang-format off
        constexpr auto text = 
            "agents:\n"
            "  Bob:\n"
            "  Jack:\n"
            "tasks:\n"
            "  - name: Task 1\n"
            "    duration: 1h\n"
            "  - name: Task 2\n"
            "    duration: 1h";
        // clang-format on
        const auto config = angonoka::load_text(text);

        const auto params = to_schedule_params(config);
        const auto schedule_params = to_schedule_params(config);
        Optimizer optimizer{params, BatchSize{5}, MaxIdleIters{10}};

        expect(optimizer.energy() == 2.F);
        expect(optimizer.estimated_progress() == 0.F);
        expect(optimizer.state()[1].agent_id == 0);

        optimizer.update();

        expect(optimizer.estimated_progress() == 0.F);

        while (!optimizer.has_converged()) optimizer.update();

        // Might be non-deterministic
        expect(optimizer.energy() == 1.F);
        expect(optimizer.estimated_progress() == 1.F);
        // Each task has a different agent
        expect(
            optimizer.state()[1].agent_id
            != optimizer.state()[0].agent_id);

        optimizer.reset();

        expect(optimizer.energy() == 2.F);
        expect(optimizer.estimated_progress() == 0.F);
    };

    "Optimizer special memeber functions"_test = [] {
        using namespace angonoka::stun;

        // clang-format off
        constexpr auto text = 
            "agents:\n"
            "  Bob:\n"
            "  Jack:\n"
            "tasks:\n"
            "  - name: Task 1\n"
            "    duration: 1h\n"
            "  - name: Task 2\n"
            "    duration: 1h";
        // clang-format on
        const auto config = angonoka::load_text(text);

        const auto params = to_schedule_params(config);
        const auto schedule_params = to_schedule_params(config);
        Optimizer optimizer{params, BatchSize{5}, MaxIdleIters{10}};

        // TODO: Fix lambda capture
        should("copy ctor") = [=]() mutable {
            Optimizer other{optimizer};

            expect(other.energy() == 2.F);

            while (!optimizer.has_converged()) optimizer.update();

            expect(other.energy() == 2.F);
        };

        should("copy assignment") = [=]() mutable {
            Optimizer other{params, BatchSize{5}, MaxIdleIters{10}};
            other = optimizer;

            expect(other.energy() == 2.F);

            while (!optimizer.has_converged()) optimizer.update();

            expect(other.energy() == 2.F);
        };

        should("move ctor") = [=]() mutable {
            Optimizer other{std::move(optimizer)};

            expect(other.energy() == 2.F);

            while (!other.has_converged()) other.update();

            expect(other.energy() == 1.F);
        };

        should("move assignment") = [=]() mutable {
            Optimizer other{params, BatchSize{5}, MaxIdleIters{10}};
            other = std::move(optimizer);

            expect(other.energy() == 2.F);

            while (!other.has_converged()) other.update();

            expect(other.energy() == 1.F);
        };

        should("self copy") = [=]() mutable {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
            optimizer = optimizer;
#pragma clang diagnostic pop

            expect(optimizer.energy() == 2.F);
        };

        should("self move") = [=]() mutable {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
            optimizer = std::move(optimizer);
#pragma clang diagnostic pop

            expect(optimizer.energy() == 2.F);
        };
    };
};
