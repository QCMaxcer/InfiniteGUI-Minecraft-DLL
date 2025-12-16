#include "Text.h"
#include "ImGuiStd.h"

void Text::Toggle()
{
}

void Text::DrawContent()
{
    ImVec4 targetTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    //获取io
    ImGuiIO& io = ImGui::GetIO();
    //计算速度
    float speed = 3.0f * io.DeltaTime;
    color.color = ImLerp(color.color, targetTextColor, speed);
    ImGuiStd::TextColoredShadow(color.color, (prefix + text + suffix).c_str());
}

void Text::DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
{
    ImGui::SetCursorPosX(bigPadding);

    ImGui::Checkbox(u8"启用", &isEnabled);

    ImGui::SetCursorPosX(bigPadding);
    ImGuiStd::InputTextStd(u8"文本内容", text);
    DrawAffixSettings(bigPadding, centerX, itemWidth);
    DrawWindowSettings(bigPadding, centerX, itemWidth);
}

void Text::Load(const nlohmann::json& j)
{
    LoadAffix(j);
    LoadWindow(j);
    if (j.contains("Enabled")) isEnabled = j["Enabled"];
    if (j.contains("text")) text = j["text"];
}

void Text::Save(nlohmann::json& j) const
{
    SaveWindow(j);
    SaveAffix(j);
    j["Enabled"] = isEnabled;
    j["text"] = text;
}