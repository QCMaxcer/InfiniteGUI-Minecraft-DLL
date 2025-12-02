#pragma once
#include <Windows.h>

#include "imgui\imgui.h"
#include "Item.h"
#include "WindowModule.h"
#include "AffixModule.h"
#include "UpdateModule.h"
#include "KeybindModule.h"
#include "SoundModule.h"
#include <string>
#include <chrono>

enum SprintState {
    Idle,
    Sprinting,
    Sneaking,
    Walking
};

struct sprint_element {
    ImVec4 color;
};

class Sprint : public WindowModule, public UpdateModule, public KeybindModule, public AffixModule, public Item, public SoundModule
{
public:
    Sprint() {

        type = Util; // 信息项类型
        multiType = Singleton;    // 信息项是否可以多开
        isEnabled = false; // 是否启用
        name = u8"强制疾跑";
        description = u8"强制疾跑";
        isPlaySound = true;
        soundVolume = 0.5f;

        keybinds.insert(std::make_pair(u8"激活键：", 'I'));
        keybinds.insert(std::make_pair(u8"前进键：", 'W'));
        keybinds.insert(std::make_pair(u8"疾跑键：", VK_CONTROL));
        keybinds.insert(std::make_pair(u8"潜行键：", VK_SHIFT));

        prefix = u8"[";
        suffix = "]";

        updateIntervalMs = 5;
        lastUpdateTime = std::chrono::steady_clock::now();
    }

    static Sprint& Instance() {
        static Sprint instance;
        return instance;
    }

    void Toggle() override;
    void OnKeyEvent(bool state, bool isRepeat, WPARAM key) override;

    void Update() override;
    void DrawContent() override;
    void DrawSettings() override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;

private:

    void GetWalking();
    void GetSneaking();
    void SetSprinting();
    bool isActivated = false;
    bool isWalking = false;
    SprintState lastState = Idle;
    SprintState state = Idle;

    sprint_element color = { ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text)) };
};