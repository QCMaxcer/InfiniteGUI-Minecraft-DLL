// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <windows.h>

#include "infoManager.h"
#include "ConfigManager.h"
#include "MainUI.h"
#include "GlobalConfig.h"
#include "StringConverter.h"

#include "fonts\Uranus_Pixel_11Px.h"


    // 全局在 dllmain.cpp 顶部声明（extern 或 static）
InfoManager infoManager;
MainUI mainUI(&infoManager);
ConfigManager configManager;
GlobalConfig globalConfig;


typedef BOOL(WINAPI* OldSwapBuffers)(HDC);
OldSwapBuffers fpSwapBuffers = NULL;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

// 全局状态
static HWND g_hwnd = NULL;
static bool g_isInit = false;
static WNDPROC g_oldWndProc = NULL;
static bool g_uiActive = false;       // F1 切换 UI 激活
static bool g_cursorClipped = false;  // 是否已限制鼠标
static RECT g_clipRectBackup;         // 备份 cliprect（可选）

// 为安全：避免重复初始化/卸载
static bool g_hookInitialized = false;
static bool g_mhInitialized = false;

// 仅当 uiActive=true 时，让 ImGui 处理 Win32 消息
LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (g_uiActive)
    {
        // 只有 UI 激活时才把消息交给 ImGui 处理
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
            return TRUE;
    }

    // 始终把消息交回原来的窗口过程（不管 UI 是否激活）
    return CallWindowProc(g_oldWndProc, hWnd, message, wParam, lParam);
}

void setStyle()
{
    //https://github.com/ocornut/imgui/issues/707#issuecomment-2393152123
    // Setup style
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    //colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    //colors[ImGuiCol_SliderGrabActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.24f, 0.24f, 0.24f, 0.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.3f);

    style.WindowRounding = globalConfig.roundCornerRadius;
    style.FrameRounding = globalConfig.roundCornerRadius;
    style.GrabRounding = globalConfig.roundCornerRadius;
    style.ScrollbarRounding = globalConfig.roundCornerRadius;
    style.TabRounding = globalConfig.roundCornerRadius;
    style.ChildRounding = globalConfig.roundCornerRadius;
    style.PopupRounding = globalConfig.roundCornerRadius;

}

// 线程函数：更新所有 item 状态
void UpdateThread() {
    while (true) {
        infoManager.UpdateAll();  // 调用UpdateAll()来更新所有item
        std::this_thread::sleep_for(std::chrono::milliseconds(1));  // 休眠100ms，可以根据实际需求调整
    }
}

// 启动更新线程
void StartUpdateThread() {
    std::thread updateThread(UpdateThread);
    updateThread.detach();  // 将线程设为后台线程
}



// 在找到有效 HDC/窗口后初始化 ImGui（只初始化一次）
void InitImGuiForContext(HWND hwnd)
{
    if (g_isInit) return;
    if (!hwnd) return;

    g_hwnd = hwnd;

    // 替换窗口过程以便我们可以在 UI 激活时处理消息
    g_oldWndProc = (WNDPROC)SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)HookedWndProc);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 可选
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // 可选

    // 默认关闭 ImGui 捕获鼠标（只有激活 UI 时允许）
    io.MouseDrawCursor = false;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouse; // 禁止 ImGui 捕获鼠标输入（我们在切换时会调整）
    io.IniFilename = nullptr; // 禁止生成 imgui.ini
    ImGui::StyleColorsDark();
    setStyle();

    // 初始化平台/渲染绑定。GLSL 版本根据 MC 的 GL context 版本可调整
    ImGui_ImplWin32_Init(g_hwnd);

    //ImGui_ImplOpenGL3_Init("#version 130"); // 如果你的 MC 是较新 GL，可改为 "#version 330 core"
     // 尝试获取当前 OpenGL 上下文信息
    const GLubyte* glVersionStr = glGetString(GL_VERSION);
    const GLubyte* glRendererStr = glGetString(GL_RENDERER);

    if (!glVersionStr)
    {
        MessageBoxA(hwnd, "OpenGL context not ready! Please call after SwapBuffers hook is active.", "Error", MB_ICONERROR);
        return;
    }

    std::string glVersion(reinterpret_cast<const char*>(glVersionStr));
    std::string glRenderer(reinterpret_cast<const char*>(glRendererStr ? glRendererStr : (const GLubyte*)"Unknown"));

    int major = 0, minor = 0;
    sscanf(glVersion.c_str(), "%d.%d", &major, &minor);

    const char* glsl_version = "#version 130"; // fallback default

    if (major >= 4) glsl_version = "#version 410";
    else if (major == 3 && minor >= 3) glsl_version = "#version 330";
    else if (major == 3 && minor >= 2) glsl_version = "#version 150";
    else glsl_version = "#version 130";

    // 输出检测结果到控制台（或日志）
    std::string msg = "Detected OpenGL version: " + glVersion +
        "\nRenderer: " + glRenderer +
        "\nUsing GLSL: " + glsl_version + "\n";
    OutputDebugStringA(msg.c_str());
    //MessageBoxA(hwnd, msg.c_str(), "Info", MB_ICONINFORMATION);
    ImGui_ImplOpenGL3_Init(glsl_version);
    g_isInit = true;

    ConfigManager::Load(ConfigManager::GetConfigPath(), globalConfig, infoManager);

    StartUpdateThread();  // 启动更新线程
    ImFont* font;
    if (globalConfig.fontPath == "default")
        font = io.Fonts->AddFontFromMemoryCompressedTTF(Ur_data, Ur_size, 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    else
        font = io.Fonts->AddFontFromFileTTF(globalConfig.fontPath.c_str(), 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    if (font == nullptr) {
        font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    }
    io.FontDefault = font;
}

// 恢复鼠标限制（释放）
void ReleaseCursorClip()
{
    if (g_cursorClipped)
    {
        ClipCursor(NULL);
        g_cursorClipped = false;
    }
}

// 应用或释放鼠标限制（当 UI 激活时可选）
void ApplyCursorClip(bool enable)
{
    if (!g_hwnd) return;
    if (enable && !g_cursorClipped)
    {
        RECT rc;
        GetClientRect(g_hwnd, &rc);
        // 将客户端坐标转换为屏幕坐标
        MapWindowPoints(g_hwnd, NULL, (POINT*)&rc, 2);
        ClipCursor(&rc);
        g_cursorClipped = true;
    }
    else if (!enable && g_cursorClipped)
    {
        ClipCursor(NULL);
        g_cursorClipped = false;
    }
}

// Hooked SwapBuffers - 每次换帧都会被调用
BOOL WINAPI MySwapBuffers(HDC hdc)
{
    // 获取与当前 HDC 关联的顶层窗口（只在首次时用于初始化）
    if (!g_hwnd)
    {
        HWND possible = WindowFromDC(hdc);
        if (possible) g_hwnd = possible;
    }

    // 在首次有有效窗口时初始化 ImGui（安全）
    if (!g_isInit && g_hwnd)
        InitImGuiForContext(g_hwnd);

    // 切换 UI 激活开关（按下一次切换）
    if (GetAsyncKeyState(globalConfig.menuKey) & 1)
    {
        g_uiActive = !g_uiActive;

        // 切换 ImGui 鼠标捕获设置
        ImGuiIO& io = ImGui::GetIO();
        if (g_uiActive)
        {
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse; // 允许 ImGui 处理鼠标
            io.MouseDrawCursor = true;
            // 限制鼠标到游戏窗口（可选，方便 UI 拖动）
            ApplyCursorClip(true);
        }
        else
        {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;  // 禁止 ImGui 捕获鼠标
            io.MouseDrawCursor = false;
            // 释放鼠标
            ApplyCursorClip(false);
        }
    }

    // 如果 ImGui 未初始化，直接调用原 SwapBuffers
    if (!g_isInit)
        return fpSwapBuffers(hdc);

    // --- 新帧 ---
    ImGui_ImplOpenGL3_NewFrame(); // 如果不可用，可省略（一些版本没有此函数，但通常有）
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    {

        infoManager.RenderAll(&globalConfig, g_hwnd); 

        // 如果 UI 激活，弹出交互主界面
        if (g_uiActive)
        {
        mainUI.Render(&globalConfig); 

        }
    }

    // 渲染 ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 如果多视口 (通常注入到 MC 时不使用多视口) -- 跳过

    // 调用原始 SwapBuffers
    return fpSwapBuffers(hdc);
}

// 初始化 Hook（在单独线程里执行）
DWORD WINAPI InitThread(LPVOID)
{

    if (g_mhInitialized) return 0;

    if (MH_Initialize() != MH_OK)
    {
        return 0;
    }
    g_mhInitialized = true;

    // 创建 Hook（对 wglSwapBuffers）
    // 注意：直接使用 &SwapBuffers 也可（GDI SwapBuffers），这里 hook opengl 的 SwapBuffers（用户提供的代码用 SwapBuffers）
    if (MH_CreateHookApi(L"opengl32", "wglSwapBuffers", &MySwapBuffers, reinterpret_cast<void**>(&fpSwapBuffers)) != MH_OK)
    {
        // fallback: try hooking SwapBuffers from GDI32 (less likely)
        if (MH_CreateHook(&::SwapBuffers, &MySwapBuffers, reinterpret_cast<void**>(&fpSwapBuffers)) != MH_OK)
        {
            // failed to create hook
            MH_Uninitialize();
            g_mhInitialized = false;
            return 0;
        }
    }

    // 启用所有 hook
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        g_mhInitialized = false;
        return 0;
    }

    g_hookInitialized = true;
    return 0;
}

void Uninit()
{
    // 卸载 ImGui
    if (g_isInit)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_isInit = false;
    }

    // 恢复窗口过程
    if (g_hwnd && g_oldWndProc)
    {
        SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)g_oldWndProc);
        g_oldWndProc = NULL;
    }

    // 释放鼠标限制
    ReleaseCursorClip();

    // 卸载 MinHook
    if (g_mhInitialized)
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_RemoveHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        g_mhInitialized = false;
        g_hookInitialized = false;
    }
}

// DllMain：在 DLL 注入后创建线程初始化 Hook，卸载时清理
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // 禁用线程库通知，优化
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, InitThread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        Uninit();
        break;
    }
    return TRUE;
}