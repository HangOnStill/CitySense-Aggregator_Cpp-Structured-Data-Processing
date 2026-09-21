cmake_minimum_required(VERSION 3.24)
project(CitySense LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CITYSENSE_WARN_AS_ERR "Treat warnings as errors for project sources" ON)
option(CITYSENSE_ENABLE_EXTENDED_TESTS "Build extended contract tests" ON)

function(apply_warnings target)
  if (MSVC)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endif()

  if (CITYSENSE_WARN_AS_ERR)
    if (MSVC)
      target_compile_options(${target} PRIVATE /WX)
    else()
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()

# -------------------------------------------------------------------
# Reusable processing library
# -------------------------------------------------------------------
add_library(citysense_lib STATIC
    src/app/Options.cpp
    src/core/Processor.cpp
    src/sim/Simulator.cpp
    src/sim/PatternAnalytics.cpp
)
target_include_directories(citysense_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)
apply_warnings(citysense_lib)

if (UNIX AND NOT APPLE)
  target_link_libraries(citysense_lib PUBLIC pthread)
endif()
# -------------------------------------------------------------------
# Main app
# -------------------------------------------------------------------
add_executable(citysense
    src/app/main.cpp
)
target_link_libraries(citysense PRIVATE citysense_lib)
apply_warnings(citysense)

# -------------------------------------------------------------------
# Tests (Catch2)
# -------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
  Catch2
  URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.6.0.zip
  URL_HASH SHA256=aa0ebf551ffbf098ec1e253b5fee234c30b4ee54a31b1be63cb1a7735d3cf391
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(Catch2)

include(CTest)
include(Catch)

enable_testing()

# Ensure directory for JUnit XML exists (CTest + CI)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test-results)

# ----------------- Public tests -----------------
add_executable(citysense_tests
  tests/test_build.cpp
  tests/test_detectors.cpp
  tests/test_aggregation.cpp
  tests/test_api_shapes.cpp
  tests/test_json_exporter.cpp
  tests/test_options.cpp
  tests/test_parallel_ingest.cpp
  tests/test_processor.cpp
  tests/test_reader_csv.cpp
  tests/test_reader_json.cpp
  tests/test_simulator.cpp
)
target_link_libraries(citysense_tests PRIVATE citysense_lib Catch2::Catch2WithMain)
target_compile_definitions(citysense_tests PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data")
apply_warnings(citysense_tests)

if (UNIX AND NOT APPLE)
  target_link_libraries(citysense_tests PRIVATE pthread)
endif()
catch_discover_tests(citysense_tests
  TEST_PREFIX "public::"
  REPORTER junit
  OUTPUT_DIR ${CMAKE_BINARY_DIR}/test-results
  OUTPUT_PREFIX "public-"
)

add_test(
  NAME cli::one_hour_simulation
  COMMAND citysense --mode sim --hours 1 --batch 1000 --seed 42
)
set_tests_properties(cli::one_hour_simulation PROPERTIES
  PASS_REGULAR_EXPRESSION "Total ingested rows: 540"
)

# ----------------- Extended contract tests -----------------
if (CITYSENSE_ENABLE_EXTENDED_TESTS)
  add_executable(citysense_tests_extended
    tests/extended/test_csv_contract.cpp
    tests/extended/test_windowing.cpp
    tests/extended/test_parallel.cpp
    tests/extended/test_exceptions.cpp
  )
  target_link_libraries(citysense_tests_extended PRIVATE citysense_lib Catch2::Catch2WithMain)
  target_compile_definitions(citysense_tests_extended PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/data")
  apply_warnings(citysense_tests_extended)

  if (UNIX AND NOT APPLE)
    target_link_libraries(citysense_tests_extended PRIVATE pthread)
  endif()

  catch_discover_tests(citysense_tests_extended
    TEST_PREFIX "extended::"
    REPORTER junit
    OUTPUT_DIR ${CMAKE_BINARY_DIR}/test-results
    OUTPUT_PREFIX "extended-"
    PROPERTIES LABELS "extended"
  )
endif()
