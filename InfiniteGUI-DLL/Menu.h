#pragma once
#include "Item.h"
#include "RenderModule.h"
#include "WindowStyleModule.h"
#include "KeybindModule.h"
#include "Blur.h"
#include "SettingMenu.h"
enum MenuState {
    MENU_STATE_MAIN,
    MENU_STATE_SETTINGS,
};

enum SETTING_STATE {
    SETTING_STATE_MODULES,
    SETTING_STATE_GENERAL,
    SETTING_STATE_ABOUT,
};

struct panel_element {
    float state = 0.0f;
    float blurriness = 0;
};

class Menu : public RenderModule, public WindowStyleModule , public Item, public KeybindModule {
public:

    Menu() {
        type = Hud; // 信息项类型
        name = u8"菜单";
        description = u8"显示菜单";
        icon = "J";
        isEnabled = false;
        settingMenu = new SettingMenu();
        blur = new Blur();
        Reset();
    }
    ~Menu() {
        delete settingMenu;
        delete blur;
    }
    panel_element panelAnim;

    MenuState state = MENU_STATE_MAIN;
    SETTING_STATE setting_state = SETTING_STATE_MODULES;

    static Menu& Instance()
    {
        static Menu instance;
        return instance;
    }

    void Render() override;
    void Toggle() override;
    void Reset() override
    {
        ResetKeybind();
        ResetWindowStyle();

        keybinds.insert(std::make_pair(u8"菜单快捷键：", VK_OEM_5));
        itemStyle.fontSize = 24.0f;
        itemStyle.bgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.15f);
    }
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;
    void OnKeyEvent(bool state, bool isRepeat, WPARAM key) override;
    int GetKeyBind();
private:
    void ShowMain();
    void ShowSidePanels() const;
    void ShowSettings(bool* done);
    void OldSettings();
    void DrawItemList() const;
    void DrawItemEditor(Item* item, const float& bigPadding, const float& centerX, const float& itemWidth) const;
    Blur * blur;
    SettingMenu * settingMenu;
    bool initialized = false;
};