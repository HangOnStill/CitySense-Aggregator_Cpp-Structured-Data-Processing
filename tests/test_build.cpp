#include <catch2/catch_test_macros.hpp>

#include "model/SensorRecord.hpp"

TEST_CASE("Sensor records use safe defaults") {
    const model::SensorRecord record;
    REQUIRE(record.zone_id == 0);
    REQUIRE(record.sensor_id.empty());
    REQUIRE_FALSE(record.speed.has_value());
    REQUIRE_FALSE(record.pm25.has_value());
}
