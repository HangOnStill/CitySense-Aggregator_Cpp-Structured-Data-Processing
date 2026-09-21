#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include "core/Aggregator.hpp"
#include "export/JsonExporter.hpp"

TEST_CASE("JSON exporter writes deterministic summary fields") {
    core::Aggregator aggregator{0};
    model::SensorRecord record;
    record.zone_id = 2;
    record.speed = 30.0;
    aggregator.consume(record);

    const auto path = std::filesystem::temp_directory_path() /
        "citysense-summary-test.json";
    export_::JsonExporter{path.string()}.emit(aggregator.summary());

    std::ifstream input(path);
    const std::string json{std::istreambuf_iterator<char>{input},
                           std::istreambuf_iterator<char>{}};
    REQUIRE(json.find("\"total_count\": 1") != std::string::npos);
    REQUIRE(json.find("\"2\": {\"record_count\": 1") != std::string::npos);
    REQUIRE(json.find("\"speed_mean\": 30") != std::string::npos);
    input.close();
    std::filesystem::remove(path);
}
