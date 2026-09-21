#include <catch2/catch_test_macros.hpp>
#include <chrono>

#include "io/ReaderCSV.hpp"

TEST_CASE("CSV reader preserves timestamps and numeric zone identifiers") {
    io::ReaderCSV reader({TEST_DATA_DIR "/air.csv"});
    const auto rows = reader.next_batch(10);
    REQUIRE_FALSE(rows.empty());
    REQUIRE(rows.front().zone_id == 1);
    REQUIRE(rows.front().sensor_id == "A1");
    REQUIRE(rows.front().ts.time_since_epoch() != std::chrono::seconds{0});
    REQUIRE(reader.malformed_count() == 0);
}
