#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <stdexcept>

#include "core/Processor.hpp"

TEST_CASE("Processor filters, buckets, and computes rolling statistics") {
    core::Processor::Config config;
    config.zone_filter = {1};
    config.bucket_minutes = 5;
    config.rolling_window_size = 2;
    core::Processor processor{config};

    model::SensorRecord first;
    first.ts = std::chrono::system_clock::time_point{std::chrono::minutes{1}};
    first.sensor_id = "traffic-1";
    first.zone_id = 1;
    first.speed = 10.0;

    auto second = first;
    second.ts = std::chrono::system_clock::time_point{std::chrono::minutes{2}};
    second.speed = 20.0;

    auto filtered = second;
    filtered.zone_id = 2;

    processor.process_batch({first, second, filtered});
    const auto diagnostics = processor.get_diagnostics();
    REQUIRE(diagnostics.total_records == 3);
    REQUIRE(diagnostics.processed == 2);
    REQUIRE(diagnostics.filtered_out == 1);

    const auto buckets = processor.get_bucket_stats();
    REQUIRE(buckets.size() == 1);
    REQUIRE(buckets.front().count == 2);
    REQUIRE(buckets.front().speed_mean == Catch::Approx(12.5));
    REQUIRE(buckets.front().speed_median == Catch::Approx(12.5));
}

TEST_CASE("Processor rejects invalid configuration") {
    core::Processor::Config config;
    config.bucket_minutes = 0;
    REQUIRE_THROWS_AS(core::Processor{config}, std::invalid_argument);
}
