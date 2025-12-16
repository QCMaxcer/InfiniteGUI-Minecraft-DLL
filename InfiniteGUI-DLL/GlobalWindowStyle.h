#pragma once
#include "Item.h"
#include "WindowStyleModule.h"

class GlobalWindowStyle : public WindowStyleModule, public Item {
public:
    GlobalWindowStyle() {
        type = Hidden; // 信息项类型
        name = u8"全局窗口样式";
        description = u8"设置全局窗口样式";
        icon = "L";
        Reset();
    }
    ~GlobalWindowStyle() {}

    static GlobalWindowStyle& Instance()
    {
        static GlobalWindowStyle instance;
        return instance;
    }

    void Toggle() override;
    void Reset() override
    {
        ResetWindowStyle();
        isEnabled = true;
    }
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) override;
    ItemStyle& GetGlobeStyle();
private:
};