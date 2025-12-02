#include "Menu.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "ImGuiStd.h"
#include "App.h"
#include "Images.h"
#include "pics\MCInjector-small.h"
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

void ShowFontSelection(GlobalConfig* globalConfig) {
    static std::vector<FileUtils::FontInfo> fontFiles;

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

    if(ImGui::TreeNodeEx(u8"字体选择", ImGuiTreeNodeFlags_FramePadding))
    {
        //ImGui::BeginGroup();
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
        //ImGui::EndGroup();
        ImGui::TreePop();
    }


}

Menu::Menu()
{
}


static ImVec4 myWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
static ImVec4 tarWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.3f);
void Menu::Render(bool* done)
{
    if(!open)
        return;
    static std::once_flag flag2;
    std::call_once(flag2, [&]
        {
            static std::thread announcementThread;
            // 启动后台线程
            announcementThread = std::thread([]()
                {
                    App::Instance().GetAnnouncement();
                });
            announcementThread.detach();
        });
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
    ImGui::End();

    ImGui::PopStyleColor();
    switch (state)
    {
    case MENU_STATE_MAIN:
        ShowMain();
        break;
    case MENU_STATE_SETTINGS:
        ShowSettings(done);
        break;
        
    }

    return;
}

void Menu::ShowMain()
{
    bool isBtnHovered = false;

    //使窗口显示在屏幕中间
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - ImGui::GetIO().DisplaySize.x / 2), (ImGui::GetIO().DisplaySize.y - ImGui::GetIO().DisplaySize.y / 2)), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(206.0f, 66.0f), ImGuiCond_Always);
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    //获取io
    ImGuiIO& io = ImGui::GetIO();
    //ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // 边框透明\
            //设置按钮背景半透明

    ImVec4 myColor = colors[ImGuiCol_Button];
    ImVec4 myColorHovered = colors[ImGuiCol_ButtonHovered];
    ImVec4 myColorActive = colors[ImGuiCol_ButtonActive];
    myColor.w = 0.6f;
    myColorHovered.w = 0.6f;
    myColorActive.w = 0.6f;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, myWindowBgColor);
    ImGui::PushStyleColor(ImGuiCol_Button, myColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, myColorHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, myColorActive);
    ImGui::Begin(u8"设置按钮", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav);
    //让设置按钮显示在屏幕中间
    ImGui::PushFont(NULL, style.FontSizeBase * 1.5f);

    ImVec2 myBtnSize(200.0f, 60.0f);     // 可随意修改按钮大小
    if (ImGuiStd::DrawCenteredButton(u8"设置", myBtnSize))
    {
        state = MENU_STATE_SETTINGS;
        tarWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.3f);
    }
    if (ImGui::IsItemHovered())
        isBtnHovered = true;
    else
        isBtnHovered = false;
    ImGui::PopStyleColor(4);
    ImGui::PopFont();
    ImGui::End();
    float panel_speed = 10.0f * io.DeltaTime;
    if (state == MENU_STATE_MAIN)
    {
        if (isBtnHovered)
        {
            tarWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
            panelAnim.state = ImLerp(panelAnim.state, 1.0f, panel_speed);
        }
        else
        {
            tarWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.3f);
            panelAnim.state = ImLerp(panelAnim.state, 0.0f, panel_speed);
        }
    }
    ShowSidePanels();
}

void Menu::ShowSidePanels()
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
        ImGui::PushFont(App::Instance().iconFont, style.FontSizeBase * 1.5f);
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
        ImGui::BeginChild("AnnounceChild", ImVec2(0, 0), true, ImGuiWindowFlags_NoInputs);
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
        ImGui::SameLine();
        ImGui::PopFont();
        ImGui::PushFont(App::Instance().iconFont, style.FontSizeBase * 1.5f);
        float right = ImGui::GetContentRegionAvail().x;
        float textWidth =
            ImGui::GetFontSize() +
            ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + right - textWidth);
        ImGuiStd::TextShadow(u8"\uE034"); //"&#xe034"
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::SetNextWindowBgAlpha(0.3f); // 半透明
        ImGui::BeginChild("UpdateLogChild", ImVec2(0, 0), true, ImGuiWindowFlags_NoInputs);
        //ImGui::PushFont(io.FontDefault);
        ImGuiStd::TextShadow(u8"   -25.11.30-    |    -B0.9.0-");
        ImGui::BulletText(u8"发布第一支测试版本");
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
        static std::once_flag flag;
        std::call_once(flag, [&]
            {
                App::Instance().logoTexture.id = LoadTextureFromMemory(logo, logoSize, &App::Instance().logoTexture.width, &App::Instance().logoTexture.height);
            });
        // 按钮中心
        ImVec2 startPos = Center;

        // 目标位置：向上偏移 100 像素
        ImVec2 targetPos = ImVec2(Center.x, Center.y - 150);

        // 插值当前坐标
        ImVec2 currentPos = targetPos;
            //ImLerp(currentPos, targetPos, t);


        // logo 原始大小
        ImVec2 logoSize = ImVec2(
            (float)App::Instance().logoTexture.width,
            (float)App::Instance().logoTexture.height
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
            ImGui::Image(App::Instance().logoTexture.id,
                finalSize
            );

            ImGui::End();

            ImGui::PopStyleVar(2);
        }
    }
}

void Menu::ShowSettings(bool* done)
{
    static bool isStyleEditorShow = false;
    if (isStyleEditorShow)
    {

        ImGui::ShowStyleEditor();
    }

    //使窗口显示在屏幕中间
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - ImGui::GetIO().DisplaySize.x / 2), (ImGui::GetIO().DisplaySize.y - ImGui::GetIO().DisplaySize.y / 2)), ImGuiCond_Once, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(1000, 618), ImGuiCond_Once);

    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.1f));
    ImGui::Begin(u8"主控制面板", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleColor(2);

    ImGui::SetCursorPos(ImVec2(3, 3));
    //设置控件与左边的间隔
        ImGui::BeginChild("主控制面板Inner", ImVec2(994, 612), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);


        ImGui::Text(u8"-->主控制面板");

        ImGui::SameLine();

        if (ImGui::Button(u8"保存配置"))
            ConfigManager::Instance().Save(FileUtils::configPath);


        ImGui::SameLine();

        //将退出按钮显示在右上角
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 45);

        if (ImGui::Button("  X  "))
        {
            Toggle();
            //*done = true;
        }

        ImGui::SameLine();

        //将退出按钮显示在右上角
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 90);

        if (ImGui::Button("  +  "))
        {
            isStyleEditorShow = !isStyleEditorShow;
        }

        ImGui::Separator();
        ImGui::BeginChild("Content", ImVec2(0, 0), true);
        //显示公告
        ImGui::BeginChild("Announce", ImVec2(0, 100), true);

        ImGuiStd::TextShadow(App::Instance().announcement.c_str());
        ImGui::EndChild();


        ImGui::Separator();
        if (ImGui::CollapsingHeader(u8"全局设置", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))
        {
            if (ImGui::SliderFloat(u8"圆角半径", &GlobalConfig::Instance().roundCornerRadius, 0.0f, 10.0f, "%.1f"))
            {
                ImGuiStyle& style = ImGui::GetStyle();
                float roundCornerRadius = GlobalConfig::Instance().roundCornerRadius;
                style.WindowRounding = roundCornerRadius;
                style.FrameRounding = roundCornerRadius;
                style.GrabRounding = roundCornerRadius;
                style.ScrollbarRounding = roundCornerRadius;
                style.TabRounding = roundCornerRadius;
                style.ChildRounding = roundCornerRadius;
                style.PopupRounding = roundCornerRadius;
            }

            //设置主UI快捷键

            ImGuiStd::Keybind(u8"UI快捷键：", GlobalConfig::Instance().menuKey);

            ShowFontSelection(&GlobalConfig::Instance());

        }

        ImGui::Separator();

        // 添加各种信息项按钮

        if (ImGui::Button(u8"添加 文本")) {
            ItemManager::Instance().AddMulti(std::make_unique<TextItem>());
        }

        if (ImGui::Button(u8"添加 文件数量")) {
            ItemManager::Instance().AddMulti(std::make_unique<FileCountItem>());
        }

        if (ImGui::Button(u8"添加 粉丝数")) {
            ItemManager::Instance().AddMulti(std::make_unique<BilibiliFansItem>());
        }

        if (ImGui::Button(u8"添加 计数器"))
        {
            ItemManager::Instance().AddMulti(std::make_unique<CounterItem>());
        }

        ImGui::Separator();
        DrawItemList();

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

        if (ImGui::Button(u8"自毁"))
        {
            *done = true;
        }
        ImGui::EndChild();
        ImGui::EndChild();

    ImGui::End();
}

void Menu::DrawItemList()
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

        if (item->IsMultiInstance())
        {
            if (ImGui::Button(u8"删除"))
            {
                ItemManager::Instance().RemoveMulti(item);
                ImGui::PopID();
                break;
            }
            ImGui::SameLine();
        }
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
    }
}

void Menu::DrawItemEditor(Item* item)
{
    ImGui::Text(u8"编辑项：%s", item->name.c_str());

    // 根据不同类型显示不同配置项
    item->DrawSettings();
}

void Menu::Toggle(bool open)
{
    this->open = open;
}

void Menu::Toggle()
{
    open = !open;
    if (!open)
    {
        RECT rect;
        // 获取窗口的矩形位置和大小
        if (GetWindowRect(App::Instance().clientHwnd, &rect)) {
            // 计算窗口中心的位置
            int centerX = (rect.left + rect.right) / 2;
            int centerY = (rect.top + rect.bottom) / 2;

            // 设置鼠标位置
            SetCursorPos(centerX, centerY);
        }
        myWindowBgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        panelAnim.state = 0.0f;
    }
    else
    {
        state = MENU_STATE_MAIN;
    }
}