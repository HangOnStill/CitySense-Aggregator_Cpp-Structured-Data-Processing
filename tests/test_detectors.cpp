#include <catch2/catch_test_macros.hpp>
#include <chrono>

#include "core/Window.hpp"
#include "detectors/AirAlert.hpp"
#include "detectors/NoiseSpike.hpp"
#include "detectors/TrafficCongestion.hpp"

namespace {
model::SensorRecord record_at(int minute) {
    model::SensorRecord record;
    record.ts = std::chrono::system_clock::time_point{std::chrono::minutes{minute}};
    record.zone_id = 1;
    return record;
}
}

TEST_CASE("Air detector reports elevated mean PM2.5") {
    core::Window window;
    auto first = record_at(0);
    first.pm25 = 30.0;
    auto second = record_at(1);
    second.pm25 = 40.0;
    window.records = {first, second};

    const auto findings = detectors::AirAlert{25.0}.detect(window);
    REQUIRE(findings.size() == 1);
    REQUIRE(findings.front().detector == "AirAlert");
    REQUIRE(findings.front().value == 35.0);
}

TEST_CASE("Noise detector counts spikes inside its configured window") {
    core::Window window;
    for (int minute = 0; minute < 3; ++minute) {
        auto record = record_at(minute);
        record.db = 80.0;
        window.records.push_back(record);
    }

    const auto findings = detectors::NoiseSpike{70.0, 3, 5}.detect(window);
    REQUIRE(findings.size() == 1);
    REQUIRE(findings.front().value == 3.0);
}

TEST_CASE("Traffic detector requires consecutive slow minutes") {
    core::Window window;
    for (int minute = 0; minute < 2; ++minute) {
        auto record = record_at(minute);
        record.speed = 10.0;
        window.records.push_back(record);
    }

    const auto findings = detectors::TrafficCongestion{20.0, 2}.detect(window);
    REQUIRE(findings.size() == 1);
    REQUIRE(findings.front().value == 2.0);
}
