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
    //if(App::Instance().clientHwnd != opengl_hook::handle_window)
    //    return;
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
    if (child_showBorder)  //是否显示边框
    {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.43f, 0.43f, 0.50f, 0.50f));
    }
    for (auto& key_box : key_boxes)
    {
        if(key_box.type == space && !showSpace) continue;
        if(key_box.type == mouse && !showMouse) continue;
        ImVec4 targetTextColor = key_box.state ? ImVec4(0.0f, 0.0f, 0.0f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_Text);
        ImVec4 targetBgColor = key_box.state ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);

        //计算速度
        float speed_off = 15.0f * io.DeltaTime;
        float speed_on = speed_off * 2.5f;
        key_box.color.fontColor = ImLerp(key_box.color.fontColor, targetTextColor, key_box.state ? speed_on : speed_off);
        key_box.color.backgroundColor = ImLerp(key_box.color.backgroundColor, targetBgColor, key_box.state ? speed_on : speed_off);

        ImGui::SetCursorPos(cursorPos);
        //设置child的背景透明度
        ImGui::SetNextWindowBgAlpha(child_alpha);

        //设置child的背景颜色
        ImGui::PushStyleColor(ImGuiCol_ChildBg, key_box.color.backgroundColor);
        ImGui::BeginChild(key_box.label.c_str(), ImVec2(key_box.width, key_box.height), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);
        ImVec2 text_size = ImGui::CalcTextSize(key_box.label.c_str());
        ImVec2 text_pos = ImVec2(
            (key_box.width - text_size.x) * 0.5f,
            (key_box.height - text_size.y) * 0.5f
        );
        ImGui::SetCursorPos(text_pos);
        ImGuiStd::TextColoredShadow(key_box.color.fontColor, key_box.label.c_str());
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
    if (child_showBorder) ImGui::PopStyleColor(); //是否显示边框
}

void KeystrokesItem::DrawSettings()
{
    DrawItemSettings();
    ImGui::Checkbox(u8"显示空格", &showSpace);

    ImGui::Checkbox(u8"显示鼠标", &showMouse);
    ImGui::Checkbox(u8"显示CPS", &showCps);
    ImGui::Separator();
    ImGui::Checkbox(u8"显示边框", &child_showBorder);
    ImGui::InputFloat(u8"字体大小", &fontSize, 1.0f, 1.0f, "%.1f");
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
    ImGui::SliderFloat(u8"背景透明度", &child_alpha, 0.0f, 1.0f, "%.1f");
    //if (ImGui::CollapsingHeader(u8"通用设置"))
    //{
    //    DrawWindowSettings();
    //}
}

void KeystrokesItem::Load(const nlohmann::json& j)
{
    LoadItem(j);
    if (j.contains("showSpace")) showSpace = j["showSpace"];
    if (j.contains("showMouse")) showMouse = j["showMouse"];
    if (j.contains("showCps")) showCps = j["showCps"];
    if (j.contains("min_box_size")) min_box_size = j["min_box_size"];
    if (j.contains("padding")) padding = j["padding"];
    if (j.contains("child_alpha")) child_alpha = j["child_alpha"];
    if (j.contains("child_showBorder")) child_showBorder = j["child_showBorder"];
    if (j.contains("fontSize")) fontSize = j["fontSize"];

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
    j["child_alpha"] = child_alpha;
    j["child_showBorder"] = child_showBorder;
    j["fontSize"] = fontSize;
}