#include "stun/random_utils.h"
#include "stun/schedule_params.h"
#include "stun/utils.h"
#include <boost/ut.hpp>
#include <range/v3/action/push_back.hpp>
#include <range/v3/view/chunk.hpp>

using namespace boost::ut;

suite mutate = [] {
    "Mutate schedule"_test = [] {
        using namespace angonoka::stun;

        ScheduleParams params;
        params.agent_performance = {1.F, 2.F, 3.F};
        params.task_duration = {1.F, 2.F, 3.F};
        {
            std::vector<int16> available_agents_data = {0, 1, 2};
            std::vector<span<int16>> available_agents
                = {available_agents_data,
                   available_agents_data,
                   available_agents_data};
            params.available_agents
                = {std::move(available_agents_data),
                   std::move(available_agents)};
        }
        params.dependencies
            = {std::vector<int16>{},
               std::vector<span<int16>>{{}, {}, {}}};

        "no dependencies"_test = [&] {
            RandomUtils random{0};

            std::vector<ScheduleItem> schedule{
                {0, 0},
                {1, 1},
                {2, 2}};

            Mutator mut{params, random};
            mut(schedule);

            expect(
                schedule
                == std::vector<ScheduleItem>{{0, 0}, {2, 1}, {1, 2}});

            for (int i{0}; i < 100; ++i) mut(schedule);

            expect(
                schedule
                == std::vector<ScheduleItem>{{1, 2}, {0, 0}, {2, 2}});
        };

        "with dependencies"_test = [&] {
            using ranges::actions::push_back;
            using ranges::views::chunk;
            std::vector<int16> dependencies_data = {0, 1};
            std::vector<span<int16>> dependencies{{}};
            push_back(dependencies, dependencies_data | chunk(1));
            params.dependencies
                = {std::move(dependencies_data),
                   std::move(dependencies)};

            RandomUtils random{1};

            std::vector<ScheduleItem> schedule{
                {0, 0},
                {1, 1},
                {2, 2}};

            Mutator mut{params, random};
            mut(schedule);

            expect(
                schedule
                == std::vector<ScheduleItem>{{0, 0}, {1, 0}, {2, 2}});

            for (int i{0}; i < 100; ++i) mut(schedule);

            expect(
                schedule
                == std::vector<ScheduleItem>{{0, 0}, {1, 0}, {2, 1}});
        };

        "single task"_test = [&] {
            params.dependencies
                = {std::vector<int16>{},
                   std::vector<span<int16>>{{}}};

            {
                std::vector<int16> available_agents_data = {0, 1, 2};
                std::vector<span<int16>> available_agents
                    = {available_agents_data};
                params.available_agents
                    = {std::move(available_agents_data),
                       std::move(available_agents)};
            }

            RandomUtils random{0};

            std::vector<ScheduleItem> schedule{{0, 0}};

            Mutator mut{params, random};
            mut(schedule);

            expect(schedule == std::vector<ScheduleItem>{{0, 2}});

            for (int i{0}; i < 100; ++i) mut(schedule);

            expect(schedule == std::vector<ScheduleItem>{{0, 0}});
        };
    };

    "Mutator type traits"_test = [] {
        using angonoka::stun::Mutator;
        expect(std::is_nothrow_destructible_v<Mutator>);
        expect(!std::is_default_constructible_v<Mutator>);
        expect(std::is_copy_constructible_v<Mutator>);
        expect(std::is_copy_assignable_v<Mutator>);
        expect(std::is_nothrow_move_constructible_v<Mutator>);
        expect(std::is_nothrow_move_assignable_v<Mutator>);
    };
};
