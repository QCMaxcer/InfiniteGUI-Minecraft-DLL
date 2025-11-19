#include "MainUI.h"
#include "imgui/imgui.h"
#include "ImGuiStd.h"
#include "App.h"
#include "ConfigManager.h"
#include "GlobalConfig.h"
#include "FileManager.h"
#include "StringConverter.h"

struct FontInfo {
    std::string name;
    std::wstring path;
};

std::vector<FontInfo> GetFontsFromDirectory(const std::wstring& directory) {
    std::vector<FontInfo> fontInfos;

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((directory + L"\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
        return fontInfos;

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wstring filename = findFileData.cFileName;
            if (filename.find(L".ttf") != std::string::npos || filename.find(L".otf") != std::string::npos) {
                FontInfo fontInfo;
                fontInfo.name = StringConverter::WstringToUtf8(filename);
                fontInfo.path = directory + L"\\" + filename;
                fontInfos.push_back(fontInfo);
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return fontInfos;
}

void ShowFontSelection(GlobalConfig* globalConfig) {
    static std::vector<FontInfo> fontFiles;

    // Get font files if not loaded yet
    if (fontFiles.empty()) {
        fontFiles = GetFontsFromDirectory(L"C:\\Windows\\Fonts");
        ////获取用户名
        //std::wstring username = L"";
        //DWORD usernameSize = 0;

        // 获取用户名
        wchar_t username[256]; // 确保足够空间来存储用户名
        DWORD usernameSize = sizeof(username) / sizeof(username[0]);
        GetUserNameW(username, &usernameSize);
        std::vector<FontInfo> userFonts = GetFontsFromDirectory(L"C:\\Users\\" + std::wstring(username) + L"\\AppData\\Local\\Microsoft\\Windows\\Fonts");
        fontFiles.insert(fontFiles.end(), userFonts.begin(), userFonts.end());
    }

    if(ImGui::CollapsingHeader(u8"字体选择", ImGuiTreeNodeFlags_FramePadding))
    {
        ImGui::BeginGroup();
        if (ImGui::Selectable(u8"默认字体-Uranus_Pixel_11Px (重启生效)")) {
            // Load the selected font
            globalConfig->fontPath = "default";
        }

        for (size_t i = 0; i < fontFiles.size(); ++i) {
            if (ImGui::Selectable(fontFiles[i].name.c_str())) {
                // Load the selected font
                ImGuiIO& io = ImGui::GetIO();
                io.FontDefault = io.Fonts->AddFontFromFileTTF(StringConverter::WstringToUtf8(fontFiles[i].path).c_str(), 20.0f, nullptr, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
                globalConfig->fontPath = StringConverter::WstringToUtf8(fontFiles[i].path);
            }
        }
        ImGui::EndGroup();
    }

}

MainUI::MainUI(InfoManager* manager)
    : manager(manager)
{
}

void MainUI::Render(GlobalConfig* globalConfig, bool* p_open)
{

    // 首次渲染时，记录圆角状态
    static int firstRender = 5;
    static bool prevRoundCorner = false;
    static bool isStyleEditorShow = false;
    if (isStyleEditorShow)
    {

        ImGui::ShowStyleEditor();
    }

    //使窗口显示在屏幕中间
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - ImGui::GetIO().DisplaySize.x / 2), (ImGui::GetIO().DisplaySize.y - ImGui::GetIO().DisplaySize.y / 2)), ImGuiCond_Once, ImVec2(0.5f, 0.5f));


    ImGui::Begin(u8"主控制面板", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);

    ImGui::Text(u8"-->主控制面板");

    ImGui::SameLine();

    if (ImGui::Button(u8"保存配置"))
        ConfigManager::Save(FileManager::GetConfigPath(), *globalConfig, *manager);


    ImGui::SameLine();

    //将退出按钮显示在右上角
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 45);

    if (ImGui::Button("  X  "))
    {
        //PostQuitMessage(0);
        *p_open = false;
        // 切换 ImGui 鼠标捕获设置
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouse;  // 禁止 ImGui 捕获鼠标
        //io.MouseDrawCursor = false;
    }

    ImGui::SameLine();

    //将退出按钮显示在右上角
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 90);

    if (ImGui::Button("  +  "))
    {
        isStyleEditorShow = !isStyleEditorShow;
    }

    ImGui::Separator();

    //显示公告
    ImGui::BeginChild("Announce", ImVec2(0, 100), true);
    ImGuiStd::TextShadow(App::Instance().announcement.c_str());
    ImGui::EndChild();


    ImGui::Separator();
    if (ImGui::CollapsingHeader(u8"全局设置", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))
    {
        if (ImGui::SliderFloat(u8"圆角半径", &globalConfig->roundCornerRadius, 0.0f, 10.0f, "%.1f"))
        {
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = globalConfig->roundCornerRadius;
            style.FrameRounding = globalConfig->roundCornerRadius;
            style.GrabRounding = globalConfig->roundCornerRadius;
            style.ScrollbarRounding = globalConfig->roundCornerRadius;
            style.TabRounding = globalConfig->roundCornerRadius;
            style.ChildRounding = globalConfig->roundCornerRadius;
            style.PopupRounding = globalConfig->roundCornerRadius;
        }

        //设置主UI快捷键

        ImGuiStd::Keybind(u8"UI快捷键：", globalConfig->menuKey);

        ShowFontSelection(globalConfig);

    }

    ImGui::Separator();

    // 添加各种信息项按钮

    if (ImGui::Button(u8"添加 文本 TextItem")) {
        manager->AddItem(std::make_unique<TextItem>());
    }

    if (ImGui::Button(u8"添加 时间 TimeItem")) {
        manager->AddItem(std::make_unique<TimeItem>());
    }

    if (ImGui::Button(u8"添加 FPS显示 FpsItem")) {
        manager->AddItem(std::make_unique<FpsItem>());
    }

    if (ImGui::Button(u8"添加 文件数量 FileCountItem")) {
        manager->AddItem(std::make_unique<FileCountItem>());
    }

    if (ImGui::Button(u8"添加 B站粉丝数 BilibiliFansItem")) {
        manager->AddItem(std::make_unique<BilibiliFansItem>());
    }

    if (ImGui::Button(u8"添加 计数器 CounterItem"))
    {
        manager->AddItem(std::make_unique<CounterItem>());
    }

    if (ImGui::Button(u8"添加 B站弹幕显示 BilibiliDanmakuItem")) {
        manager->AddItem(std::make_unique<DanmakuItem>());
    }


    ImGui::Separator();
    DrawItemList();

    ImGui::Separator();
    //显示关于
    if (ImGui::CollapsingHeader(u8"关于", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))
    {
        ImGuiStd::TextShadow(u8"无限小窗InfOverlay");
        ImGui::SameLine();
        std::string appVersion = std::to_string(App::Instance().appVersion.major) + "." + std::to_string(App::Instance().appVersion.minor) + "." + std::to_string(App::Instance().appVersion.build);
        ImGuiStd::TextShadow(("v" + appVersion).c_str());
        ImGui::SameLine();
        if (ImGui::Button(u8"检查更新"))
        {
            if (!App::Instance().CheckUpdate())
            {
                ImGui::OpenPopup(u8"发现新版本");
            }
            else
            {
                ImGui::OpenPopup(u8"目前已是最新版本");
            }
        }
        if (ImGui::BeginPopup(u8"发现新版本"))
        {
            ImGuiStd::TextShadow(u8"发现新版本！");
            std::string cloudVersion = std::to_string(App::Instance().cloudVersion.major) + "." + std::to_string(App::Instance().cloudVersion.minor) + "." + std::to_string(App::Instance().cloudVersion.build);
            ImGuiStd::TextShadow((u8"最新版本：" + cloudVersion).c_str());
            if (ImGui::Button(u8"确定"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopup(u8"目前已是最新版本"))
        {
            ImGuiStd::TextShadow(u8"目前已是最新版本");
            if (ImGui::Button(u8"确定"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGuiStd::TextShadow((u8"作者：" + App::Instance().appAuthor).c_str());
        ImGui::SameLine();
        if (ImGui::Button(u8"Bilibili"))
        {
            ShellExecute(NULL, NULL, L"https://space.bilibili.com/399194206", NULL, NULL, SW_SHOWNORMAL);
        }
        ImGuiStd::TextShadow(u8"相关链接：");
        ImGui::SameLine();
        if (ImGui::Button(u8"爱发电"))
        {
            ShellExecute(NULL, NULL, L"https://ifdian.net/a/qc_max", NULL, NULL, SW_SHOWNORMAL);
        }
        ImGui::SameLine();
        ImGuiStd::TextShadow(u8" & ");

        ImGui::SameLine();
        if (ImGui::Button(u8"GitHub"))
        {
            ShellExecute(NULL, NULL, L"https://github.com/QCMaxcer/InfOverlay-DLL", NULL, NULL, SW_SHOWNORMAL);
        }
    }
    ImGui::End();
}

void MainUI::DrawItemList()
{
    auto& items = manager->GetItems();

    for (int i = 0; i < items.size(); i++)
    {
        InfoItem* item = items[i].get();

        ImGui::Checkbox(("##" + std::to_string(i)).c_str(), &item->isEnabled);
        ImGui::SameLine();

        ImGui::Text("[%d] %s", i, item->windowTitle.c_str());
        ImGui::SameLine();
        ImGui::PushID(i);

        if (ImGui::Button(u8"删除"))
        {
            manager->RemoveItem(i);
            ImGui::PopID();
            break;
        }

        ImGui::SameLine();
        if (ImGui::Button(u8"设置"))
        {
            ImGui::OpenPopup("ItemEditor");
        }

        if (ImGui::BeginPopup("ItemEditor"))
        {
            DrawItemEditor(item);
            ImGui::EndPopup();
        }

        ImGui::PopID();
        ImGui::SameLine();
        //避免冲突

        std::string clickThroughStr = u8"固定##" + std::to_string(i);

        ImGui::Checkbox(clickThroughStr.c_str(), &item->clickThrough);
    }
}

void MainUI::DrawItemEditor(InfoItem* item)
{
    ImGui::Text(u8"编辑项：%s", item->windowTitle.c_str());

    // 根据不同类型显示不同配置项

    if (auto t = dynamic_cast<TextItem*>(item)) {
        Draw_TextItemSettings(t);
    }
    else if (auto t = dynamic_cast<TimeItem*>(item)) {
        Draw_TimeItemSettings(t);
    }
    else if (auto f = dynamic_cast<FpsItem*>(item)) {
        Draw_FpsItemSettings(f);
    }
    else if (auto f = dynamic_cast<FileCountItem*>(item)) {
        Draw_FileCountItemSettings(f);
    }
    else if (auto b = dynamic_cast<BilibiliFansItem*>(item)) {
        Draw_BilibiliItemSettings(b);
    }
    else if (auto c = dynamic_cast<CounterItem*>(item)) {
        Draw_CounterItemSettings(c);
    }
    else if (auto d = dynamic_cast<DanmakuItem*>(item)) {
        Draw_DanmakuItemSettings(d);
    }

    // 通用设置
    if (ImGui::CollapsingHeader(u8"通用设置"))
    {
        ImGui::Checkbox(u8"固定", &item->clickThrough);

        ImGui::Checkbox(u8"显示边框", &item->showBorder);


        ImGui::SliderFloat(u8"背景透明度", &item->alpha, 0.0f, 1.0f, "%.1f");

        ImGui::Checkbox(u8"自定义窗口大小", &item->isCustomSize);
        if (item->isCustomSize) {
            ImGui::InputFloat(u8"宽度", &item->width, 1.0f, 1.0f, "%.1f");
            ImGui::InputFloat(u8"高度", &item->height, 1.0f, 1.0f, "%.1f");
        }
        ImGui::InputFloat(u8"窗口 X", &item->x, 1.0f, 1.0f, "%.1f");
        ImGui::InputFloat(u8"窗口 Y", &item->y, 1.0f, 1.0f, "%.1f");

        ImGui::InputFloat(u8"字体大小", &item->fontSize, 1.0f, 1.0f, "%.1f");


        ImGui::InputInt(u8"刷新间隔(毫秒)", &item->refreshIntervalMs);
        ImGuiStd::InputTextStd(u8"前缀", item->prefix);
        ImGuiStd::InputTextStd(u8"后缀", item->suffix);
    }
}

void MainUI::Draw_TextItemSettings(TextItem* item)
{
    ImGuiStd::InputTextStd(u8"文本内容", item->text);
}

void MainUI::Draw_TimeItemSettings(TimeItem* item)
{
    ImGui::Text(u8"TimeItem 无额外设置");
}

void MainUI::Draw_FpsItemSettings(FpsItem* item)
{
    ImGui::Text(u8"FpsItem 无额外设置");
}

void MainUI::Draw_FileCountItemSettings(FileCountItem* item)
{

    ImGuiStd::InputTextStd(u8"文件夹路径", item->folderPath);
    ImGui::Checkbox(u8"递归扫描(包括子文件夹)", &item->recursive);
    ImGuiStd::InputTextStd(u8"扩展名过滤 (.txt)", item->extensionFilter);
    ImGui::Checkbox(u8"提示音", &item->isPlaySound);
    if (item->isPlaySound)
        ImGui::SliderFloat(u8"提示音音量", &item->soundVolume, 0.0f, 1.0f, "%.2f");
}

void MainUI::Draw_BilibiliItemSettings(BilibiliFansItem* item)
{
    static std::string uidStr = std::to_string(item->uid);
    ImGuiStd::InputTextStd(u8"B站 UID", uidStr);
    ImGui::SameLine();
    if (ImGui::Button(u8"确定"))
    {
        if (uidStr.empty())
        {
            uidStr = u8"不能输入空值"; // 默认设置为你的 UID
        }
        else if (uidStr.find_first_not_of("0123456789") != std::string::npos)
        {
            uidStr = u8"只能输入数字"; // 输入非数字
        }
        else
            item->uid = std::stoll(uidStr);
    }
    ImGui::Checkbox(u8"提示音", &item->isPlaySound);
    if (item->isPlaySound)
        ImGui::SliderFloat(u8"提示音音量", &item->soundVolume, 0.0f, 1.0f, "%.2f");
}

void MainUI::Draw_CounterItemSettings(CounterItem* item)
{
    ImGui::InputInt(u8"计数值", &item->count);
    //设置增加快捷键
    ImGuiStd::Keybind(u8"增加快捷键：", item->hotkeyAdd);
//设置减少快捷键
    ImGuiStd::Keybind(u8"减少快捷键：", item->hotkeySub);

    ImGui::Checkbox(u8"提示音", &item->isPlaySound);
    if (item->isPlaySound)
	    ImGui::SliderFloat(u8"提示音音量", &item->soundVolume, 0.0f, 1.0f, "%.2f");
}

void MainUI::Draw_DanmakuItemSettings(DanmakuItem* item)
{
    ImGuiStd::InputTextStd(u8"弹幕日志文件路径", item->logPath);
    ImGui::InputInt(u8"最大弹幕数", &item->maxDanmakuCount);
}