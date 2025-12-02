#include "FileCountItem.h"
#include <filesystem>
#include "StringConverter.h"
#include "AudioManager.h"
#include "imgui\imgui_internal.h"
#include "ImGuiStd.h"
namespace fs = std::filesystem;

void FileCountItem::Toggle()
{
}

void FileCountItem::Update()
{
    size_t count = 0;
    std::wstring acp_folderPath = StringConverter::Utf8ToWstring(folderPath);
    std::wstring acp_extensionFilter = StringConverter::Utf8ToWstring(extensionFilter);

    // 如果路径不存在
    if (!fs::exists(acp_folderPath) || !fs::is_directory(acp_folderPath)) {
        fileCount = -1; // 表示错误
        return;
    }

    try {
        if (recursive) {
            for (auto& p : fs::recursive_directory_iterator(acp_folderPath)) {
                if (!fs::is_regular_file(p.status())) continue;

                if (!acp_extensionFilter.empty()) {
                    if (p.path().extension().wstring() != acp_extensionFilter)
                        continue;
                }

                count++;
            }
        }
        else {
            for (auto& p : fs::directory_iterator(acp_folderPath)) {
                if (!fs::is_regular_file(p.status())) continue;

                if (!acp_extensionFilter.empty()) {
                    if (p.path().extension().wstring() != acp_extensionFilter)
                        continue;
                }

                count++;
            }
        }

        fileCount = count;
    }
    catch (const fs::filesystem_error& e) {
        // 捕获文件系统相关的异常，并输出错误信息
        errorMessage = e.what();
        fileCount = -2;  // 错误码
    }
    catch (const std::exception& e) {
        // 捕获其他标准异常
        errorMessage = e.what();
        fileCount = -3;
    }
    catch (...) {
        // 捕获所有其他异常
        errorMessage = "未知错误";
        fileCount = -4;
    }

    if (fileCount < 0) {
        ImGuiStd::TextShadow(u8"路径无效或不可访问");
        return;
    }

    if (fileCount > lastFileCount)
    {
        color.color = ImVec4(0.1f, 1.0f, 0.1f, 1.0f); //绿色
        lastFileCount = fileCount;
        if (isPlaySound) AudioManager::Instance().playSound("filecount\\filecount_up.wav", soundVolume);
    }
    else if (fileCount < lastFileCount)
    {
        color.color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f); //红色
        lastFileCount = fileCount;
        if (isPlaySound) AudioManager::Instance().playSound("filecount\\filecount_down.wav", soundVolume);
    }

}

void FileCountItem::DrawContent()
{
    ImVec4 targetTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

    //获取io
    ImGuiIO& io = ImGui::GetIO();

    //计算速度
    float speed = 3.0f * io.DeltaTime;
    color.color = ImLerp(color.color, targetTextColor, speed);

    ImGuiStd::TextColoredShadow(color.color, (prefix + std::to_string(fileCount) + errorMessage + suffix).c_str());
}

void FileCountItem::DrawSettings()
{
    DrawItemSettings();
    ImGuiStd::InputTextStd(u8"文件夹路径", folderPath);
    ImGui::Checkbox(u8"递归扫描(包括子文件夹)", &recursive);
    ImGuiStd::InputTextStd(u8"扩展名过滤 (.txt)", extensionFilter);

    if (ImGui::CollapsingHeader(u8"通用设置"))
    {
        DrawWindowSettings();
        DrawAffixSettings();
        DrawSoundSettings();
    }
}

void FileCountItem::Load(const nlohmann::json& j)
{

    LoadItem(j);
    LoadAffix(j);
    LoadWindow(j);
    LoadSound(j);

    if (j.contains("folderPath")) folderPath = j["folderPath"];

    if (j.contains("recursive")) recursive = j["recursive"];
    if (j.contains("extensionFilter")) extensionFilter = j["extensionFilter"];

    if (j.contains("fileCount")) fileCount = j["fileCount"];
    lastFileCount = fileCount;

}

void FileCountItem::Save(nlohmann::json& j) const
{
    SaveItem(j);
    SaveAffix(j);
    SaveWindow(j);
    SaveSound(j);

    j["folderPath"] = folderPath;
    j["recursive"] = recursive;
    j["extensionFilter"] = extensionFilter;
    j["fileCount"] = fileCount;


}