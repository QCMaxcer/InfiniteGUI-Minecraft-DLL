#pragma once
#include "Item.h"
#include "AffixModule.h"
#include "WindowModule.h"
#include "SoundModule.h"
#include "KeybindModule.h"
#include <string>

struct counter_element {
    ImVec4 color;
};

class CounterItem : public Item, public AffixModule, public WindowModule, public SoundModule, public KeybindModule
{
public:
    CounterItem() {
        type = Hud; // 信息项类型
        multiType = MultiInstance;    // 信息项是否可以多开
        name = u8"计数器";
        description = u8"显示计数器";
        isPlaySound = true;
        soundVolume = 0.5f;

        keybinds.insert(std::make_pair(u8"增加快捷键：", VK_F6));
        keybinds.insert(std::make_pair(u8"减少快捷键：", VK_F5));
        keybinds.insert(std::make_pair(u8"清空快捷键：", NULL));

        prefix = u8"[计数:";
        suffix = "]";
    }
    ~CounterItem() {}


    void Toggle() override;
    void OnKeyEvent(bool state, bool isRepeat, WPARAM key) override;
    void DrawContent() override;
    void DrawSettings() override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;

private:
    int count = 0;
    int lastCount = 0;

    counter_element color = { ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text)) };
};