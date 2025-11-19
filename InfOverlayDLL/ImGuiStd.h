#pragma once
#include "imgui/imgui.h"
#include <string>
#include "VK_Keymap.h"
#include <Windows.h>
#include <map>
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
        ImGui::SetCursorPos(ImVec2(pos.x + 1, pos.y + 1));
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
        ImGui::SetCursorPos(ImVec2(pos.x + 1, pos.y + 1));
        ImGui::TextColored(ImVec4(0, 0, 0, 0.6f), text);  // 阴影层

        ImGui::SetCursorPos(pos);
        ImGui::TextColored(color, text);  // 正常文字
    }
    struct keybind_element
    {
        ImGuiID id;
        bool binding = false;
    };
    static ImGuiID current_id = 0;


    static void Keybind(const char* text, int &key)
    {
        static std::map<const char*, keybind_element> keybind_elements;
        if (keybind_elements.find(text) == keybind_elements.end())
        {
                keybind_element element;
                element.id = current_id++;
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

}
