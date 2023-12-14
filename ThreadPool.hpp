// ThreadPool.h
#pragma once

#include "index.hpp"
#include "Session.hpp"

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    template<typename F>
    void enqueue(F&& task);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};
