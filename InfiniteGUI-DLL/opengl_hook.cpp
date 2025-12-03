#include "opengl_hook.h"
#include <Windows.h>
#include "detours\titan_hook.h"
#include <iostream>
#include <stdexcept>

#include "menu.h"
#include "ItemManager.h"
#include "gui.h"

#include "App.h"
#include "Images.h"
#include "pics\MCInjector-small.h"
#include <thread>
//#include <base/voyage.h>
//#include <mutex>
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static bool detour_wgl_swap_buffers(HDC hdc);
static LRESULT CALLBACK wndproc_hook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool isRepeat = (lParam & (1 << 30)) != 0;
	switch (message)
	{
	case WM_KEYDOWN:
	{
		if (wParam == GlobalConfig::Instance().menuKey && !isRepeat)
		{
			Menu::Instance().Toggle();
		}
		if (wParam == VK_ESCAPE && Menu::Instance().open)
			Menu::Instance().Toggle();
		break;
	}

	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	{
		break;
	}
	case WM_XBUTTONDOWN:
	{
	}
	break;
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		opengl_hook::screen_size = { width, height };
		break;
	}
	default:
	{
		break;
	}
	}
	bool state;
	if (message == WM_KEYDOWN || message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
		message == WM_MBUTTONDOWN || message == WM_XBUTTONDOWN)
		state = true;
	else if (message == WM_KEYUP || message == WM_LBUTTONUP || message == WM_RBUTTONUP ||
		message == WM_MBUTTONUP || message == WM_XBUTTONUP)
		state = false;
	else
		state = true;

	ItemManager::Instance().ProcessKeyEvents(state, isRepeat, wParam);
	if (Menu::Instance().open)
	{
		// 只有 UI 激活时才把消息交给 ImGui 处理
		ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
		switch (message)
		{
		case WM_IME_STARTCOMPOSITION:
		case WM_IME_ENDCOMPOSITION:
		case WM_IME_COMPOSITION:

		case WM_IME_SETCONTEXT:
		case WM_IME_NOTIFY:
		case WM_IME_CONTROL:
		case WM_IME_COMPOSITIONFULL:
		case WM_IME_SELECT:
		case WM_IME_CHAR:
		case WM_IME_REQUEST:
		case WM_IME_KEYDOWN:
		case WM_IME_KEYUP:

		case WM_INPUTLANGCHANGEREQUEST:
		case WM_INPUTLANGCHANGE:

		case WM_DESTROY:
		case WM_QUIT:
		case WM_CLOSE:
			goto end;
		default:
			return TRUE;
		}

	}
end:
	// 始终把消息交回原来的窗口过程（不管 UI 是否激活）
	return CallWindowProcW(opengl_hook::o_wndproc, hWnd, message, wParam, lParam);
}



TitanHook<decltype(&detour_wgl_swap_buffers)>wgl_swap_buffers_hook;
void opengl_hook::init()
{
	HMODULE h_ogl_32 = GetModuleHandleW(L"opengl32.dll");
	if (!h_ogl_32)
	{
		throw std::runtime_error("unable to get ogl32 handle!");
	}
	LPVOID pfunc_wgl_swap_buffers = (LPVOID)GetProcAddress(h_ogl_32, "wglSwapBuffers");
	if (!pfunc_wgl_swap_buffers)
	{
		throw std::runtime_error("unable to get wglswapbuffer's function address!");
	}
	wgl_swap_buffers_hook.InitHook(pfunc_wgl_swap_buffers, detour_wgl_swap_buffers);
	wgl_swap_buffers_hook.SetHook();

	return;
}
bool opengl_hook::clean()
{

	wglMakeCurrent(handle_device_ctx, opengl_hook::o_gl_ctx);
	wglDeleteContext(custom_gl_ctx);
	SetWindowLongPtrW(handle_window, GWLP_WNDPROC, (LONG_PTR)o_wndproc);
	wgl_swap_buffers_hook.RemoveHook();
	gui.clean();
	//ImGui::DestroyContext();
	return false;
}
//#include <base/features/events/events.h>
bool detour_wgl_swap_buffers(HDC hdc)
{
	opengl_hook::o_gl_ctx = wglGetCurrentContext();
	opengl_hook::handle_device_ctx = hdc;
	static std::once_flag flag;
	std::call_once(flag, [&]
		{
			// 只执行一次
			opengl_hook::handle_window = WindowFromDC(opengl_hook::handle_device_ctx);
			opengl_hook::custom_gl_ctx = wglCreateContext(opengl_hook::handle_device_ctx);
			//imgui的初始化
			opengl_hook::o_wndproc = (WNDPROC)SetWindowLongPtrW(opengl_hook::handle_window, GWLP_WNDPROC, (LONG_PTR)wndproc_hook);
			RECT area;
			GetClientRect(opengl_hook::handle_window, &area);

			opengl_hook::screen_size.x = area.right - area.left;
			opengl_hook::screen_size.y = area.bottom - area.top;

			opengl_hook::gui.init();
		});
	if (WindowFromDC(hdc) != opengl_hook::handle_window) return wgl_swap_buffers_hook.GetOrignalFunc()(hdc);

	wglMakeCurrent(hdc, opengl_hook::custom_gl_ctx);

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
			App::Instance().logoTexture.id = LoadTextureFromMemory(logo, logoSize, &App::Instance().logoTexture.width, &App::Instance().logoTexture.height);
		});

	// 切换 ImGui 鼠标捕获设置
	ImGuiIO& io = ImGui::GetIO();
	if (Menu::Instance().open)
	{
		io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse; // 允许 ImGui 处理鼠标
	}
	else
	{
		io.ConfigFlags |= ImGuiConfigFlags_NoMouse;  // 禁止 ImGui 捕获鼠标
	}
	//渲染代码
	opengl_hook::gui.render();

	//渲染结束
	wglMakeCurrent(hdc, opengl_hook::o_gl_ctx);

	return wgl_swap_buffers_hook.GetOrignalFunc()(hdc);
}
