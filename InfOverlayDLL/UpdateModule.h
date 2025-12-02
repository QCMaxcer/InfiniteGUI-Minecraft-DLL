#pragma once
#include <chrono>
class UpdateModule
{
public:
	virtual void Update() = 0;

    // 检查是否到了更新的时间
    bool ShouldUpdate() {
        //if (updateIntervalMs == -1) return false;
        auto now = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime).count();
        return elapsedTime >= updateIntervalMs;
    }

    // 更新操作
    void MarkUpdated() {
        lastUpdateTime = std::chrono::steady_clock::now();
    }
protected:
	int updateIntervalMs;    // 默认 1 秒
	std::chrono::steady_clock::time_point lastUpdateTime = std::chrono::steady_clock::now();  // 记录最后更新时间
};