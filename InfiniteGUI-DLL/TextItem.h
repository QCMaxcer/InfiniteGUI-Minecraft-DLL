#pragma once
#include "Item.h"
#include "WindowModule.h"
#include "nlohmann/json.hpp"
#include "Text.h"

class TextItem : public Item, public RenderModule
{
public:
    TextItem() {
        type = Hud; // 信息项类型
        name = u8"文本显示";
        description = u8"显示一段文本";
        icon = "(";
        Reset();
    }

    static TextItem& Instance() {
        static TextItem text;
        return text;
    }

    void Toggle() override;
    void Reset() override
    {
        isEnabled = false;
        texts.clear();
        texts.emplace_back();
    }
    void Render() override;
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;
private:
    std::vector<Text> texts;
};