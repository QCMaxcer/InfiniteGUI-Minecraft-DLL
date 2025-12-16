#pragma once
#include "AffixModule.h"
#include "WindowModule.h"
#include "nlohmann/json.hpp"
#include <string>

struct text_element {
    ImVec4 color;
};


class Text : public WindowModule, public AffixModule
{
public:
    Text() {
        Reset();
    }

    void Toggle();
    void Reset()
    {
        ResetAffix();
        ResetWindow();
        text = u8"«Î ‰»ÎŒƒ±æ";
    }
    void DrawContent() override;
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth);
    void Load(const nlohmann::json& j);
    void Save(nlohmann::json& j) const;
    std::string GetText() const { return text; }
    void SetText(const std::string& newText) { text = newText; }
    bool GetEnabled() const { return isEnabled; }
private:
    bool isEnabled = true;
    std::string text;
    text_element color;
};