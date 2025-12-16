#pragma once
#include "Item.h"
#include "AffixModule.h"
#include "SoundModule.h"
#include "UpdateModule.h"
#include "WindowModule.h"
#include <string>

struct file_count_element {
    ImVec4 color;
};

class FileCountItem : public Item, public AffixModule, public SoundModule, public UpdateModule, public WindowModule {
public:
    FileCountItem() {
        type = Hud; // 信息项类型
        name = u8"文件数量显示";
        description = u8"显示文件夹内文件数量";
        icon = u8"\uE02A";
        updateIntervalMs = 1000;
        lastUpdateTime = std::chrono::steady_clock::now();
        Reset();
    }

    static FileCountItem& Instance() {
        static FileCountItem text;
        return text;
    }

    void Toggle() override;
    void Reset() override
    {
        ResetAffix();
        ResetSound();
        ResetWindow();
        prefix = "[";
        suffix = u8"个文件]";
        fileCount = 0;
        lastFileCount = 0;
        folderPath = "C:\\Users";
        errorMessage = "";
        recursive = false;
        extensionFilter = "";
    }
    void Update() override;
    void DrawContent() override;
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;

private:
    size_t fileCount = 0;
    size_t lastFileCount = 0;

    // 配置项
    std::string folderPath = "C:\\Users";   // 默认路径
    std::string errorMessage = "";
    bool recursive = false;           // 是否递归扫描
    std::string extensionFilter = ""; // 例如 ".txt" 为空则全部文件

    file_count_element color = { ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text)) };
};