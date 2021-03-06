#pragma once

#include "configuration.h"
#include <boost/variant2/variant.hpp>
#include <chrono>
#include <future>
#include <memory>
#include <readerwriterqueue/readerwriterqueue.h>
#include <tuple>

namespace angonoka {
template <typename... Ts>
using variant = boost::variant2::variant<Ts...>;

// TODO: doc, test, expects
struct Prediction {
};

template <typename... Ts>
using Queue = moodycamel::ReaderWriterQueue<Ts...>;

/**
    Events without attributes.
*/
enum class SimpleProgressEvent {
    ScheduleOptimizationStart,
    Finished
};

/**
    Emitted during the schedule optimization step.

    @var progress Optimization progress from 0.0 to 1.0
    @var makespan The best makespan so far
*/
struct ScheduleOptimizationEvent {
    float progress;
    std::chrono::seconds makespan;
};

/**
    Emitted when the schedule optimization is complete.

    @var makespan The makespan after optimization
*/
struct ScheduleOptimizationComplete {
    std::chrono::seconds makespan;
};

/**
    Prediction progress event.
*/
using ProgressEvent = variant<
    SimpleProgressEvent,
    ScheduleOptimizationEvent,
    ScheduleOptimizationComplete>;

/**
    Predict likelihood of a given system configuration.

    The event queue provides a way to monitor the progress
    of the prediction function.

    // TODO: test

    @param config Tasks, agents and other data.

    @return Prediction future and the event queue.
*/
std::tuple<
    std::future<Prediction>,
    std::shared_ptr<Queue<ProgressEvent>>>
predict(const Configuration& config);
} // namespace angonoka
