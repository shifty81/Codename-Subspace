#include "core/threading/TaskSystem.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace subspace {

struct TaskSystem::Impl {
    std::vector<std::thread>          workers;
    std::queue<std::function<void()>> queue;
    std::mutex                        mutex;
    std::condition_variable           cv;
    bool                              stop = false;
};

TaskSystem::~TaskSystem() {
    if (_impl) Shutdown();
}

void TaskSystem::Init(uint32_t threadCount) {
    if (_impl) return; // already running
    _impl = new Impl{};

    uint32_t n = threadCount > 0
                     ? threadCount
                     : static_cast<uint32_t>(std::thread::hardware_concurrency());
    if (n == 0) n = 2;

    for (uint32_t i = 0; i < n; ++i) {
        _impl->workers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(_impl->mutex);
                    _impl->cv.wait(lock, [this] {
                        return _impl->stop || !_impl->queue.empty();
                    });
                    if (_impl->stop && _impl->queue.empty()) return;
                    task = std::move(_impl->queue.front());
                    _impl->queue.pop();
                }
                task();
            }
        });
    }
}

void TaskSystem::Shutdown() {
    if (!_impl) return;
    {
        std::unique_lock<std::mutex> lock(_impl->mutex);
        _impl->stop = true;
    }
    _impl->cv.notify_all();
    for (auto& t : _impl->workers) t.join();
    delete _impl;
    _impl = nullptr;
}

bool TaskSystem::IsRunning() const noexcept {
    return _impl != nullptr && !_impl->stop;
}

std::future<void> TaskSystem::Submit(std::function<void()> task) {
    auto promise = std::make_shared<std::promise<void>>();
    std::future<void> fut = promise->get_future();

    if (!_impl) {
        // Fall back to inline execution when not initialised.
        task();
        promise->set_value();
        return fut;
    }

    {
        std::unique_lock<std::mutex> lock(_impl->mutex);
        _impl->queue.push([t = std::move(task), p = std::move(promise)]() mutable {
            t();
            p->set_value();
        });
    }
    _impl->cv.notify_one();
    return fut;
}

} // namespace subspace
