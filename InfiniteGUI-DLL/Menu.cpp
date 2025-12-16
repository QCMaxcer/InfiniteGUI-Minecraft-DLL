#include "Menu.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "ImGuiStd.h"
#include "App.h"
#include "ConfigManager.h"
#include "GlobalConfig.h"
#include "FileUtils.h"
#include "StringConverter.h"
#include "TextItem.h"
#include "FileCountItem.h"
#include "BilibiliFansItem.h"
#include "CounterItem.h"
#include "opengl_hook.h"
#include <thread>
#include "ChangeLog.h"
#include "fonts.h"
#include "ItemManager.h"
#include "MainMenuButton.h"
void ShowFontSelection(GlobalConfig* globalConfig) {
    static std::vector<FileUtils::FontInfo> fontFiles;
    ImGuiIO& io = ImGui::GetIO();
    // Get font files if not loaded yet
    if (fontFiles.empty()) {
        fontFiles = FileUtils::GetFontsFromDirectory(L"C:\\Windows\\Fonts");
        ////获取用户名
        //std::wstring username = L"";
        //DWORD usernameSize = 0;

        // 获取用户名
        wchar_t username[256]; // 确保足够空间来存储用户名
        DWORD usernameSize = sizeof(username) / sizeof(username[0]);
        GetUserNameW(username, &usernameSize);
        std::vector<FileUtils::FontInfo> userFonts = FileUtils::GetFontsFromDirectory(L"C:\\Users\\" + std::wstring(username) + L"\\AppData\\Local\\Microsoft\\Windows\\Fonts");
        fontFiles.insert(fontFiles.end(), userFonts.begin(), userFonts.end());
    }

    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    font_cfg.OversampleH = 1;
    font_cfg.OversampleV = 1;
    font_cfg.PixelSnapH = true;

    if(ImGui::TreeNodeEx(u8"字体选择", ImGuiTreeNodeFlags_FramePadding))
    {
        //ImGui::BeginGroup();
        if (ImGui::Selectable(u8"默认字体-阿里巴巴普惠体(重启生效)")) {
            // Load the selected font
            opengl_hook::gui.font = io.Fonts->AddFontFromMemoryTTF(Fonts::alibaba.data, Fonts::alibaba.size, 20.0f, &font_cfg, io.Fonts->GetGlyphRangesChineseFull());
            io.FontDefault = opengl_hook::gui.font;
            globalConfig->fontPath = "default";
        }

        for (size_t i = 0; i < fontFiles.size(); ++i) {
            if (ImGui::Selectable(fontFiles[i].name.c_str())) {
                // Load the selected font
                opengl_hook::gui.font = io.Fonts->AddFontFromFileTTF(StringConverter::WstringToUtf8(fontFiles[i].path).c_str(), 20.0f, &font_cfg, ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
                io.FontDefault = opengl_hook::gui.font;
                globalConfig->fontPath = StringConverter::WstringToUtf8(fontFiles[i].path);
            }
        }
        //ImGui::EndGroup();
        ImGui::TreePop();
    }
}

bool CheckNameValid(const std::string& inputName)
{
    bool valid = true;
    for (auto c : inputName) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
            valid = false;
            break;
        }
    }
    return valid;
}

void ShowConfigSelection(GlobalConfig* globalConfig) {
    // 当前 profile 名称（utf8）
    std::string curProfile = ConfigManager::Instance().GetCurrentProfile();
    ImVec2 btnSize = ImVec2(150.0f, 0.0f);

    // 按钮索引用来生成唯一 id
    static int uniqueId = 0;

    // 临时 UI 状态（静态以便跨帧保存输入）
    static std::string inputName;           // 用于新增/重命名的输入
    static std::string renameOldName;       // 重命名时要替换的旧名字
    static std::string deleteTargetName;    // 待删除的名字（在确认弹窗中使用）
    static std::string lastErrorMsg;        // 校验/操作失败提示

    // 弹窗标识
    const char* addPopupName = u8"创建";
    const char* renamePopupName = u8"重命名";
    const char* deletePopupName = u8"删除";

    if (ImGui::TreeNodeEx((u8"配置选择： " + StringConverter::AcpToUtf8(curProfile)).c_str(),
        ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_AllowOverlap))
    {
        std::vector<std::string> profiles = ConfigManager::Instance().GetProfiles();

        for (size_t i = 0; i < profiles.size(); ++i) {
            std::string profileName = profiles[i];

            // 主按钮：切换配置
            if (ImGui::Button(profileName.c_str(), btnSize)) {
                // Load the selected config (保存当前 -> 切换)
                ConfigManager::Instance().SwitchProfile(profileName, true);
            }

            ImGui::SameLine();

            // 图标按钮（占位/未来用）
            ImGui::PushFont(opengl_hook::gui.iconFont);
            // 重命名按钮（紧跟在 icon 后面；用小字体或图标也行）
            if (ImGui::Button((u8"\uE02F##rename" + std::to_string(i)).c_str())) {
                renameOldName = profileName;
                inputName = profileName; // 预填旧名字
                lastErrorMsg.clear();
                ImGui::OpenPopup(renamePopupName);
            }

            // 删除按钮（只有 profiles > 1 且 不是当前配置时显示）
            if (profiles.size() > 1 && profileName != curProfile)
            {
                ImGui::SameLine();
                if (ImGui::Button((u8"9##del" + std::to_string(i)).c_str())) {
                    // 打开删除确认弹窗
                    deleteTargetName = profileName;
                    lastErrorMsg.clear();
                    ImGui::OpenPopup(deletePopupName);
                }
            }
            ImGui::PopFont();
        }

        // 新建配置按钮
        if (ImGui::Button("+", btnSize))
        {
            // 生成一个默认唯一名（profile_1, profile_2...）
            std::vector<std::string> profilesNow = ConfigManager::Instance().GetProfiles();
            std::string base = "profile";
            int idx = 1;
            std::string candidate;
            do {
                candidate = base + "_" + std::to_string(idx++);
            } while (std::find(profilesNow.begin(), profilesNow.end(), candidate) != profilesNow.end());
            inputName = candidate;
            lastErrorMsg.clear();
            ImGui::OpenPopup(addPopupName);
        }

        // ----------------------
    // Create Profile Popup
    // ----------------------
        if (ImGui::BeginPopupModal(addPopupName, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {

            ImGuiStd::TextShadow(u8"输入新配置名称：");
            ImGui::SetNextItemWidth(300.0f);
            // 使用 InputText 的 std::string 版本
            char bufCreate[256];
            std::strncpy(bufCreate, inputName.c_str(), sizeof(bufCreate));
            if (ImGui::InputText("##newProfile", bufCreate, sizeof(bufCreate))) {
                inputName = std::string(bufCreate);
            }

            if (!lastErrorMsg.empty()) {
                ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1.0f), "%s", lastErrorMsg.c_str());
            }

            if (ImGui::Button(u8"创建")) {
                // 校验
                if (inputName.empty()) {
                    lastErrorMsg = u8"名称不能为空。";
                }
                else if (!CheckNameValid(inputName)) {
                    lastErrorMsg = u8"名称只能包含英文数字下划线。";
                }
                else {
                    auto profilesNow = ConfigManager::Instance().GetProfiles();
                    if (std::find(profilesNow.begin(), profilesNow.end(), inputName) != profilesNow.end()) {
                        lastErrorMsg = u8"该名称已存在。";
                    }
                    else {
                        bool ok = ConfigManager::Instance().CreateProfile(inputName);
                        if (!ok) {
                            lastErrorMsg = u8"创建配置文件失败(可能是 IO 错误？)。";
                        }
                        else {
                            ImGui::CloseCurrentPopup();
                            lastErrorMsg.clear();
                            // 可选：自动切换到新配置
                            // ConfigManager::Instance().SwitchProfile(inputName, true);
                        }
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"取消")) {
                ImGui::CloseCurrentPopup();
                lastErrorMsg.clear();
            }

            ImGui::EndPopup();
        }

        // ----------------------
        // Rename Profile Popup
        // ----------------------
        if (ImGui::BeginPopupModal(renamePopupName, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGuiStd::TextShadow(u8"重命名：");
            ImGui::SetNextItemWidth(300.0f);
            char bufRename[256];
            std::strncpy(bufRename, inputName.c_str(), sizeof(bufRename));
            if (ImGui::InputText("##renameProfile", bufRename, sizeof(bufRename))) {
                inputName = std::string(bufRename);
            }

            if (!lastErrorMsg.empty()) {
                ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1.0f), "%s", lastErrorMsg.c_str());
            }

            if (ImGui::Button(u8"确认")) {
                if (inputName.empty()) {
                    lastErrorMsg = u8"名称不能为空。";
                }
                else if (!CheckNameValid(inputName)) {
                    lastErrorMsg = u8"名称只能包含英文数字下划线。";
                }
                else if (inputName == renameOldName) {
                    // 没改名，直接关闭
                    ImGui::CloseCurrentPopup();
                    lastErrorMsg.clear();
                }
                else {
                    auto profilesNow = ConfigManager::Instance().GetProfiles();
                    if (std::find(profilesNow.begin(), profilesNow.end(), inputName) != profilesNow.end()) {
                        lastErrorMsg = u8"该名称已存在。";
                    }
                    else {
                        bool ok = ConfigManager::Instance().RenameProfile(renameOldName, inputName);
                        if (!ok) {
                            lastErrorMsg = u8"创建配置文件失败(可能是 IO 错误？)。";
                        }
                        else {
                            // 若重命名的是当前 profile，需要更新 currentProfile UI 文本（ConfigManager 实现一般会处理）
                            ImGui::CloseCurrentPopup();
                            lastErrorMsg.clear();
                        }
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"取消")) {
                ImGui::CloseCurrentPopup();
                lastErrorMsg.clear();
            }
            ImGui::EndPopup();
        }

        // ----------------------
        // Delete Confirmation Popup
        // ----------------------
        if (ImGui::BeginPopupModal(deletePopupName, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            std::string deleteTargetNameUtf8 = StringConverter::AcpToUtf8(deleteTargetName);
            std::string deleteTargetNameUtf8Confirm = u8"确认删除\"" + deleteTargetNameUtf8 + u8"\"?(该操作不可恢复)";
            ImGuiStd::TextShadow(deleteTargetNameUtf8Confirm.c_str());
            if (!lastErrorMsg.empty()) {
                ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1.0f), "%s", lastErrorMsg.c_str());
            }

            if (ImGui::Button(u8"删除")) {
                // 调用删除（ConfigManager 会阻止删除当前 profile 或保留至少一个）
                bool ok = ConfigManager::Instance().DeleteProfile(deleteTargetName);
                if (!ok) {
                    lastErrorMsg = u8"删除失败，可能是因为它是当前配置或只有一个配置。";
                }
                else {
                    ImGui::CloseCurrentPopup();
                    lastErrorMsg.clear();
                    // 如果用户刚删了当前之外的 profile，通常不需额外操作
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"取消")) {
                ImGui::CloseCurrentPopup();
                lastErrorMsg.clear();
            }
            ImGui::EndPopup();
        }



        ImGui::TreePop();
    }
}


static ImVec4 myWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
static ImVec4 tarWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.3f);
void Menu::Render()
{
    if(!isEnabled)
        return;
    if (blur->menu_blur) blur->RenderBlur(panelAnim.blurriness);
    //使窗口显示在屏幕中间
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - ImGui::GetIO().DisplaySize.x / 2), (ImGui::GetIO().DisplaySize.y - ImGui::GetIO().DisplaySize.y / 2)), ImGuiCond_Once, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2((float)opengl_hook::screen_size.x + 10, (float)opengl_hook::screen_size.y + 10), ImGuiCond_Always);


    //获取io
    ImGuiIO& io = ImGui::GetIO();
    //计算速度
    float speed = 5.0f * io.DeltaTime;
    myWindowBgColor = ImLerp(myWindowBgColor, tarWindowBgColor, speed);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, myWindowBgColor);
    ImGui::Begin(u8"菜单背景", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav);
    ImGui::PopStyleColor();
    ImGui::PushFont(NULL, itemStyle.fontSize);
    if (itemStyle.rainbowFont)
        processRainbowFont();
    else
        ImGui::PushStyleColor(ImGuiCol_Text, itemStyle.fontColor); // 字体颜色
    switch (state)
    {
    case MENU_STATE_MAIN:
        ShowMain();
        break;
    case MENU_STATE_SETTINGS:
        ImGui::PushStyleColor(ImGuiCol_WindowBg, itemStyle.bgColor);
        ImGui::PushStyleColor(ImGuiCol_Border, itemStyle.borderColor);
        PushRounding(itemStyle.windowRounding);
        panelAnim.blurriness = (float)blur->blurriness_value;
        ShowSettings(&opengl_hook::gui.done);
        OldSettings();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(7);
        break;
        
    }
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::End();

    return;
}

void Menu::OnKeyEvent(bool state, bool isRepeat, WPARAM key)
{
    if (key == NULL || isRepeat) return;
    if (state) //按键按下
    {
        if (key == VK_ESCAPE && isEnabled)
        {
            isEnabled = false;
            Toggle();
        }
    }
}

int Menu::GetKeyBind()
{
    return keybinds.at(u8"菜单快捷键：");
}
MainMenuButton* myButton;
void Menu::ShowMain()
{
    if (myButton == nullptr)
    {
        myButton = new MainMenuButton(u8"设置", ImVec2(200.0f, 60.0f));
    }
    bool isBtnHovered = false;
    ////使窗口显示在屏幕中间
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    ImVec4 myColor = colors[ImGuiCol_Button];
    ImVec4 myColorHovered = colors[ImGuiCol_ButtonHovered];
    ImVec4 myColorActive = colors[ImGuiCol_ButtonActive];
    myColor.w = 0.6f;
    myColorHovered.w = 0.6f;
    myColorActive.w = 0.6f;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_Button, myColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, myColorHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, myColorActive);
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetCursorScreenPos(ImVec2((opengl_hook::screen_size.x - 200.0f) / 2, (opengl_hook::screen_size.y - 60.0f) / 2));
    bool isWindowDraging = ImGui::IsMouseDragging(0, 1.0f);
    ImGui::PushFont(NULL, style.FontSizeBase * 1.5f);
    if (myButton->Draw(isWindowDraging))
    {
        state = MENU_STATE_SETTINGS;
        tarWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.3f);
    }
    if (ImGui::IsItemHovered())
        isBtnHovered = true;
    else
        isBtnHovered = false;
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::PopFont();
    ImGui::PopStyleColor(4);
    float panel_speed = 10.0f * io.DeltaTime;
    if (state == MENU_STATE_MAIN)
    {
        if (isBtnHovered)
        {
            tarWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
            panelAnim.state = ImLerp(panelAnim.state, 1.0f, panel_speed);
            panelAnim.blurriness = ImLerp(panelAnim.blurriness, (float)blur->blurriness_value, panel_speed);
        }
        else
        {
            tarWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.3f);
            panelAnim.state = ImLerp(panelAnim.state, 0.0f, panel_speed);
            panelAnim.blurriness = ImLerp(panelAnim.blurriness, 0.0f, panel_speed);
        }
    }
    ShowSidePanels();
}

void Menu::ShowSidePanels() const
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    // 动画值（0~1）
    float t = panelAnim.state;

    if (t <= 0.001f)
        return;   // 完全缩回的时候不画，提高性能

    // 面板大小
    ImVec2 panelSize = ImVec2(350, 500);
    // 按钮屏幕坐标（世界坐标）
    ImVec2 Center; 
    Center.x = io.DisplaySize.x / 2;
    Center.y = io.DisplaySize.y / 2;


    // 插值缩放
    float scale = ImLerp(0.0f, 1.0f, t);

    // 最终面板缩放尺寸
    ImVec2 finalSize = ImVec2(panelSize.x * scale, panelSize.y * scale);

    // 左右滑动距离
    float slideOffset = 400.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, scale - 0.2f * (1.0f - scale));
    ImGui::PushTextWrapPos(0.0f); // 设置自动换行
    //======================
    // 左侧 ABOUT 面板动画
    //======================
    {
        ImVec2 targetPos = ImVec2(Center.x  -slideOffset, Center.y);
        ImVec2 currentPos = ImLerp(Center, targetPos, t);

        ImGui::SetNextWindowPos(ImVec2(currentPos.x - finalSize.x * 0.5f, currentPos.y - finalSize.y * 0.5f));
        ImGui::SetNextWindowSize(finalSize);

        //ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::SetNextWindowBgAlpha(0.7f); // 半透明
        ImGui::Begin("##AboutPanel", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs);
        ImGui::PushFont(opengl_hook::gui.iconFont, 28.0f);
        ImGuiStd::TextShadow("I");
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::PushFont(io.FontDefault, style.FontSizeBase * 1.5f);
        float right = ImGui::GetContentRegionAvail().x;
        float textWidth =
            ImGui::CalcTextSize(u8"公告").x +
            ImGui::GetStyle().ItemSpacing.x;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + right - textWidth);
        ImGuiStd::TextShadow(u8"公告");
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::SetNextWindowBgAlpha(0.3f); // 半透明
        ImGui::BeginChild("AnnounceChild", ImVec2(0, 0), true, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar);
        ImGuiStd::TextShadow(App::Instance().announcement.c_str());
        ImGui::EndChild();
        ImGui::End();

        ImGui::PopStyleVar();
    }

    //======================
    // 右侧 更新日志 面板动画
    //======================
    {
        ImVec2 targetPos = ImVec2(Center.x + slideOffset, Center.y);
        ImVec2 currentPos = ImLerp(Center, targetPos, t);

        ImGui::SetNextWindowPos(ImVec2(currentPos.x - finalSize.x * 0.5f, currentPos.y - finalSize.y * 0.5f));
        ImGui::SetNextWindowSize(finalSize);

        //ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::SetNextWindowBgAlpha(0.7f); // 半透明
        ImGui::Begin("##UpdateLogPanel", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs);
        ImGui::PushFont(io.FontDefault, style.FontSizeBase * 1.5f);
        ImGuiStd::TextShadow(u8"日志");
        ImVec2 separatorPos = ImGui::GetCursorPos();
        ImGui::SameLine();
        ImGui::PopFont();
        ImGui::PushFont(opengl_hook::gui.iconFont, 28.0f);
        float right = ImGui::GetContentRegionAvail().x;
        float textWidth =
            ImGui::GetFontSize() +
            ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + right - textWidth);
        ImGuiStd::TextShadow(u8"\uE034"); //"&#xe034"
        ImGui::PopFont();
        ImGui::SetCursorPos(separatorPos); //separator的位置由字体大小决定，需要重新设置
        ImGui::Separator();
        ImGui::SetNextWindowBgAlpha(0.3f); // 半透明
        ImGui::BeginChild("UpdateLogChild", ImVec2(0, 0), true, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar);
        ChangeLog::Instance().Draw();
        ImGui::EndChild();
        ImGui::End();

        ImGui::PopStyleVar();
    }
    ImGui::PopStyleVar();
    ImGui::PopTextWrapPos();
    //======================
// LOGO 顶部动画面板
//======================
    {
        // 按钮中心
        ImVec2 startPos = Center;

        // 目标位置：向上偏移 100 像素
        ImVec2 targetPos = ImVec2(Center.x, Center.y - 150);

        // 插值当前坐标
        ImVec2 currentPos = targetPos;
            //ImLerp(currentPos, targetPos, t);


        // logo 原始大小
        ImVec2 logoSize = ImVec2(
            (float)opengl_hook::gui.logoTexture.width,
            (float)opengl_hook::gui.logoTexture.height
        );

        // 动画缩放，0~1
        float scale = t;

        // 最终缩放大小
        ImVec2 finalSize = ImVec2(
            logoSize.x * scale,
            logoSize.y * scale
        );

        // logo 窗口透明度（跟着scale淡入）
        float logoAlpha = scale;

        if (finalSize.x > 1 && finalSize.y > 1)
        {
            ImGui::SetNextWindowPos(
                ImVec2(
                    currentPos.x - finalSize.x * 0.5f,
                    currentPos.y - finalSize.y * 0.5f
                )
            );

            ImGui::SetNextWindowSize(finalSize);
            ImGui::SetNextWindowBgAlpha(0.0f); // 完全无背景（相当于纯 Image）

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, logoAlpha);

            ImGui::Begin("##LogoPanel", nullptr,
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoInputs |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoBackground
            );

            // 居中绘制图片
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::Image(opengl_hook::gui.logoTexture.id,
                finalSize
            );

            ImGui::End();

            ImGui::PopStyleVar(2);
        }
    }
}

ImVec2 menuInnerSize = ImVec2(900, 556);
ImVec2 menuSize = ImVec2(menuInnerSize.x + 6.0f, menuInnerSize.y + 6.0f);
void Menu::ShowSettings(bool* done)
{
    //使窗口显示在屏幕中间
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - ImGui::GetIO().DisplaySize.x / 2), (ImGui::GetIO().DisplaySize.y - ImGui::GetIO().DisplaySize.y / 2)), ImGuiCond_Once, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(menuSize, ImGuiCond_Once);

    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    //ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::Begin(u8"主控制面板", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleColor();

    ImGui::SetCursorPos(ImVec2(3, 3));
    //设置控件与左边的间隔
    if (!initialized)
    {
        settingMenu->Init();
        initialized = true;
    }
    settingMenu->Draw();
    ImGui::End();
}

void Menu::OldSettings()
{
    //自动大小
    ImGui::SetNextWindowSize(ImVec2(500, 1280), ImGuiCond_Once);
    ImGui::Begin(u8"设置", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
    ImGui::Text(u8"-->主控制面板");

    ImGui::SameLine();

    if (ImGui::Button(u8"保存配置"))
        ConfigManager::Instance().Save();


    ImGui::SameLine();

    //将退出按钮显示在右上角
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 45);

    if (ImGui::Button("  X  "))
    {
        isEnabled = false;
        Toggle();
        //*done = true;
    }

    //ImGui::SameLine();

    ////将退出按钮显示在右上角
    //ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 90);

    //if (ImGui::Button("  +  "))
    //{
    //    isStyleEditorShow = !isStyleEditorShow;
    //}

    ImGui::Separator();
    ImGui::BeginChild("Content", ImVec2(0, 0), true);
    //显示公告
    ImGui::BeginChild("Announce", ImVec2(0, 100), true);

    ImGuiStd::TextShadow(App::Instance().announcement.c_str());
    ImGui::EndChild();


    ImGui::Separator();
    if (ImGui::CollapsingHeader(u8"全局设置", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))
    {
        ImGuiStyle& style = ImGui::GetStyle();
        for (Item* item : ItemManager::Instance().GetAllItems())
        {
            if (item->type == Hidden)
            {
                ImGui::PushFont(NULL, style.FontSizeBase * 0.8f);
                ImGui::BeginDisabled();
                ImGuiStd::TextShadow(item->name.c_str());
                ImGui::EndDisabled();
                ImGui::PopFont();
                //item->DrawSettings();
            }
        }

        ShowFontSelection(&GlobalConfig::Instance());
        ShowConfigSelection(&GlobalConfig::Instance());
    }

    ImGui::Separator();

    // 添加各种信息项按钮

    //if (ImGui::Button(u8"添加 文本")) {
    //    ItemManager::Instance().AddMulti(std::make_unique<TextItem>());
    //}

    //if (ImGui::Button(u8"添加 文件数量")) {
    //    ItemManager::Instance().AddMulti(std::make_unique<FileCountItem>());
    //}

    //if (ImGui::Button(u8"添加 粉丝数")) {
    //    ItemManager::Instance().AddMulti(std::make_unique<BilibiliFansItem>());
    //}

    //if (ImGui::Button(u8"添加 计数器"))
    //{
    //    ItemManager::Instance().AddMulti(std::make_unique<CounterItem>());
    //}

    ImGui::Separator();
    DrawItemList();
    ImGui::PushFont(NULL, ImGui::GetFontSize() * 1.25f);
    ImGui::PopFont();
    ImGui::Separator();

    //显示关于
    if (ImGui::CollapsingHeader(u8"关于", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))
    {
        ImGuiStd::TextShadow(App::Instance().appName.c_str());
        ImGui::SameLine();
        std::string appVersion = std::to_string(App::Instance().appVersion.major) + "." + std::to_string(App::Instance().appVersion.minor) + "." + std::to_string(App::Instance().appVersion.build);
        ImGuiStd::TextShadow(("v" + appVersion).c_str());
        ImGui::SameLine();
        static std::atomic<bool> checkingUpdate = false;   // 是否在检查
        static std::atomic<bool> updateFinished = false;   // 检查是否完成
        static bool updateHasNew = false;                  // 是否发现新版本
        static std::thread updateThread;                   // 工作线程
        // 点击按钮：开始异步检查
        if (ImGui::Button(u8"检查更新"))
        {
            // 打开“正在检查”窗口
            ImGui::OpenPopup(u8"-->检查更新...");

            checkingUpdate = true;
            updateFinished = false;

            // 启动后台线程
            updateThread = std::thread([]()
                {
                    bool result = App::Instance().CheckUpdate();
                    updateHasNew = !result;   // result=false => 有新版本
                    updateFinished = true;
                    checkingUpdate = false;
                });

            updateThread.detach();
        }
        if (ImGui::BeginPopupModal(u8"-->检查更新...", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (checkingUpdate)
            {
                ImGuiStd::TextShadow(u8"正在检查更新，请稍候...");
            }
            else if (updateFinished)
            {
                if (updateHasNew)
                {
                    ImGuiStd::TextShadow(u8"发现新版本！");
                    std::string cloudVersion =
                        std::to_string(App::Instance().cloudVersion.major) + "." +
                        std::to_string(App::Instance().cloudVersion.minor) + "." +
                        std::to_string(App::Instance().cloudVersion.build);

                    ImGuiStd::TextShadow((u8"最新版本：" + cloudVersion).c_str());
                }
                else
                    ImGuiStd::TextShadow(u8"目前已是最新版本");
                if (ImGui::Button(u8"确定"))
                {
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
        ImGuiStd::TextShadow(u8"作者：");
        ImGui::SameLine();
        if (ImGui::Button(App::Instance().appAuthor.c_str()))
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
            ShellExecute(NULL, NULL, L"https://github.com/QCMaxcer/InfiniteGUI-Minecraft-DLL", NULL, NULL, SW_SHOWNORMAL);
        }
    }

    //if (ImGui::Button(u8"自毁"))
    //{
    //    *done = true;
    //}
    ImGui::EndChild();
    ImGui::End();
}

void Menu::DrawItemList() const
{
    auto& items = ItemManager::Instance().GetAllItems();

    for (int i = 0; i < items.size(); i++)
    {
        Item* item = items[i];
        if (item->type == Hidden)
            continue;

        if (ImGui::Checkbox(("##" + std::to_string(i)).c_str(), &item->isEnabled))
        {
            item->Toggle();
        }
        ImGui::SameLine();

        ImGui::Text("[%d] %s", i, item->name.c_str());
        ImGui::SameLine();
        ImGui::PushID(i);

        if (ImGui::Button(u8"删除"))
        {
            //ItemManager::Instance().RemoveMulti(item);
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
            //DrawItemEditor(item);
            ImGui::EndPopup();
        }

        ImGui::PopID();
    }
}

void Menu::DrawItemEditor(Item* item, const float& bigPadding, const float& centerX, const float& itemWidth) const
{
    ImGui::Text(u8"编辑项：%s", item->name.c_str());

    // 根据不同类型显示不同配置项
    item->DrawSettings(bigPadding, centerX, itemWidth);
}

void Menu::Toggle()
{
    if (!isEnabled)
    {
        RECT rect;
        // 获取窗口的矩形位置和大小
        if (GetWindowRect(opengl_hook::handle_window, &rect)) {
            // 计算窗口中心的位置
            int centerX = (rect.left + rect.right) / 2;
            int centerY = (rect.top + rect.bottom) / 2;

            // 设置鼠标位置
            SetCursorPos(centerX, centerY);
        }
        myWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        panelAnim.state = 0.0f;
        panelAnim.blurriness = 0.0f;
    }
    else
    {
        state = MENU_STATE_MAIN;
    }
}

void Menu::DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
{
    float bigItemWidth = centerX * 2.0f - bigPadding * 4.0f;

    ImGui::SetCursorPosX(bigPadding);
    ImGui::Checkbox(u8"背景模糊", &blur->menu_blur);
    ImGui::SetCursorPosX(bigPadding);
    ImGui::PushItemWidth(bigItemWidth);
    ImGui::SliderInt(u8"模糊强度", &blur->blurriness_value, 0, 10);
    DrawKeybindSettings(bigPadding, centerX, itemWidth);
    DrawStyleSettings(bigPadding, centerX, itemWidth);
}

void Menu::Load(const nlohmann::json& j)
{
    LoadKeybind(j);
    if(j.contains("menu_blur")) blur->menu_blur = j["menu_blur"].get<bool>();
    if(j.contains("blurriness_value")) blur->blurriness_value = j["blurriness_value"].get<int>();
    LoadStyle(j);
    //LoadItem(j);
}

void Menu::Save(nlohmann::json& j) const
{
    SaveKeybind(j);
    j["menu_blur"] = blur->menu_blur;
    j["blurriness_value"] = blur->blurriness_value;
    SaveStyle(j);
    j["type"] = name;
}
