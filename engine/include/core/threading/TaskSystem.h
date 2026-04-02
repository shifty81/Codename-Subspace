#pragma once

#include <cstdint>
#include <functional>
#include <future>

namespace subspace {

/// Fixed-size thread pool that executes submitted tasks asynchronously.
///
/// Usage:
/// @code
///   TaskSystem tasks;
///   tasks.Init(4);
///
///   auto f = tasks.Submit([]{ /* heavy work */ });
///   f.wait();
///
///   tasks.Shutdown();
/// @endcode
class TaskSystem {
public:
    TaskSystem() = default;
    ~TaskSystem();

    // Non-copyable, non-movable (owns worker threads).
    TaskSystem(const TaskSystem&)            = delete;
    TaskSystem& operator=(const TaskSystem&) = delete;

    /// Spawn worker threads and begin accepting tasks.
    /// @param threadCount  Number of worker threads.
    ///                     Pass 0 to use std::thread::hardware_concurrency().
    void Init(uint32_t threadCount = 0);

    /// Drain all pending tasks, join workers, and release resources.
    void Shutdown();

    /// Whether the task system has been initialised.
    bool IsRunning() const noexcept;

    /// Enqueue a callable for asynchronous execution.
    /// @param task  Callable with signature void().
    /// @return A future that becomes ready once the task completes.
    std::future<void> Submit(std::function<void()> task);

private:
    struct Impl;
    Impl* _impl = nullptr;
};

} // namespace subspace
