#include "HttpUpdateWorker.h"
#include "HttpClient.h"  // Äã×Ô¼ºµÄ HttpClient

int HttpUpdateWorker::AddTask(
    const std::wstring& url,
    int intervalMs,
    std::function<void(const std::string&)> callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    int id = m_nextId++;

    m_tasks.push_back({
        id,
        url,
        intervalMs,
        std::chrono::steady_clock::now(),
        callback
        });

    return id;
}

void HttpUpdateWorker::RemoveTask(int taskId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_tasks.erase(
        std::remove_if(m_tasks.begin(), m_tasks.end(),
            [taskId](const HttpTask& t) {
                return t.taskId == taskId;
            }),
        m_tasks.end()
    );
}


void HttpUpdateWorker::Start()
{
    if (m_running) return;

    m_running = true;
    m_thread = std::thread(&HttpUpdateWorker::WorkerLoop, this);
}

void HttpUpdateWorker::Stop()
{
    if (!m_running) return;

    m_running = false;

    if (m_thread.joinable())
        m_thread.join();
}

void HttpUpdateWorker::WorkerLoop()
{
    HttpClient client;

    while (m_running)
    {
        auto now = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            for (auto& task : m_tasks)
            {
                if (now - task.lastFetch >= 
                    std::chrono::milliseconds(task.intervalMs))
                {
                    std::string response;
                    if (client.HttpGet(task.url, response))
                    {
                        if (task.onResult)
                            task.onResult(response);
                    }
                    task.lastFetch = now;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
