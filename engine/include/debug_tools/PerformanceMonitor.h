#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

/// A single timestamped performance sample.
struct PerfSample {
    float value = 0.0f;      ///< Measured value (ms, count, bytes, etc.)
    float timestamp = 0.0f;  ///< Elapsed game time when sampled.
};

/// Accumulates samples for a named metric and provides statistics.
class PerfMetric {
public:
    explicit PerfMetric(const std::string& name = "", int maxSamples = 120);

    /// Record a new sample value.
    void Record(float value, float timestamp = 0.0f);

    /// Get the most recent sample value.
    float GetLatest() const;

    /// Get the average over all stored samples.
    float GetAverage() const;

    /// Get the minimum value across stored samples.
    float GetMin() const;

    /// Get the maximum value across stored samples.
    float GetMax() const;

    /// Get the number of stored samples.
    int GetSampleCount() const;

    /// Get the metric name.
    const std::string& GetName() const;

    /// Clear all samples.
    void Clear();

    /// Get all stored samples (for graphing).
    const std::vector<PerfSample>& GetSamples() const;

private:
    std::string _name;
    int _maxSamples;
    std::vector<PerfSample> _samples;
};

/// Tracks per-system timing budgets and overall performance metrics.
/// Designed for the debug HUD / telemetry overlay.
class PerformanceMonitor {
public:
    PerformanceMonitor();

    // ------------------------------------------------------------------
    // Frame-level tracking
    // ------------------------------------------------------------------

    /// Call at the start of each frame.
    void BeginFrame();

    /// Call at the end of each frame. Records frame time and FPS.
    void EndFrame();

    /// Get the last recorded frame time in milliseconds.
    float GetFrameTimeMs() const;

    /// Get the computed frames-per-second (smoothed).
    float GetFPS() const;

    /// Get the total number of frames recorded.
    int GetFrameCount() const;

    // ------------------------------------------------------------------
    // System timing
    // ------------------------------------------------------------------

    /// Begin timing a named system/scope.
    void BeginSection(const std::string& name);

    /// End timing a named system/scope. Records the elapsed ms.
    void EndSection(const std::string& name);

    /// Get the latest timing for a named section (ms).
    float GetSectionTime(const std::string& name) const;

    /// Get the metric object for a named section.
    const PerfMetric* GetMetric(const std::string& name) const;

    // ------------------------------------------------------------------
    // Custom counters
    // ------------------------------------------------------------------

    /// Record a custom counter value (entity count, draw calls, etc.).
    void RecordCounter(const std::string& name, float value);

    /// Get the latest counter value.
    float GetCounter(const std::string& name) const;

    // ------------------------------------------------------------------
    // Queries
    // ------------------------------------------------------------------

    /// Get names of all tracked metrics (sections + counters).
    std::vector<std::string> GetAllMetricNames() const;

    /// Get a formatted summary string for the performance overlay.
    std::string GetSummary() const;

    /// Reset all metrics.
    void Reset();

private:
    std::unordered_map<std::string, PerfMetric> _metrics;

    // Frame tracking
    float _frameStartTime = 0.0f;
    float _lastFrameTimeMs = 0.0f;
    float _fps = 0.0f;
    int _frameCount = 0;
    float _elapsedTime = 0.0f;

    // Section timing helper
    std::unordered_map<std::string, float> _sectionStartTimes;

    /// Internal: get or create a metric by name.
    PerfMetric& GetOrCreateMetric(const std::string& name);
};

} // namespace subspace
