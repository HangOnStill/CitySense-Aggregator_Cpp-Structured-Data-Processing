# Data directory

The small files `air.csv`, `noise.csv`, `traffic.csv`, `sample.csv`, and
`sample.ndjson` are versioned test fixtures. They are enough to exercise the
readers, aggregator, parallel ingestion path, and CLI examples.

The larger files ending in `_data.csv` are generated course-project artifacts.
They are intentionally excluded from tests and are not needed to build or run
the deterministic simulator. New generated datasets should remain local rather
than being committed.

All fixture timestamps use ISO 8601 UTC and all fixtures share these required
columns:

- `timestamp`
- `sensor_id`
- `zone_id`

Metric columns are source-specific: `speed` and `flow` for traffic, `pm25` and
`pm10` for air quality, and `db` for noise.
