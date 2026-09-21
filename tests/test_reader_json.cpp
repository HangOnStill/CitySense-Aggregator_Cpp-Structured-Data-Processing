#include <catch2/catch_test_macros.hpp>
#include <chrono>

#include "io/ReaderJSON.hpp"

TEST_CASE("NDJSON reader parses records and skips malformed lines") {
    io::ReaderJSON reader({TEST_DATA_DIR "/sample.ndjson"});
    const auto rows = reader.next_batch(10);

    REQUIRE(rows.size() == 2);
    REQUIRE(rows.front().sensor_id == "air-1");
    REQUIRE(rows.front().zone_id == 1);
    REQUIRE(rows.front().pm25 == 12.5);
    REQUIRE(rows.front().ts.time_since_epoch() == std::chrono::seconds{1700000000});
}
