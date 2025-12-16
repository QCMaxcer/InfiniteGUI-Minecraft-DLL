#pragma once

#include <Windows.h>
#include "imgui\imgui.h"
#include "Item.h"
#include "WindowModule.h"
#include "UpdateModule.h"
#include <string>
#include <chrono>

struct keystrokes_element {
    ImVec4 backgroundColor;
    ImVec4 fontColor;
};

enum key_type {
    normal,
    space,
    mouse
};

struct key_box
{
    key_type type;
    std::string label;
    bool state;
    float width;
    float height;
    bool needReturn;
    keystrokes_element color;
    int key;

    void SetSize(float min_box_size, float padding)
    {
        float space_width = min_box_size * 3 + padding * 2;
        float mouse_width = (space_width - padding) / 2.0f;
        switch (type)
        {
        case normal:
            width = min_box_size;
            this->height = min_box_size;
            break;
        case space:
            width = space_width;
            this->height = min_box_size / 2;
            break;
        case mouse:
            width = mouse_width;
            this->height = min_box_size;
            break;
        default:
            width = min_box_size;
            this->height = min_box_size;
            break;
        }
    }

    key_box(key_type type, std::string label, float min_box_size, float padding, bool needReturn, int key)
    {
        this->type = type;
        this->label = label;
        this->state = false;
        SetSize(min_box_size, padding);
        this->needReturn = needReturn;
        this->color = { ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_FrameBg)), ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text)) };
        this->key = key;
    }
};

class KeystrokesItem : public Item, public WindowModule, public UpdateModule
{
public:
    KeystrokesItem() {
        type = Hud; // 信息项类型
        name = u8"按键显示";
        description = u8"显示按键状态";
        icon = u8"\uE05D";
        updateIntervalMs = 5;
        lastUpdateTime = std::chrono::steady_clock::now();

        Reset();
    }
    ~KeystrokesItem() {}

    static KeystrokesItem& Instance() {
        static KeystrokesItem instance;
        return instance;
    }

    void Toggle() override;
    void Reset() override
    {
        ResetWindow();
        isTransparentBg = true;
        isCustomSize = false;
        isEnabled = false;
        key_boxes.clear();
        key_boxes.push_back(key_box(normal, "W", min_box_size, padding, true, 'W'));
        key_boxes.push_back(key_box(normal, "A", min_box_size, padding, false, 'A'));
        key_boxes.push_back(key_box(normal, "S", min_box_size, padding, false, 'S'));
        key_boxes.push_back(key_box(normal, "D", min_box_size, padding, true, 'D'));
        key_boxes.push_back(key_box(space, "-----", min_box_size, padding, true, VK_SPACE));
        key_boxes.push_back(key_box(mouse, "LMB", min_box_size, padding, false, VK_LBUTTON));
        key_boxes.push_back(key_box(mouse, "RMB", min_box_size, padding, true, VK_RBUTTON));
        showSpace = true;
        showMouse = true;
        showCps = false;
        padding = 6.0f;
        min_box_size = 32.0f;
    }
    void Update() override;
    void DrawContent() override;
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;

private:
    static void DrawSpaceLine(const char* text, const ImVec2& key_box_size);
    bool showSpace = true;
    bool showMouse = true;
    bool showCps = false;
    std::vector<key_box> key_boxes;
    float padding = 6.0f;
    float min_box_size = 32.0f;
};