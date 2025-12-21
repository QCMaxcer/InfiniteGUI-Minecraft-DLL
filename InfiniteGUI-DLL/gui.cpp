#include "gui.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"
#include "opengl_hook.h"

#include <GL/glew.h>
#include <GL/GL.h>

#include "fonts.h"
#include "ImGuiSty.h"
#include "ItemManager.h"
#include "GlobalConfig.h"
#include "GuiFrameLimiter.h"

static ImGuiContext* imGuiContext = nullptr;
static CachedDrawData g_Cache;
void Gui::init()
{
	imGuiContext = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 可选
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // 可选

	// 默认关闭 ImGui 捕获鼠标（只有激活 UI 时允许）
	//io.MouseDrawCursor = false;
	//io.ConfigFlags |= ImGuiConfigFlags_NoMouse; // 禁止 ImGui 捕获鼠标输入（我们在切换时会调整）
	io.IniFilename = nullptr; // 禁止生成 imgui.ini
	SetStyleGray();

	ImGui_ImplWin32_Init(opengl_hook::handle_window);
	ImGui_ImplOpenGL3_Init();
	//ImFontConfig config{};
	//config.FontDataOwnedByAtlas = false;
	//font = io.Fonts->AddFontFromMemoryTTF(Fonts::harmony.data, Fonts::harmony.size, 17.f, &config, io.Fonts->GetGlyphRangesChineseFull());

	ImFontConfig font_cfg;
	font_cfg.FontDataOwnedByAtlas = false;
	font_cfg.OversampleH = 1;
	font_cfg.OversampleV = 1;
	font_cfg.PixelSnapH = true;

	if (GlobalConfig::Instance().fontPath == "default")
		font = io.Fonts->AddFontFromMemoryTTF(Fonts::alibaba.data, Fonts::alibaba.size, 20.0f, &font_cfg, io.Fonts->GetGlyphRangesChineseFull());
	else
		font = io.Fonts->AddFontFromFileTTF(GlobalConfig::Instance().fontPath.c_str(), 20.0f, &font_cfg, io.Fonts->GetGlyphRangesChineseFull());
	if (font == nullptr) {
		{
			MessageBox(NULL, L"字体加载失败", L"提示", MB_OK);
			font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 20.0f, &font_cfg, io.Fonts->GetGlyphRangesChineseFull());
		}
	}
	iconFont = io.Fonts->AddFontFromMemoryTTF(Fonts::icons.data, Fonts::icons.size, 20.0f, &font_cfg);


	io.FontDefault = font;

	if (const GLenum err = glewInit(); GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	isInit = true;
}
void Gui::clean()
{
	if(!isInit) return;
	while (opengl_hook::rendering)
	{

	}
	g_Cache.Clear();
	if (imGuiContext)ImGui::GetIO().Fonts->Clear();
	if ((ImGui::GetCurrentContext() ? (void*)ImGui::GetIO().BackendRendererUserData : nullptr))ImGui_ImplOpenGL3_Shutdown();
	if ((ImGui::GetCurrentContext() ? (void*)ImGui::GetIO().BackendPlatformUserData : nullptr))ImGui_ImplWin32_Shutdown();
	if (imGuiContext)ImGui::DestroyContext(imGuiContext);
	isInit = false;
}

static std::atomic_flag clipCursor = ATOMIC_FLAG_INIT;
static RECT originalClip;
void Gui::render()
{
	ItemManager::Instance().RenderAllBeforeGui();
	if (GuiFrameLimiter::Instance().ShouldUpdate() && ItemManager::Instance().IsDirty())
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ItemManager::Instance().RenderAllGui();
		ImGui::Render();
		CacheDrawData(g_Cache, ImGui::GetDrawData());
	}
	// 每一帧都画
	RenderCachedDrawData(g_Cache);
	ItemManager::Instance().RenderAllAfterGui();
}
