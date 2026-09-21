#pragma once
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../core/Aggregator.hpp"

namespace export_ {

// Deterministic JSON output without a runtime dependency.
class JsonExporter {
    std::string path_;
public:
    explicit JsonExporter(std::string path) : path_(std::move(path)) {}

    void emit(const core::Summary& s) const {
        std::ofstream out(path_, std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Could not open JSON output: " + path_);
        }

        std::vector<int> zones;
        zones.reserve(s.by_zone.size());
        for (const auto& [zone, _] : s.by_zone) zones.push_back(zone);
        std::sort(zones.begin(), zones.end());

        out << "{\n  \"total_count\": " << s.total_count << ",\n  \"zones\": {";
        for (std::size_t i = 0; i < zones.size(); ++i) {
            const int zone = zones[i];
            out << (i == 0 ? "\n" : ",\n")
                << "    \"" << zone << "\": {\"record_count\": "
                << s.by_zone.at(zone);

            if (const auto it = s.metrics_by_zone.find(zone);
                it != s.metrics_by_zone.end()) {
                const auto& m = it->second;
                out << ", \"speed_mean\": " << m.speed.mean()
                    << ", \"flow_mean\": " << m.flow.mean()
                    << ", \"pm25_mean\": " << m.pm25.mean()
                    << ", \"pm10_mean\": " << m.pm10.mean()
                    << ", \"db_mean\": " << m.db.mean();
            }
            out << "}";
        }
        out << (zones.empty() ? "" : "\n") << "  }\n}\n";
        if (!out) {
            throw std::runtime_error("Failed while writing JSON output: " + path_);
        }
    }
};

} // namespace export_
