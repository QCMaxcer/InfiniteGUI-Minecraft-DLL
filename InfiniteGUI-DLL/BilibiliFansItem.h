#pragma once
#include "Item.h"
#include "AffixModule.h"
#include "SoundModule.h"
#include "UpdateModule.h"
#include "HttpModule.h"
#include "WindowModule.h"
#include <string>


struct bilibili_fans_element {
    ImVec4 color;
};

class BilibiliFansItem : public Item, public AffixModule, public SoundModule, public HttpModule, public UpdateModule, public WindowModule {
public:
    BilibiliFansItem() {
        type = Hud; // 信息项类型
        name = u8"粉丝数显示";
        description = u8"显示B站用户的粉丝数";
        icon = u8"\uE045";
        httpUpdateIntervalMs = 3000;
        updateIntervalMs = 50;
        lastUpdateTime = std::chrono::steady_clock::now();
        Reset();
    }

    ~BilibiliFansItem() override {
        if (httpTaskId != -1)
        {
            HttpRemoveTask();
        }
    }

    static BilibiliFansItem& Instance() {
        static BilibiliFansItem instance;
        return instance;
    }

    void Toggle() override;
    void Reset() override
    {
        ResetAffix();
        ResetSound();
        ResetWindow();
        isEnabled = false;
        prefix = u8"[粉丝数:";
        suffix = "]";
        if (httpTaskId != -1)
        {
            HttpRemoveTask();
        }
        uid = 399194206;
        fansCount = -1;
        lastFansCount = -1;
    }
    void HttpAddTask() override;
    void HttpRemoveTask() override;
    void Update() override;
    void DrawContent() override;
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;

private:
    long long uid = 399194206;          // B站用户UID
    std::atomic<int> pendingFans{ -1 };   // 后台线程写，主线程读
    int fansCount = -1;                 // 主线程内部值
    int lastFansCount = -1;             // 主线程上一帧值

    bilibili_fans_element color = { ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text)) };
};