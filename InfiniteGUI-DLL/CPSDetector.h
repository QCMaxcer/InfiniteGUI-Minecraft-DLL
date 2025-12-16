#pragma once
#include <Windows.h>
#include "Item.h"
#include "UpdateModule.h"
#include <string>
#include <vector>
#include <deque>
#include <chrono>
#include "KeyState.h"
//struct click_container {
//    std::vector<int> leftContainer;
//    std::vector<int> rightContainer;
//
//    void AddLeftClick(int times)
//    {
//        leftContainer.push_back(times);
//    }
//
//    void AddRightClick(int times)
//    {
//        rightContainer.push_back(times);
//    }
//
//    void processClick()
//    {
//        for (int i = 0; i < leftContainer.size(); i++)
//        {
//            leftContainer[i] -= 1;
//            if (leftContainer[i] == 0)
//            {
//                leftContainer.erase(leftContainer.begin() + i);
//            }
//        }
//        for (int i = 0; i < rightContainer.size(); i++)
//        {
//            rightContainer[i] -= 1;
//            if (rightContainer[i] == 0)
//            {
//                rightContainer.erase(rightContainer.begin() + i);
//            }
//        }
//    }
//
//    int GetLeftCPS()
//    {
//        int left = (int)leftContainer.size();
//        return left;
//    }
//    int GetRightCPS()
//    {
//        int right = (int)rightContainer.size();
//        return right;
//    }
//};

typedef std::chrono::steady_clock::time_point click;

struct click_container {
    std::deque<click> leftContainer;
    std::deque<click> rightContainer;

    void AddLeftClick()
    {
        leftContainer.push_back(std::chrono::steady_clock::now());
    }

    void AddRightClick()
    {
        rightContainer.push_back(std::chrono::steady_clock::now());
    }

    void processClick()
    {
        if (leftContainer.size() > 0)
        {
            auto now = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - leftContainer.front()).count();
            if (diff > 1000)
            {
                leftContainer.pop_front();
            }
        }
        if (rightContainer.size() > 0)
        {
            auto now = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - rightContainer.front()).count();
            if (diff > 1000)
            {
                rightContainer.pop_front();
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
        name = u8"CPS检测";
        description = u8"检测左右键CPS";
        icon = "!";
        updateIntervalMs = 2;
        lastUpdateTime = std::chrono::steady_clock::now();
        //lastCpsTime = std::chrono::steady_clock::now();
        Reset();
    } 
    ~CPSDetector() {}

    static CPSDetector& Instance() {
        static CPSDetector instance;
        return instance;
    }

    void Toggle() override;
    void Reset() override
    {
        isEnabled = true;
    }
    void Update() override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) override;

    int GetLeftCPS();
    int GetRightCPS();
private:
    KeyState keyStateHelper;
    click_container cps;
    //// 检查是否到了更新的时间
    //bool ShouldCpsUpdate();

    //// 更新操作
    //void MarkUpCPSdated();
    //int cpsIntervalMs = 50;
    //std::chrono::steady_clock::time_point lastCpsTime;  // 记录最后更新时间
};