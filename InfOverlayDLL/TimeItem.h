#pragma once
#include "Item.h"
#include "AffixModule.h"
#include "UpdateModule.h"
#include "WindowModule.h"
#include "nlohmann/json.hpp"

#include <string>
#include <chrono>
#include <ctime>

class TimeItem : public Item, public AffixModule , public UpdateModule, public WindowModule {
public:
    TimeItem() {
        type = Hud; // 信息项类型
        multiType = Singleton;    // 信息项是否可以多开
        isEnabled = false;
        name = u8"时间显示";
        description = u8"显示当前时间";
        currentTimeStr = u8"正在获取系统时间...";
        updateIntervalMs = 1000;
        lastUpdateTime = std::chrono::steady_clock::now();
    }
    //Instance()
    static TimeItem& Instance() {
        static TimeItem instance;
        return instance;
    }

    void Toggle() override;
    void Update() override;
    void DrawContent() override;
    void DrawSettings() override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;

private:
    std::string currentTimeStr;
};