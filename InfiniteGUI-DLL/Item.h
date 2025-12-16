#pragma once
#include <string>
#include "imgui/imgui.h"
#include <nlohmann/json.hpp>
#pragma comment(lib, "dwmapi.lib")

enum GameState {
    InGameMenu,
    InMenu,
    InGame,
};

enum ItemType{
    Hud,
    Util,
    Visual,
    Hidden
}; //改成可同时拥有多个类型

enum MultiType {
    Singleton,
    MultiInstance
};

class Item {
public:
    Item() = default;
    virtual ~Item() = default;

    // ---------------------------
    //   必须被子类实现的接口
    // ---------------------------
    virtual void Toggle() = 0;
    virtual void Reset() = 0;
    virtual void Load(const nlohmann::json& j) = 0;
    virtual void Save(nlohmann::json& j) const = 0;
    virtual void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) = 0;

    void DrawItemSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
    {
        if(ImGui::Checkbox(u8"启用", &isEnabled)) Toggle();
    }

    void LoadItem(const nlohmann::json& j)
    {
        if (j.contains("isEnabled")) isEnabled = j["isEnabled"];
    }
    void SaveItem(nlohmann::json& j) const
    {
        j["type"] = name;
        j["isEnabled"] = isEnabled;
    }

    bool isEnabled = true; // 是否启用该信息项
    ItemType type = ItemType::Hud; // 信息项类型
    std::string name = "Item"; // 信息项名称
    std::string description = "No description"; // 信息项描述
    std::string icon = "R"; // 信息项图标路径
    //std::string icon; // 信息项图标路径
};