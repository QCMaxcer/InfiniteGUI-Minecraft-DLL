#include "KeystrokesItem.h"
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "ImGuiStd.h"
#include <string>
#include <windows.h>
#include "App.h"
void KeystrokesItem::Toggle()
{
}
void KeystrokesItem::Update()
{
    if(opengl_hook::handle_window != GetForegroundWindow())
        return;
    for (auto& key_box : key_boxes)
    {
        if(GetAsyncKeyState(key_box.key) & 0x8000)
            key_box.state = true;
        else
            key_box.state = false;
    }
}

void KeystrokesItem::DrawContent()
{
    //获取io
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 window_padding = ImGui::GetStyle().WindowPadding;
    ImVec2 cursorPos;
    cursorPos.x = min_box_size + padding + window_padding.x;
    cursorPos.y = window_padding.y;
    ImGui::PushStyleColor(ImGuiCol_Border, *itemStylePtr.borderColor);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, *itemStylePtr.windowRounding);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, *itemStylePtr.bgColor);
    for (auto& key_box : key_boxes)
    {
        if(key_box.type == space && !showSpace) continue;
        if(key_box.type == mouse && !showMouse) continue;
        ImVec4 targetTextColor = key_box.state ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_Text);
        ImVec4 targetBgColor = key_box.state ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);

        //计算速度
        float speed_off = 15.0f * io.DeltaTime;
        float speed_on = speed_off * 2.5f;
        key_box.color.fontColor = ImLerp(key_box.color.fontColor, targetTextColor, key_box.state ? speed_on : speed_off);
        key_box.color.backgroundColor = ImLerp(key_box.color.backgroundColor, targetBgColor, key_box.state ? speed_on : speed_off);

        ImGui::SetCursorPos(cursorPos);

        //设置child的背景颜色
        ImGui::PushStyleColor(ImGuiCol_ChildBg, key_box.color.backgroundColor);
        ImGui::BeginChild(key_box.label.c_str(), ImVec2(key_box.width, key_box.height), true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);
        if (key_box.label == "-----")
            DrawSpaceLine(key_box.label.c_str(), ImVec2(key_box.width, key_box.height));
        else
        {
            ImVec2 text_size = ImGui::CalcTextSize(key_box.label.c_str());
            ImVec2 text_pos = ImVec2(
                (key_box.width - text_size.x) * 0.5f,
                (key_box.height - text_size.y) * 0.5f
            );
            ImGui::SetCursorPos(text_pos);
            ImGuiStd::TextColoredShadow(key_box.color.fontColor, key_box.label.c_str());
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();         //设置child的背景颜色
        if (key_box.needReturn)
        {
            cursorPos.y += key_box.height + padding;
            cursorPos.x = window_padding.x;
        }
        else
            cursorPos.x += key_box.width + padding;
    }
    ImGui::PopStyleVar(); //窗口圆角
    ImGui::PopStyleColor(2); //是否显示边框
}

void KeystrokesItem::DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
{
    //DrawItemSettings();
    ImGui::Checkbox(u8"显示空格", &showSpace);

    ImGui::Checkbox(u8"显示鼠标", &showMouse);
    ImGui::Checkbox(u8"显示CPS", &showCps);
    ImGui::Separator();
    bool needResize = false;
    if(ImGui::SliderFloat(u8"按键边长", &min_box_size, 12.0f, 108.0f, "%.1f"))
    {
        needResize = true;
    }
    if(ImGui::SliderFloat(u8"按键间隔", &padding, 1.0f, 18.0f, "%.1f"))
    {
        needResize = true;
    }
    if(needResize)
    {
        for (auto& key_box : key_boxes)
        {
            key_box.SetSize(min_box_size, padding);
        }
    }
    DrawWindowSettings(bigPadding, centerX, itemWidth);
}

void KeystrokesItem::Load(const nlohmann::json& j)
{
    LoadItem(j);
    if (j.contains("showSpace")) showSpace = j["showSpace"];
    if (j.contains("showMouse")) showMouse = j["showMouse"];
    if (j.contains("showCps")) showCps = j["showCps"];
    if (j.contains("min_box_size")) min_box_size = j["min_box_size"];
    if (j.contains("padding")) padding = j["padding"];
    for (auto& key_box : key_boxes)
    {
        key_box.SetSize(min_box_size, padding);
    }
    LoadWindow(j);
}

void KeystrokesItem::Save(nlohmann::json& j) const
{
    SaveItem(j);
    SaveWindow(j);
    j["showSpace"] = showSpace;
    j["showMouse"] = showMouse;
    j["showCps"] = showCps;
    j["min_box_size"] = min_box_size;
    j["padding"] = padding;
}

void KeystrokesItem::DrawSpaceLine(const char* text, const ImVec2& key_box_size)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 spaceFontSize = ImGui::CalcTextSize(text);
    ImVec2 text_pos = ImVec2(
        (key_box_size.x - spaceFontSize.x) * 0.5f,
        key_box_size.y * 0.5f
    );
    ImGui::SetCursorPos(text_pos);
    ImVec2 linePos1 = ImGui::GetCursorScreenPos();
    ImVec2 linePos2 = ImVec2(linePos1.x + spaceFontSize.x, linePos1.y);
    //shadow
    ImVec2 linePos3 = ImVec2(linePos1.x + 1.0f, linePos1.y + 1.0f);
    ImVec2 linePos4 = ImVec2(linePos2.x + 1.0f, linePos2.y + 1.0f);
    //shadow
    ImU32 shadowColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.6f));
    draw_list->AddLine(linePos3, linePos4, shadowColor, spaceFontSize.y * 0.1f);
    draw_list->AddLine(linePos1, linePos2, ImGui::GetColorU32(ImGuiCol_Text), spaceFontSize.y * 0.1f);
    ImGui::Dummy(ImVec2(0, 0));
}
