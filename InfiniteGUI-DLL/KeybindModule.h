#pragma once

#include <Windows.h>
#include <vector>
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include "ImguiStd.h"
class KeybindModule
{
public:
	virtual void OnKeyEvent(bool state, bool isRepeat, WPARAM key) = 0;
    void DrawKeybindSettings(const float& bigPadding,const float& centerX,const float& itemWidth)
    {
        ImGui::PushFont(NULL, ImGui::GetFontSize() * 0.8f);
        ImGui::BeginDisabled();
        ImGuiStd::TextShadow(u8"按键绑定设置");
        ImGui::EndDisabled();
        ImGui::PopFont();

        // 当前起始 Y
        float startY = ImGui::GetCursorPosY();
        float itemHeight = ImGui::GetFrameHeightWithSpacing();

        int index = 0;

        for (auto& [name, key] : keybinds)
        {
            bool isLeft = (index % 2 == 0);

            float x = isLeft
                ? bigPadding
                : centerX + bigPadding;

            float y = startY + (index / 2) * itemHeight;

            ImGui::SetCursorPos(ImVec2(x, y));
            ImGui::SetNextItemWidth(itemWidth);

            ImGuiStd::Keybind(name.c_str(), key);
            index++;
        }

    }
protected:
    void LoadKeybind(const nlohmann::json& j)
    {
        for (auto& [name, key] : keybinds)
        {
            if (j.contains(name))
            {
                key = j[name];
            }
        }
    }
    void SaveKeybind(nlohmann::json& j) const
    {
        for (auto& [name, key] : keybinds)
        {
            j[name] = key;
        }
    }

    void ResetKeybind()
    {
        keybinds.clear();
    }

	std::map<std::string, int> keybinds; //name, key
};