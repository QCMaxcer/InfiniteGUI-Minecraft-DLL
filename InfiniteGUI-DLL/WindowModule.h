#pragma once
#include "WindowStyleModule.h"
#include "GlobalWindowStyle.h"
#include <string>
#include "imgui/imgui.h"
#include "ImGuiStd.h"
#include <nlohmann/json.hpp>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#include "GlobalConfig.h"
#include "WindowSnapper.h"
#include "App.h"
#include "opengl_hook.h"
static const float SNAP_DISTANCE = 15.0f;
static bool isSnapping = false;

struct Customize
{
    bool windowRounding = false;
    bool fontSize = false;
    bool fontColor = false;
    bool bgColor = false;
    bool borderColor = false;
};

struct ItemStylePtr
{
    float* windowRounding;
    float* fontSize;
    ImVec4* fontColor;
    ImVec4* bgColor;
    ImVec4* borderColor;
};


class WindowModule : public WindowStyleModule
{
public:

    WindowModule()
    {
        SetStyle();
    }
    virtual void DrawContent() = 0;       // 绘制内容（文本、图形等）

private:
    void SetStyle()
    {
        if (custom.windowRounding)
            itemStylePtr.windowRounding = &itemStyle.windowRounding;
        else
            itemStylePtr.windowRounding = &GlobalWindowStyle::Instance().GetGlobeStyle().windowRounding;
        if (custom.fontSize)
            itemStylePtr.fontSize = &itemStyle.fontSize;
        else
            itemStylePtr.fontSize = &GlobalWindowStyle::Instance().GetGlobeStyle().fontSize;
        if (custom.fontColor)
            itemStylePtr.fontColor = &itemStyle.fontColor;
        else
            itemStylePtr.fontColor = &GlobalWindowStyle::Instance().GetGlobeStyle().fontColor;
        if (custom.bgColor)
            itemStylePtr.bgColor = &itemStyle.bgColor;
        else
            itemStylePtr.bgColor = &GlobalWindowStyle::Instance().GetGlobeStyle().bgColor;
        if (custom.borderColor)
            itemStylePtr.borderColor = &itemStyle.borderColor;
        else
            itemStylePtr.borderColor = &GlobalWindowStyle::Instance().GetGlobeStyle().borderColor;
    }

    ImVec4* EditWindowColor(const char* label, ImVec4* color,ImVec4* globalColorPtr, bool& custom, ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoLabel)
    {
        if (ImGuiStd::EditColor(label, *color))
        {
            custom = true;
        }
        if(custom)
        {
            ImGui::SameLine();
            ImGui::PushFont(App::Instance().iconFont);
            if (ImGui::Button((u8"\uE02E" + std::string("##") + label).c_str()))
            {
                custom = false;
                *color = *globalColorPtr;
            }
            ImGui::PopFont();
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(u8"使用全局样式");
            }
            return color;
        }
        else
            return globalColorPtr;
    }
public:

    void DrawWindowSettings()
    {
        ImGui::Checkbox(u8"固定", &clickThrough);

        ImGui::Checkbox(u8"自定义窗口大小", &isCustomSize);
        if (isCustomSize) {
            ImGui::InputFloat(u8"宽度", &width, 1.0f, 1.0f, "%.1f");
            ImGui::InputFloat(u8"高度", &height, 1.0f, 1.0f, "%.1f");
        }
        ImGui::InputFloat(u8"窗口 X", &x, 1.0f, 1.0f, "%.1f");
        ImGui::InputFloat(u8"窗口 Y", &y, 1.0f, 1.0f, "%.1f");


        if (ImGui::SliderFloat(u8"窗口圆角", &itemStyle.windowRounding, 0.0f, 10.0f, "%.1f"))
        {
            custom.windowRounding = true;
            itemStylePtr.windowRounding = &itemStyle.windowRounding;
        }
        if (custom.windowRounding)
        {
            ImGui::SameLine();
            ImGui::PushFont(App::Instance().iconFont);
            if (ImGui::Button((u8"\uE02E" + std::string("##windowRounding")).c_str()))
            {
                custom.windowRounding = false;
                itemStyle.windowRounding = GlobalWindowStyle::Instance().GetGlobeStyle().windowRounding;
                itemStylePtr.windowRounding = &GlobalWindowStyle::Instance().GetGlobeStyle().windowRounding;
            }
            ImGui::PopFont();
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(u8"使用全局样式");
            }
        }
        if(ImGui::InputFloat(u8"字体大小", &itemStyle.fontSize, 1.0f, 1.0f, "%.1f"))
        {
            custom.fontSize = true;
            itemStylePtr.fontSize = &itemStyle.fontSize;
        } 
        if (custom.fontSize)
        {
            ImGui::SameLine();
            ImGui::PushFont(App::Instance().iconFont);
            if (ImGui::Button((u8"\uE02E" + std::string("##fontSize")).c_str()))
            {
                custom.fontSize = false;
                itemStyle.fontSize = GlobalWindowStyle::Instance().GetGlobeStyle().fontSize;
                itemStylePtr.fontSize = &GlobalWindowStyle::Instance().GetGlobeStyle().fontSize;
            }
            ImGui::PopFont();
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(u8"使用全局样式");
            }
        }
        ImVec4* colors = ImGui::GetStyle().Colors;
        itemStylePtr.fontColor = EditWindowColor(u8"字体颜色", &itemStyle.fontColor, &GlobalWindowStyle::Instance().GetGlobeStyle().fontColor, custom.fontColor);
        itemStylePtr.bgColor = EditWindowColor(u8"背景颜色", &itemStyle.bgColor, &GlobalWindowStyle::Instance().GetGlobeStyle().bgColor, custom.bgColor);
        itemStylePtr.borderColor = EditWindowColor(u8"边框颜色", &itemStyle.borderColor, &GlobalWindowStyle::Instance().GetGlobeStyle().borderColor, custom.borderColor);
    }

    // ---------------------------
//   渲染整个窗口（统一逻辑）
// ---------------------------
    virtual void RenderWindow()
    {
        if (!isWindowShow) return;
        if (!isCustomSize)
            //ImGui::SetNextWindowSizeConstraints(ImVec2(10, 10), ImVec2(2560, 1440));
            ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);

        if (!isMoving)
        {
            ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
            if (isCustomSize)
                ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
        }
        HWND g_hwnd = App::Instance().clientHwnd;

        if (isTransparentBg)
        {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // 背景透明
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // 边框透明
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, *itemStylePtr.bgColor); // 背景透明
            ImGui::PushStyleColor(ImGuiCol_Border, *itemStylePtr.borderColor); // 边框透明
        }
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, *itemStylePtr.windowRounding);
        ImGui::PushStyleColor(ImGuiCol_Text, *itemStylePtr.fontColor); // 字体颜色
        ImGuiWindowFlags flags = 0;
        //if (!allowResize) flags |= ImGuiWindowFlags_NoResize;
        //if (!allowMove)   flags |= ImGuiWindowFlags_NoMove;
        if (!isCustomSize) flags |= ImGuiWindowFlags_NoResize;
        flags |= ImGuiWindowFlags_NoTitleBar;
        flags |= ImGuiWindowFlags_NoScrollbar;
        flags |= ImGuiWindowFlags_NoSavedSettings;


        if (clickThrough) flags |= ImGuiWindowFlags_NoInputs;

        ImGui::Begin(GetActualWindowName().c_str(), nullptr, flags);

        isMoving = ImGui::IsMouseDragging(0, 1.0f) && ImGui::IsWindowFocused();

        isHovered = ImGui::IsWindowHovered();

        ImGui::PushFont(NULL, *itemStylePtr.fontSize);
        DrawContent();
        ImGui::PopFont();

        // 判断拖动/修改大小事件
        if (isMoving)
        {
            HandleDrag(g_hwnd);
        }
        else
        {
            // 当前窗口位置大小
            ImVec2 pos = ImGui::GetWindowPos();
            ImVec2 sz = ImGui::GetWindowSize();

            WindowSnapper::KeepSnapped(pos, sz, (float)opengl_hook::screen_size.x, (float)opengl_hook::screen_size.y, snapState);
            // 设置吸附后的位置
            ImGui::SetWindowPos(pos, ImGuiCond_Always);

            // 保存到 Item
            x = pos.x;
            y = pos.y;
        }

        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
    }
private:
        std::string GetActualWindowName() const {
            return "##" + std::to_string((uintptr_t)this);
        }

        void HandleDrag(HWND g_hwnd)
        {
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
                isSnapping = true;
            else
                isSnapping = false;

            // 当前窗口位置大小
            ImVec2 pos = ImGui::GetWindowPos();
            ImVec2 sz = ImGui::GetWindowSize();

            SnapResult snap;
            if (isSnapping)
            {
                // 计算吸附
                snap = WindowSnapper::ComputeSnap(pos, sz, (float)opengl_hook::screen_size.x, (float)opengl_hook::screen_size.y, SNAP_DISTANCE);
                // 画吸附线
                WindowSnapper::DrawGuides(snap, (float)opengl_hook::screen_size.x, (float)opengl_hook::screen_size.y);
            }
            else
                snap = WindowSnapper::ComputeSnap(pos, sz, (float)opengl_hook::screen_size.x, (float)opengl_hook::screen_size.y, 0.0f);

            // 设置吸附后的位置
            ImGui::SetWindowPos(snap.snappedPos, ImGuiCond_Always);

            // 保存到 Item
            x = snap.snappedPos.x;
            y = snap.snappedPos.y;
            snapState = snap.snapState;

            //保存窗口大小
            width = ImGui::GetWindowSize().x;
            height = ImGui::GetWindowSize().y;
        }

protected:
    void LoadWindow(const nlohmann::json& j)
    {
        if (j.contains("isCustomSize")) isCustomSize = j["isCustomSize"];
        if (j.contains("x")) x = j["x"];
        if (j.contains("y")) y = j["y"];
        if (j.contains("width")) width = j["width"];
        if (j.contains("height")) height = j["height"];

        if (j.contains("clickThrough")) clickThrough = j["clickThrough"];

        if (j.contains("snapState")) snapState = j["snapState"];

        if (j.contains("custom"))
        {
            custom.windowRounding = j["custom"]["windowRounding"];
            custom.fontSize = j["custom"]["fontSize"];
            custom.fontColor = j["custom"]["fontColor"];
            custom.bgColor = j["custom"]["bgColor"];
            custom.borderColor = j["custom"]["borderColor"];
        }
        SetStyle();
        LoadStyle(j);
    }
    void SaveWindow(nlohmann::json& j) const
    {
        j["isCustomSize"] = isCustomSize;
        j["x"] = x;
        j["y"] = y;
        j["width"] = width;
        j["height"] = height;

        j["clickThrough"] = clickThrough;

        j["snapState"] = snapState;

        j["custom"] = {
            {"windowRounding", custom.windowRounding},
            {"fontSize", custom.fontSize},
            {"fontColor", custom.fontColor},
            {"bgColor", custom.bgColor},
            {"borderColor", custom.borderColor}
        };
        SaveStyle(j);
    }

    bool isWindowShow = true;

    bool isCustomSize = false;    //是否自定义大小
    float x = 100.0f;
    float y = 40.0f;

    SnapState snapState = SNAP_NONE;
    float width = 250.0f;
    float height = 80.0f;

    bool allowMove = true;
    bool allowResize = true;
    bool clickThrough = false;

    bool isMoving = false;
    bool isHovered = true;

    bool isTransparentBg = false;

    Customize custom;
    ItemStylePtr itemStylePtr;
};