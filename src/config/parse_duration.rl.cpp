#include "config/load.h"
#include "exceptions.h"
#include <gsl/gsl-lite.hpp>

namespace {
// clang-format off
%%{
machine duration_parser;

action add_seconds { v += seconds{std::atoi(n)}; }
action add_minutes { v += minutes{std::atoi(n)}; }
action add_hours { v += hours{std::atoi(n)}; }
action add_days { v += days{std::atoi(n)}; }
action add_weeks { v += weeks{std::atoi(n)}; }
action add_months { v += months{std::atoi(n)}; }
action num { n = fpc; }

number = digit* >num;

seconds_long = space ('sec' | 'second') 's'?;
seconds = ('s' | seconds_long) %add_seconds;

minutes_long = space ('min' | 'minute') 's'?;
minutes = ('m' | minutes_long) %add_minutes;

hours_long = space 'hour' 's'?;
hours = ('h' | hours_long) %add_hours;

days_long = space 'day' 's'?;
days = ('d' | days_long) %add_days;

weeks_long = space 'week' 's'?;
weeks = ('w' | weeks_long) %add_weeks;

months = (space 'month' 's'?) %add_months;

duration = number space* (seconds | minutes | hours | days | weeks | months);
and = space+ 'and' space+;
main := duration ((and | space*) duration)*;

write data noentry noerror;
}%%
// clang-format on
} // namespace

namespace angonoka::detail {
std::chrono::seconds parse_duration(std::string_view text)
{
    using namespace std::chrono;
    if (text.empty()) throw InvalidDuration{};
    int cs{0};
    gsl::zstring p = const_cast<char*>(text.data());
    gsl::czstring pe = text.end();
    gsl::czstring eof = pe;
    seconds v{0};
    gsl::zstring n;
    // clang-format off
    %% write init;
    %% write exec;
    // clang-format on
    if (cs < duration_parser_first_final) throw InvalidDuration{};
    return v;
}
} // namespace angonoka::detail