#pragma once
#include <string>
#include <vector>
#include <optional>
#include <chrono>

namespace app {

    enum class IngestMode { Csv, Simulator };

    struct Options {
        std::vector<std::string> inputs;     // CSV files
        std::vector<int> zones;              // zone_id filters
        std::optional<std::chrono::system_clock::time_point> from;
        std::optional<std::chrono::system_clock::time_point> to;
        IngestMode mode = IngestMode::Csv;
        std::size_t batch_size = 1000;
        std::size_t reserve_rows = 0;
        int window_minutes = 5;
        int sim_seed = 1234;
        int sim_hours = 24;
        bool run_patterns{false};
        int  patterns_month{0};
        std::optional<std::string> output_json;
    };

    Options parse_args(int argc, char** argv);

} // namespace app
