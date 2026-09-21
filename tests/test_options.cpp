#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/Options.hpp"

namespace {
app::Options parse(std::vector<std::string> args) {
    std::vector<char*> argv;
    argv.reserve(args.size());
    for (auto& arg : args) argv.push_back(arg.data());
    return app::parse_args(static_cast<int>(argv.size()), argv.data());
}
}

TEST_CASE("CLI parses simulation and export options") {
    const auto options = parse({"citysense", "--mode", "sim", "--hours", "2",
                                "--batch", "30", "--window-minutes", "10",
                                "--seed", "99",
                                "--output-json", "summary.json"});
    REQUIRE(options.mode == app::IngestMode::Simulator);
    REQUIRE(options.sim_hours == 2);
    REQUIRE(options.batch_size == 30);
    REQUIRE(options.window_minutes == 10);
    REQUIRE(options.sim_seed == 99);
    REQUIRE(options.output_json == "summary.json");
}

TEST_CASE("CLI rejects unsafe numeric values") {
    REQUIRE_THROWS_AS(parse({"citysense", "--batch", "0"}), std::runtime_error);
    REQUIRE_THROWS_AS(parse({"citysense", "--hours", "0"}), std::runtime_error);
    REQUIRE_THROWS_AS(parse({"citysense", "--window-minutes", "0"}),
                      std::runtime_error);
}
