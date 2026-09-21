#include <catch2/catch_test_macros.hpp>
#include <chrono>

#include "sim/Clock.hpp"
#include "sim/SeededRNG.hpp"
#include "sim/Simulator.hpp"

using namespace std::chrono;

TEST_CASE("Simulator produces nine deterministic sensor records per minute") {
    sim::Clock first_clock{system_clock::time_point{seconds{0}}, 60};
    sim::Clock second_clock{system_clock::time_point{seconds{0}}, 60};
    sim::Simulator first{first_clock, sim::SeededRNG{42}};
    sim::Simulator second{second_clock, sim::SeededRNG{42}};

    first.start(sim::SimulatorProfile::Weekday);
    second.start(sim::SimulatorProfile::Weekday);

    const auto first_step = first.next_step();
    const auto matching_step = second.next_step();
    REQUIRE(first_step.size() == 9);
    REQUIRE(matching_step.size() == 9);
    REQUIRE(first_step.front().sensor_id == matching_step.front().sensor_id);
    REQUIRE(first_step.front().speed == matching_step.front().speed);
}

TEST_CASE("Simulator batching is expressed in time steps") {
    sim::Clock clock{system_clock::time_point{seconds{0}}, 60};
    sim::Simulator simulator{clock, sim::SeededRNG{7}};
    simulator.start(sim::SimulatorProfile::Weekday);

    REQUIRE(simulator.next_batch(5).size() == 45);
    simulator.pause();
    REQUIRE(simulator.next_step().empty());
    simulator.resume();
    REQUIRE(simulator.next_step().size() == 9);
}
