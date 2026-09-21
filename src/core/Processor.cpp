#include "Processor.hpp"
#include <algorithm>
#include <stdexcept>

using namespace std;

namespace core {

Processor::Processor(const Config& config) : config_(config) {
    if (config_.bucket_minutes <= 0) {
        throw std::invalid_argument("bucket_minutes must be greater than zero");
    }
    if (config_.rolling_window_size <= 0) {
        throw std::invalid_argument("rolling_window_size must be greater than zero");
    }
    if (config_.start_time && config_.end_time &&
        *config_.start_time > *config_.end_time) {
        throw std::invalid_argument("start_time must not be after end_time");
    }
}

void Processor::process_batch(const vector<model::SensorRecord>& records) {
    for(const auto& record : records){
        diag_.total_records++;

        if(!passes_filters(record)){
            continue;
        }

        model::SensorRecord transformed = apply_transformations(record);

        auto bucket_start = get_bucket_start(transformed.ts);

        BucketKey key{bucket_start, transformed.zone_id};

        auto& bucket = buckets_[key];
        ++bucket.record_count;
        //adding each metric to bucket vectors if present
        if(transformed.speed.has_value()){
            bucket.speed_values.push_back(transformed.speed.value());
        }
        if(transformed.flow.has_value()){
            bucket.flow_values.push_back(transformed.flow.value());
        }
        if(transformed.pm25.has_value()){
            bucket.pm25_values.push_back(transformed.pm25.value());
        }
        if(transformed.pm10.has_value()){
            bucket.pm10_values.push_back(transformed.pm10.value());
        }
        if(transformed.db.has_value()){
            bucket.db_values.push_back(transformed.db.value());
        }
        diag_.processed++;
    }
}

vector<BucketStats> Processor::get_bucket_stats() const {
    vector<BucketStats> results;

    // loop through all the buckets
    for(const auto& [key, bucket] : buckets_){
        BucketStats stats;

        stats.bucket_start = key.bucket_start;
        stats.zone_id = key.zone_id;
        stats.count = bucket.record_count;

                          if(!bucket.speed_values.empty()){
        compute_stats(bucket.speed_values,
                      stats.speed_mean,
                      stats.speed_median,
                      stats.speed_p90,
                      stats.speed_p99);
    }
    if(!bucket.flow_values.empty()){
        compute_stats(bucket.flow_values,
                      stats.flow_mean,
                      stats.flow_median,
                      stats.flow_p90,
                      stats.flow_p99);  
                    }
    if(!bucket.pm25_values.empty()){
        compute_stats(bucket.pm25_values,
        stats.pm25_mean, stats.pm25_median,
        stats.pm25_p90, stats.pm25_p99);
    }

    // PM10 stats
    if(!bucket.pm10_values.empty()){
        compute_stats(bucket.pm10_values,
        stats.pm10_mean, stats.pm10_median,
    stats.pm10_p90, stats.pm10_p99);
    }
    // dB stats
    if(!bucket.db_values.empty()){
        compute_stats(bucket.db_values,
        stats.db_mean, stats.db_median,
        stats.db_p90, stats.db_p99);
    }

        results.push_back(stats);
    }

    return results;

}

bool Processor::passes_filters(const model::SensorRecord& record) {
    // Filter 1: Check start time
    if(config_.start_time.has_value() && record.ts < config_.start_time.value()){
        ++diag_.filtered_out;
        return false;
    }
    
    // Filter 2: Check end time
    if(config_.end_time.has_value() && record.ts > config_.end_time.value()){
        ++diag_.filtered_out;
        return false;
    }
    
    // Filter 3: Check zone
    if(!config_.zone_filter.empty()){
        bool found = find(config_.zone_filter.begin(), config_.zone_filter.end(), record.zone_id) != config_.zone_filter.end();
        if(!found){
            ++diag_.filtered_out;
            return false;
        }
    }
    
    // Passed all filters!
    return true;
}

chrono::system_clock::time_point Processor::get_bucket_start(
    const chrono::system_clock::time_point& timestamp) const {
    auto minutes_since_epoch = chrono::duration_cast<chrono::minutes>(
        timestamp.time_since_epoch()
    ).count();

    long long bucket_number = minutes_since_epoch / config_.bucket_minutes;
    long long bucket_start_minutes = bucket_number * config_.bucket_minutes;

    return chrono::system_clock::time_point(chrono::minutes(bucket_start_minutes));
}

model::SensorRecord Processor::apply_transformations(const model::SensorRecord& record) {
    // checks if rolling averages are enabled
    if(!config_.apply_rolling_avg){
        return record;
    }
    model::SensorRecord transformed = record; // creates a copy of the record to modify

    string sensor_id = record.sensor_id;

    auto update = [&](auto& rolling, const std::optional<double>& value,
                      std::optional<double>& output) {
        if (!value) return;
        auto inserted = rolling.try_emplace(
            sensor_id, static_cast<std::size_t>(config_.rolling_window_size));
        auto it = inserted.first;
        it->second.add(*value);
        output = it->second.get_average();
    };

    if(record.speed.has_value()){
        update(speed_rolling_, record.speed, transformed.speed);
    }
    //flow
    if(record.flow.has_value()){
        update(flow_rolling_, record.flow, transformed.flow);
    }
    // PM2.5
    if(record.pm25.has_value()){
        update(pm25_rolling_, record.pm25, transformed.pm25);
    }
    // PM10
    if(record.pm10.has_value()){
        update(pm10_rolling_, record.pm10, transformed.pm10);
    }

    //Decibels (dB)
    if(record.db.has_value()){
        update(db_rolling_, record.db, transformed.db);
    }

    return transformed;
    
}

void Processor::compute_stats(const vector<double>& values,
                              optional<double>& mean,
                              optional<double>& median,
                              optional<double>& p90,
                              optional<double>& p99) const {
    if(values.empty()){
        return;
    }
    double sum = accumulate(values.begin(), values.end(), 0.0);

    mean = sum / values.size();

    vector<double> sorted = values;
    sort(sorted.begin(), sorted.end());

    size_t n = sorted.size();
    size_t mid = n/2;

    if(n % 2 == 0){
        median = (sorted[mid - 1] + sorted[mid]) / 2.0;
    }else{
        median = sorted[mid];
    }
    // 90th percentile values are 90% or below
    size_t idx_p90 = static_cast<size_t>((ceil(0.90 * n)) - 1);
    // make sure index is in bound
    if(idx_p90 >= n) idx_p90 = n - 1;
    p90 = sorted[idx_p90];

    size_t idx_p99 = static_cast<size_t>(ceil(0.99 * n)) -1;

    if (idx_p99 >= n) idx_p99 = n-1;
    p99 = sorted[idx_p99];
}

} // namespace core
