#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "ImGuiStd.h"
class AffixModule
{
public:

    void DrawAffixSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
    {
        ImGui::PushFont(NULL, ImGui::GetFontSize() * 0.8f);
        ImGui::BeginDisabled();
        ImGuiStd::TextShadow(u8"前后缀设置");
        ImGui::EndDisabled();
        ImGui::PopFont();

        // 当前 Y 位置
        float startY = ImGui::GetCursorPosY();

        // ===== 左：前缀 =====
        ImGui::SetCursorPos(ImVec2(bigPadding, startY));
        ImGui::SetNextItemWidth(itemWidth);
        ImGuiStd::InputTextStd(u8"前缀", prefix);

        // ===== 右：后缀（以窗口中心线 + 边距为起点）=====
        ImGui::SetCursorPos(ImVec2(centerX + bigPadding, startY));
        ImGui::SetNextItemWidth(itemWidth);
        ImGuiStd::InputTextStd(u8"后缀", suffix);
    }
protected:
    void LoadAffix(const nlohmann::json& j)
    {
        if (j.contains("prefix")) prefix = j["prefix"];
        if (j.contains("suffix")) suffix = j["suffix"];
    }
    void SaveAffix(nlohmann::json& j) const
    {
        j["prefix"] = prefix;
        j["suffix"] = suffix;
    }

    void ResetAffix()
    {
        prefix = "[";
        suffix = "]";
    }

	std::string prefix = "[";
	std::string suffix = "]";
};