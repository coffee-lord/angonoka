#include "cli/cli.h"
#include "cli/schedule.h"
#include "config.h"
#include <CLI/CLI.hpp>
#include <bit>
#include <fmt/printf.h>

namespace {
using namespace angonoka::cli;

/**
    CLI11 validator function for checking if a number
    is a power of 2.

    @param v CLI parameter value

    @return Error message in case of an error,
        empty otherwise.
*/
std::string power_of_2_validator(const std::string& v)
{
    try {
        const auto i = static_cast<unsigned int>(std::stoi(v));
        if (!std::has_single_bit(i)) return "Must be a power of 2";
    } catch (...) {
        return "Must be a number";
    }
    return {};
}

/**
    Add CLI11 options related to schedule optimization.

    @param cli      Instance of CLI::App
    @param params   CLI optimization parameters
*/
void optimization_options(CLI::App& cli, OptParams& params)
{
    using Params = angonoka::OptimizationParameters;

    cli.add_option(
           "--batch-size",
           params.batch_size,
           "Optimization batch size")
        ->default_val(Params::default_batch_size)
        ->check(CLI::PositiveNumber);
    cli.add_option(
           "--max-idle-iters",
           params.max_idle_iters,
           "Optimization halting condition")
        ->default_val(Params::default_max_idle_iters)
        ->check(CLI::PositiveNumber);
    cli.add_option(
           "--beta-scale",
           params.beta_scale,
           "Optimization temperature parameter inertia")
        ->default_val(Params::default_beta_scale)
        ->check(CLI::PositiveNumber);
    cli.add_option(
           "--stun-window",
           params.stun_window,
           "Optimization temperature adjustment window")
        ->default_val(Params::default_stun_window)
        ->check(CLI::PositiveNumber);
    cli.add_option(
           "--gamma",
           params.gamma,
           "Optimization STUN parameter")
        ->default_val(Params::default_gamma)
        ->check(CLI::PositiveNumber);
    cli.add_option(
           "--restart-period",
           params.restart_period,
           "Optimization temperature volatility period")
        ->default_val(Params::default_restart_period)
        ->check(CLI::PositiveNumber)
        ->check(CLI::Validator(power_of_2_validator, "POWER_OF_2"));
}

/**
    Add CLI11 options that apply to all CLI modes.

    @param cli      Instance of CLI::App
    @param options  CLI options
*/
void common_options(CLI::App& cli, Options& options)
{
    cli.set_version_flag(
        "--version",
        ANGONOKA_VERSION,
        "Display program version information and exit");
    cli.add_flag(
        "--color,!--no-color",
        options.color,
        "Force colored output");
    cli.add_flag("-q,--quiet", options.quiet, "Give less output");
    cli.add_flag("-v,--verbose", options.verbose, "Give more output");
    cli.add_flag(
        "--log-optimization-progress",
        options.log_optimization,
        "Log optimization progress to optimization.csv");
}

/**
    Add a subcommand for generating a schedule JSON.

    @param cli              Instance of CLI::App
    @param options          CLI options
    @param default_group    Default CLI parameters

    @return The schedule subcommand
*/
auto schedule_subcommand(CLI::App& cli, Options& options)
{
    auto* schedule_cmd = cli.add_subcommand(
        "schedule",
        "Output the schedule in JSON format.");
    schedule_cmd
        ->add_option("-o,--output", "Output the schedule to a file")
        ->default_str("schedule.json");
    schedule_cmd->add_option("input file", options.filename)
        ->required()
        ->check(CLI::ExistingFile);
    return schedule_cmd;
}

/**
    CLI11 parameter group for when the CLI is invoked
    without any subcommands.

    @param cli      Instance of CLI::App
    @param options  CLI options

    @return Default parameter group
*/
auto default_group(CLI::App& cli, Options& options)
{
    auto* group = cli.add_option_group("Default");
    group->add_option("input file", options.filename)
        ->required()
        ->check(CLI::ExistingFile);
    group->add_option("-o,--output", "Output the histogram to a file")
        ->default_str("time_estimation.json");
    group
        ->add_option(
            "--histogram-bin-size",
            options.bin_size,
            "Histogram bin size in seconds")
        ->check(CLI::PositiveNumber);
    return group;
}
} // namespace

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, char** argv)
{
    using namespace fmt::literals;
    using namespace angonoka::cli;

    register_abort_handlers();

    Options options;
    CLI::App cli{
        "Angonoka is a time estimation software based on statistical "
        "modeling.\n",
        ANGONOKA_NAME};
    common_options(cli, options);
    optimization_options(cli, options.opt_params);
    cli.require_subcommand(-1);
    auto* default_grp = default_group(cli, options);
    auto* schedule_cmd = schedule_subcommand(cli, options);
    default_grp->excludes(schedule_cmd);
    schedule_cmd->excludes(default_grp);

    try {
        CLI11_PARSE(cli, argc, argv);
        auto config = parse_config(options);
        parse_opt_params(options.opt_params, config.opt_params);

        // schedule subcommand
        if (schedule_cmd->parsed()) {
            options.output = (*schedule_cmd)["-o"]->as<std::string>();
            const auto json = json_schedule(config, options);
            save_schedule_json(json, options);
            return EXIT_SUCCESS;
        }

        // no subcommand
        options.output = (*default_grp)["-o"]->as<std::string>();
        const auto json = run_prediction(config, options);
        save_prediction_json(json, options);
        return EXIT_SUCCESS;
    } catch (const UserError&) {
        return EXIT_FAILURE;
    }
}
