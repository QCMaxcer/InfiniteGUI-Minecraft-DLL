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
        multiType = MultiInstance;    // 信息项是否可以多开
        name = u8"文件数量显示";
        description = u8"显示文件夹内文件数量";
        isPlaySound = true;
        soundVolume = 0.5f;
        updateIntervalMs = 1000;
        lastUpdateTime = std::chrono::steady_clock::now();

        prefix = "[";
        suffix = u8"个文件]";

    }

    void Toggle() override;
    void Update() override;
    void DrawContent() override;
    void DrawSettings() override;
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