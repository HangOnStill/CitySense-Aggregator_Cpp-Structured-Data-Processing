# CitySense

[![C++ CI](https://github.com/HangOnStill/CitySense-Aggregator_Cpp-Structured-Data-Processing/actions/workflows/ci.yml/badge.svg)](https://github.com/HangOnStill/CitySense-Aggregator_Cpp-Structured-Data-Processing/actions/workflows/ci.yml)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/build-CMake-064F8C.svg)](https://cmake.org/)

CitySense is a C++20 command-line pipeline for streaming, simulating, filtering,
and aggregating urban traffic, air-quality, and noise sensor data. It combines
bounded-window analytics with deterministic simulation and concurrent per-zone
ingestion, then exposes human-readable and JSON summaries.

## Why this project matters

- **Streaming ingestion:** reads multiple CSV sources in configurable batches
  instead of loading an entire dataset into memory.
- **Defensive data handling:** validates required headers, skips malformed rows,
  preserves UTC timestamps, and reports missing inputs explicitly.
- **Windowed analytics:** maintains rolling time windows and computes per-zone
  counts and metric means.
- **Deterministic simulation:** produces repeatable traffic, pollution, and noise
  signals from a supplied random seed.
- **Concurrency:** processes independent zones through blocking, thread-safe
  queues and verifies the result against serial ingestion.
- **Portable engineering:** builds with CMake and is tested on Linux and Windows
  with warnings treated as errors.

## Architecture

```text
CSV files or seeded simulator
            |
            v
  parsing and validation
            |
            v
 filtering / rolling transforms
            |
            v
 time-window aggregation ------> threshold detectors
            |
            +------> console summary
            +------> deterministic JSON summary
```

The code is split by responsibility:

```text
src/
  app/        CLI parsing and application entry point
  core/       windowing, aggregation, and statistical processing
  detectors/  traffic, air-quality, and noise rules
  export/     console and JSON output
  io/         streaming CSV and NDJSON readers
  model/      shared sensor record types
  parallel/   per-zone worker queues
  sim/        deterministic sensor simulation and pattern analysis
tests/        public behavior and integration tests
tests/extended/ contract, concurrency, and windowing tests
data/         small fixtures plus the original course datasets
```

## Quick start

Requirements: a C++20 compiler, CMake 3.24+, and an internet connection during
the first configuration so CMake can fetch the pinned Catch2 release.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build --build-config Release --output-on-failure
```

Run a reproducible one-hour simulation:

```bash
./build/citysense --mode sim --hours 1 --seed 42 --output-json summary.json
```

On a multi-configuration Windows generator, the executable may be located at
`build/Release/citysense.exe`.

Process the included fixtures:

```bash
./build/citysense --mode csv \
  --input data/traffic.csv \
  --input data/air.csv \
  --input data/noise.csv \
  --output-json summary.json
```

Use `--zone`, `--from`, `--to`, and `--window-minutes` to control the active
analysis window. Run
`./build/citysense --help` for the complete CLI reference.

## Example output

```text
Total ingested rows: 540
Per zone:
  zone 1: 18
  zone 2: 18
  zone 3: 18
JSON summary: summary.json
```

`total_count` is the lifetime ingest count; per-zone records and metric means
describe the active time window.

## Verification

The automated suite covers:

- CSV contracts, timestamp parsing, and missing-file behavior;
- deterministic simulation and batching;
- rolling transformations and percentile statistics;
- time-window eviction and exact per-zone aggregation;
- equivalence between serial and concurrent ingestion;
- CLI validation and JSON export.

GitHub Actions runs the same build and test flow on both Ubuntu and Windows.

## Data note

Small CSV files in `data/` are sufficient for the test suite and demo commands.
The larger `*_data.csv` files are original generated course artifacts and are
not required to build or test the application. See [data/README.md](data/README.md)
for the distinction.

## Team-project attribution

CitySense originated as a three-person course project. My interview-ready
contributions include the C++ processing pipeline, validation and error
handling, schema-consistent structured output, and modular support for new
metrics and zones. The repository history retains the team's work and should
not be interpreted as a claim of sole authorship.

The original project brief is retained in
[docs/project-proposal.pdf](docs/project-proposal.pdf) for context.
