#pragma once
#include <string>
#include "imgui/imgui.h"
#include <nlohmann/json.hpp>
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#include "GlobalConfig.h"
#include "WindowSnapper.h"
#include <chrono>

enum ItemType {
    Hud,
    Util,
    Hidden
};

enum MultiType {
    Singleton,
    MultiInstance
};

class Item {
public:
    Item() {
    }
    virtual ~Item() {
    }

    // ---------------------------
    //   必须被子类实现的接口
    // ---------------------------
    virtual void Toggle() = 0;
    virtual void Load(const nlohmann::json& j) = 0;
    virtual void Save(nlohmann::json& j) const = 0;
    virtual void DrawSettings() = 0;
    bool IsMultiInstance() const
    {
        return multiType == MultiType::MultiInstance;
    }

    void DrawItemSettings()
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
    MultiType multiType = MultiType::Singleton; // 信息项是否可以多开
    std::string name = "Item"; // 信息项名称
    std::string description = "No description"; // 信息项描述
    //std::string icon; // 信息项图标路径
};