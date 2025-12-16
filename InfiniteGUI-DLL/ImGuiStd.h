#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <string>
#include "VK_Keymap.h"
#include <Windows.h>
#include <map>
#include <nlohmann/json.hpp>

#include "opengl_hook.h"

namespace ImGuiStd {

    static bool InputTextStd(const char* label, std::string& str, ImGuiInputTextFlags flags = 0)
    {
        char buffer[1024];
        strncpy(buffer, str.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;

        if (ImGui::InputText(label, buffer, sizeof(buffer), flags)) {
            str = buffer;
            return true;
        }
        return false;
    }

    static void TextShadow(const char* text, ImVec4 shadowColor, ImVec2 offset, ...)
    {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(pos.x + offset.x, pos.y + offset.y));
        ImGui::TextColored(shadowColor, text);  // 阴影层

        ImGui::SetCursorPos(pos);
        ImGui::Text(text);  // 正常文字
    }

    static void TextShadow(const char* text, ...)
    {
        ImVec2 pos = ImGui::GetCursorPos();
        float shadowOffset = /*ImGui::GetFontSize() * 0.08f*/ 1.0f;
        ImGui::SetCursorPos(ImVec2(pos.x + shadowOffset, pos.y + shadowOffset));
        ImGui::TextColored(ImVec4(0, 0, 0, 0.6f), text);  // 阴影层

        ImGui::SetCursorPos(pos);
        ImGui::Text(text);  // 正常文字
    }

    static void TextColoredShadow(ImVec4 color, ImVec4 shadowColor, const char* text, ImVec2 offset, ...)
    {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(pos.x + offset.x, pos.y + offset.y));
        ImGui::TextColored(shadowColor, text);  // 阴影层

        ImGui::SetCursorPos(pos);
        ImGui::TextColored(color, text);  // 正常文字
    }

    static void TextColoredShadow(ImVec4 color, const char* text, ...)
    {
        ImVec2 pos = ImGui::GetCursorPos();
        float shadowOffset =/* ImGui::GetFontSize() * 0.08f*/ 1.0f;
        ImGui::SetCursorPos(ImVec2(pos.x + shadowOffset, pos.y + shadowOffset));
        ImGui::TextColored(ImVec4(0, 0, 0, 0.6f), text);  // 阴影层

        ImGui::SetCursorPos(pos);
        ImGui::TextColored(color, text);  // 正常文字
    }
    struct keybind_element
    {
        ImGuiID id = 0;
        bool binding = false;
    };
    static ImGuiID current_keybind_element_id = 0;


    static void Keybind(const char* text, int &key)
    {
        static std::map<const char*, keybind_element> keybind_elements;
        if (keybind_elements.find(text) == keybind_elements.end())
        {
                keybind_element element;
                element.id = current_keybind_element_id++;
                keybind_elements[text] = element;
        }
        keybind_element& element = keybind_elements[text];


        std::string hotkeyStr = keys[key];

        ImGui::Text(text);

        ImGui::SameLine();

        if (ImGui::Button((hotkeyStr + "##" + std::to_string(element.id)).c_str(), ImVec2(100, 0)))
        {
            element.binding = true;
        }
        if (element.binding)
        {
            ImGui::OpenPopup(text);
        }
        if (ImGui::BeginPopup(text))
        {
            ImGui::Text(u8"绑定快捷键");
            ImGui::Separator();
            for (int i = 0; i < IM_ARRAYSIZE(keys); i++)
            {
                std::string keyStr = keys[i];
                if (keyStr[0] == '-')
                {
                    continue;
                }
                if (ImGui::Selectable(keyStr.c_str()))
                {
                    key = i;
                    hotkeyStr = keyStr;
                    element.binding = false;
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::IsMouseClicked(1))
                {
                    element.binding = false;
                    ImGui::CloseCurrentPopup();
                    break;
                }

                if (i == VK_LBUTTON || i == VK_RBUTTON || i == VK_MBUTTON || i == 0)
                {
                    continue;
                }
                if (GetAsyncKeyState(i) & 0x8000)
                {
                    key = i;
                    hotkeyStr = keyStr;
                    element.binding = false;
                    ImGui::CloseCurrentPopup();
                    break;
                }

            }
            ImGui::EndPopup();
        }
    }

    static void HelpMarker(const char* Text, ImVec4 Color = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled))
    {
	    ImGui::TextColored(Color, "(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(Text);
        }
    }
    static bool DrawCenteredButton(const char* label, const ImVec2& buttonSize)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 btnSize = buttonSize;
        // 计算按钮左侧应该空多少像素
        btnSize.x = (windowSize.x - buttonSize.x) * 0.5f;

        btnSize.y = (windowSize.y - buttonSize.y) * 0.5f;
        ImGui::SetCursorPos(ImVec2(btnSize.x, btnSize.y));
        // 绘制按钮
        if (ImGui::Button(label, buttonSize))
        {
            return true;
        }
        return false;
    }

    struct edit_color_element
    {
        ImGuiID id = 0;
        ImVec2 popup_size = ImVec2(0, 0);
    };
    static ImGuiID current_edit_color_element_id = 0;
    static bool EditColor(const char* label, ImVec4& color, ImVec4 refColor, ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoLabel)
    {
        bool changed = false;
        static std::map<const char*, edit_color_element> edit_color_elements;
        if (edit_color_elements.find(label) == edit_color_elements.end())
        {
            edit_color_element element;
            element.id = current_edit_color_element_id++;
            edit_color_elements[label] = element;
        }
        edit_color_element& element = edit_color_elements[label];\
        ImVec2 target_size = ImVec2(338 * ImGui::GetFontSize() / 20.0f, 369 * ImGui::GetFontSize() / 20.0f);
        float speed = 10.0f * ImGui::GetIO().DeltaTime;
        std::string text = label + std::string(u8"：");

        TextShadow(text.c_str());
        ImGui::SameLine();

        if (ImGui::ColorButton(label, color, flags))
        {
            ImGui::OpenPopup(label);
            element.popup_size = ImVec2(0, 0);
        }

        ImGui::SetNextWindowSize(element.popup_size, ImGuiCond_Always);
        if (ImGui::BeginPopup(label, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            element.popup_size = ImLerp(element.popup_size, target_size, speed);
            //TextShadow(label);
            //ImGui::Spacing();
            if (ImGui::ColorPicker4(label, (float*)&color, flags, (float*)&refColor))
            {
                changed = true;
            }
            ImGui::EndPopup();
        }
        if (memcmp(&color, &refColor, sizeof(ImVec4)) != 0)
        {
            ImGui::PushFont(opengl_hook::gui.iconFont);
            ImGui::SameLine(); if (ImGui::Button(("Z" + std::string(u8"##") + std::to_string(element.id)).c_str())) { color = refColor; changed = true; }
            ImGui::PopFont();
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(u8"还原初始样式");
            }

        }
        return changed;
    }

    static bool EditColor(const char* label, ImVec4& color, ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoLabel)
    {
        bool changed = false;
        static std::map<const char*, edit_color_element> edit_color_elements;
        if (edit_color_elements.find(label) == edit_color_elements.end())
        {
            edit_color_element element;
            element.id = current_edit_color_element_id++;
            edit_color_elements[label] = element;
        }
        edit_color_element& element = edit_color_elements[label];
            static ImVec2 target_size = ImVec2(338 * ImGui::GetFontSize() / 20.0f, 368 * ImGui::GetFontSize() / 20.0f);
        float speed = 10.0f * ImGui::GetIO().DeltaTime;
        std::string text = label + std::string(u8"：");

        TextShadow(text.c_str());
        ImGui::SameLine();

        if (ImGui::ColorButton(label, color, flags))
        {
            ImGui::OpenPopup(label);
            element.popup_size = ImVec2(0, 0);
        }

        ImGui::SetNextWindowSize(element.popup_size, ImGuiCond_Always);
        if (ImGui::BeginPopup(label, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            element.popup_size = ImLerp(element.popup_size, target_size, speed);
            if (ImGui::ColorPicker4(label, (float*)&color, flags))
            {
                changed = true;
            }
            ImGui::EndPopup();
        }
        return changed;
    }

    static void SaveImVec4(nlohmann::json& j, const char* key, const ImVec4& v)
    {
        if (!j.is_object()) j = nlohmann::json::object();

        j[key] = nlohmann::json::array(
            { v.x, v.y, v.z, v.w }
        );
    }

    static void LoadImVec4(const nlohmann::json& j, const char* key, ImVec4& v)
    {
        if (!j.is_object()) return;
        if(!j.contains(key)) return;
        const auto& arr = j[key];
        if (!arr.is_array() || arr.size() != 4) return;
            v.x = j[key][0].get<float>();
            v.y = j[key][1].get<float>();
            v.z = j[key][2].get<float>();
            v.w = j[key][3].get<float>();
    }

}
