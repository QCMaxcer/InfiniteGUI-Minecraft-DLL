#pragma once
#include <nlohmann/json.hpp>
#include <Windows.h>

class GlobalConfig {
public:
    float roundCornerRadius = 4.0f;  // 全局窗口圆角半径
    int menuKey = VK_OEM_6;  // 全局菜单快捷键
    std::string fontPath = "default";  // 全局语言

    // 在这里加更多全局参数…

    void Load(const nlohmann::json& j);
    void Save(nlohmann::json& j) const;
};