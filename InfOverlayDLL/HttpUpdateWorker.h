#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <functional>

struct HttpTask
{
    int taskId;
    std::wstring url;
    int intervalMs;
    std::chrono::steady_clock::time_point lastFetch;
    std::function<void(const std::string&)> onResult;
};

class HttpUpdateWorker
{
public:
    static HttpUpdateWorker& Instance()
    {
        static HttpUpdateWorker inst;
        return inst;
    }

    int AddTask(const std::wstring& url,
        int intervalMs,
        std::function<void(const std::string&)> callback);

    void RemoveTask(int taskId);

    void Start();
    void Stop();

private:
    void WorkerLoop();

    std::atomic<bool> m_running{ false };
    std::thread m_thread;

    std::mutex m_mutex;
    std::vector<HttpTask> m_tasks;

    std::atomic<int> m_nextId{ 1 };
};
