#pragma once
#include <Windows.h>
#include "Item.h"
#include "UpdateModule.h"
#include <string>
#include <chrono>

struct click_container {
    std::vector<int> leftContainer;
    std::vector<int> rightContainer;

    void AddLeftClick(int times)
    {
        leftContainer.push_back(times);
    }

    void AddRightClick(int times)
    {
        rightContainer.push_back(times);
    }

    void processClick()
    {
        for (int i = 0; i < leftContainer.size(); i++)
        {
            leftContainer[i] -= 1;
            if (leftContainer[i] == 0)
            {
                leftContainer.erase(leftContainer.begin() + i);
            }
        }
        for (int i = 0; i < rightContainer.size(); i++)
        {
            rightContainer[i] -= 1;
            if (rightContainer[i] == 0)
            {
                rightContainer.erase(rightContainer.begin() + i);
            }
        }
    }

    int GetLeftCPS()
    {
        int left = (int)leftContainer.size();
        return left;
    }
    int GetRightCPS()
    {
        int right = (int)rightContainer.size();
        return right;
    }
};

class CPSDetector : public Item, public UpdateModule
{
public:
    CPSDetector() {
        type = Hidden; // 信息项类型
        multiType = Singleton;    // 信息项是否可以多开
        name = u8"CPS检测";
        description = u8"检测左右键CPS";
        isEnabled = true;
        updateIntervalMs = 2;
        lastUpdateTime = std::chrono::steady_clock::now();
        lastCpsTime = std::chrono::steady_clock::now();
    }
    ~CPSDetector() {}

    static CPSDetector& Instance() {
        static CPSDetector instance;
        return instance;
    }

    void Toggle() override;
    void Update() override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;
    void DrawSettings() override;

    int GetLeftCPS();
    int GetRightCPS();
private:

    click_container cps;
    // 检查是否到了更新的时间
    bool ShouldCpsUpdate();

    // 更新操作
    void MarkUpCPSdated();
    int cpsIntervalMs = 50;
    std::chrono::steady_clock::time_point lastCpsTime;  // 记录最后更新时间
};