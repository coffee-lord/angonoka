#pragma once

#include "configuration.h"
#include "options.h"
#include <CLI/App.hpp>

namespace angonoka::cli {
/**
    Parse the configuration YAML.

    @param options CLI options

    @return Tasks and agents.
*/
Configuration parse_config(const Options& options);

/**
    Run the prediction algorithm on given configuration.

    @param config   Agent and tasks configuration
    @param options  CLI options
*/
void run_prediction(
    const Configuration& config,
    const Options& options);

/**
    Parse optimization-related CLI parameters.

    TODO: Doc, test

    @param cli      App instance
    @param params   Optimization parameters to be set
*/
void parse_opt_params(
    const OptParams& in_params,
    OptimizationParameters& params);
} // namespace angonoka::cli
