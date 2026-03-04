#include "debug_tools/PerformanceMonitor.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace subspace {

// ---------------------------------------------------------------------------
// PerfMetric
// ---------------------------------------------------------------------------

PerfMetric::PerfMetric(const std::string& name, int maxSamples)
    : _name(name)
    , _maxSamples(maxSamples > 0 ? maxSamples : 120)
{
}

void PerfMetric::Record(float value, float timestamp) {
    PerfSample sample;
    sample.value = value;
    sample.timestamp = timestamp;
    _samples.push_back(sample);

    // Evict oldest when over capacity.
    while (static_cast<int>(_samples.size()) > _maxSamples) {
        _samples.erase(_samples.begin());
    }
}

float PerfMetric::GetLatest() const {
    if (_samples.empty()) return 0.0f;
    return _samples.back().value;
}

float PerfMetric::GetAverage() const {
    if (_samples.empty()) return 0.0f;
    float sum = 0.0f;
    for (const auto& s : _samples) sum += s.value;
    return sum / static_cast<float>(_samples.size());
}

float PerfMetric::GetMin() const {
    if (_samples.empty()) return 0.0f;
    float minVal = _samples[0].value;
    for (const auto& s : _samples) {
        if (s.value < minVal) minVal = s.value;
    }
    return minVal;
}

float PerfMetric::GetMax() const {
    if (_samples.empty()) return 0.0f;
    float maxVal = _samples[0].value;
    for (const auto& s : _samples) {
        if (s.value > maxVal) maxVal = s.value;
    }
    return maxVal;
}

int PerfMetric::GetSampleCount() const {
    return static_cast<int>(_samples.size());
}

const std::string& PerfMetric::GetName() const {
    return _name;
}

void PerfMetric::Clear() {
    _samples.clear();
}

const std::vector<PerfSample>& PerfMetric::GetSamples() const {
    return _samples;
}

// ---------------------------------------------------------------------------
// PerformanceMonitor
// ---------------------------------------------------------------------------

PerformanceMonitor::PerformanceMonitor() = default;

void PerformanceMonitor::BeginFrame() {
    auto now = std::chrono::steady_clock::now();
    _frameStartTime = std::chrono::duration<float, std::milli>(
        now.time_since_epoch()).count();
}

void PerformanceMonitor::EndFrame() {
    auto now = std::chrono::steady_clock::now();
    float endTime = std::chrono::duration<float, std::milli>(
        now.time_since_epoch()).count();

    _lastFrameTimeMs = endTime - _frameStartTime;
    if (_lastFrameTimeMs < 0.0f) _lastFrameTimeMs = 0.0f;

    ++_frameCount;
    _elapsedTime += _lastFrameTimeMs;

    // Record to metrics
    auto& ftMetric = GetOrCreateMetric("FrameTime");
    ftMetric.Record(_lastFrameTimeMs, _elapsedTime);

    // Compute smoothed FPS (from latest frame time)
    _fps = (_lastFrameTimeMs > 0.0f) ? (1000.0f / _lastFrameTimeMs) : 0.0f;

    auto& fpsMetric = GetOrCreateMetric("FPS");
    fpsMetric.Record(_fps, _elapsedTime);
}

float PerformanceMonitor::GetFrameTimeMs() const {
    return _lastFrameTimeMs;
}

float PerformanceMonitor::GetFPS() const {
    return _fps;
}

int PerformanceMonitor::GetFrameCount() const {
    return _frameCount;
}

// ---------------------------------------------------------------------------
// Section timing
// ---------------------------------------------------------------------------

void PerformanceMonitor::BeginSection(const std::string& name) {
    auto now = std::chrono::steady_clock::now();
    float t = std::chrono::duration<float, std::milli>(
        now.time_since_epoch()).count();
    _sectionStartTimes[name] = t;
}

void PerformanceMonitor::EndSection(const std::string& name) {
    auto it = _sectionStartTimes.find(name);
    if (it == _sectionStartTimes.end()) return;

    auto now = std::chrono::steady_clock::now();
    float endTime = std::chrono::duration<float, std::milli>(
        now.time_since_epoch()).count();

    float elapsed = endTime - it->second;
    if (elapsed < 0.0f) elapsed = 0.0f;

    auto& metric = GetOrCreateMetric(name);
    metric.Record(elapsed, _elapsedTime);

    _sectionStartTimes.erase(it);
}

float PerformanceMonitor::GetSectionTime(const std::string& name) const {
    auto it = _metrics.find(name);
    if (it == _metrics.end()) return 0.0f;
    return it->second.GetLatest();
}

const PerfMetric* PerformanceMonitor::GetMetric(const std::string& name) const {
    auto it = _metrics.find(name);
    if (it == _metrics.end()) return nullptr;
    return &it->second;
}

// ---------------------------------------------------------------------------
// Custom counters
// ---------------------------------------------------------------------------

void PerformanceMonitor::RecordCounter(const std::string& name, float value) {
    auto& metric = GetOrCreateMetric(name);
    metric.Record(value, _elapsedTime);
}

float PerformanceMonitor::GetCounter(const std::string& name) const {
    auto it = _metrics.find(name);
    if (it == _metrics.end()) return 0.0f;
    return it->second.GetLatest();
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

std::vector<std::string> PerformanceMonitor::GetAllMetricNames() const {
    std::vector<std::string> names;
    names.reserve(_metrics.size());
    for (const auto& pair : _metrics) {
        names.push_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::string PerformanceMonitor::GetSummary() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "FPS: " << _fps
       << " | Frame: " << _lastFrameTimeMs << "ms"
       << " | Frames: " << _frameCount;

    // Append section timings
    for (const auto& pair : _metrics) {
        if (pair.first == "FrameTime" || pair.first == "FPS") continue;
        ss << " | " << pair.first << ": " << pair.second.GetLatest();
    }

    return ss.str();
}

void PerformanceMonitor::Reset() {
    _metrics.clear();
    _sectionStartTimes.clear();
    _frameStartTime = 0.0f;
    _lastFrameTimeMs = 0.0f;
    _fps = 0.0f;
    _frameCount = 0;
    _elapsedTime = 0.0f;
}

// ---------------------------------------------------------------------------
// Internal
// ---------------------------------------------------------------------------

PerfMetric& PerformanceMonitor::GetOrCreateMetric(const std::string& name) {
    auto it = _metrics.find(name);
    if (it == _metrics.end()) {
        _metrics.emplace(name, PerfMetric(name));
        return _metrics.at(name);
    }
    return it->second;
}

} // namespace subspace
