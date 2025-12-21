#include "opengl_hook.h"
#include <Windows.h>
#include "detours\titan_hook.h"
#include <iostream>
#include <stdexcept>

#include "menu.h"
#include "ItemManager.h"
#include "GameStateDetector.h"
#include "gui.h"

#include "Images.h"
#include "pics\MCInjector-small.h"
#include <thread>

#include "App.h"
//#include <base/voyage.h>
//#include <mutex>
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static bool detour_wgl_swap_buffers(HDC hdc);
static LRESULT CALLBACK wndproc_hook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void opengl_hook::lwjgl2FullscreenHandler() {
	remove_hook();
	clean();
	init();
	handle_window = WindowFromDC(handle_device_ctx);
	o_wndproc = (WNDPROC)SetWindowLongPtrW(handle_window, GWLP_WNDPROC, (LONG_PTR)wndproc_hook);
	custom_gl_ctx = wglCreateContext(handle_device_ctx);
	gui.logoTexture.id = NULL;
	RECT area;
	GetClientRect(handle_window, &area);
	screen_size.x = area.right - area.left;
	screen_size.y = area.bottom - area.top;
}

static LRESULT CALLBACK wndproc_hook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool isRepeat = (lParam & (1 << 30)) != 0;
	switch (message)
	{
	case WM_KEYDOWN:
	{
		if (wParam == Menu::Instance().GetKeyBind() && !isRepeat)
		{
			Menu::Instance().isEnabled =!Menu::Instance().isEnabled;
			Menu::Instance().Toggle();
		}
		break;
	}
	case WM_INPUT:
	{
		UINT size;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
		BYTE* buffer = new BYTE[size];
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER));

		RAWINPUT* raw = (RAWINPUT*)buffer;
		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			int dx = raw->data.mouse.lLastX;
			int dy = raw->data.mouse.lLastY;

			// dx / dy 就是你的视角移动量
			GameStateDetector::Instance().ProcessMouseMovement(dx, dy);
		}
		delete[] buffer;
	}

	break;
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
	if (Menu::Instance().isEnabled)
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

void opengl_hook::remove_hook()
{
	SetWindowLongPtrW(handle_window, GWLP_WNDPROC, (LONG_PTR)o_wndproc);
	wgl_swap_buffers_hook.RemoveHook();
}

bool opengl_hook::clean()
{

	wglMakeCurrent(handle_device_ctx, opengl_hook::o_gl_ctx);
	wglDeleteContext(custom_gl_ctx);
	gui.clean();
	//ImGui::DestroyContext();
	return false;
}

bool detour_wgl_swap_buffers(HDC hdc)
{
	if (opengl_hook::gui.done)
	{
		return wgl_swap_buffers_hook.GetOrignalFunc()(hdc);
	} //detach
	//glPushMatrix();
	opengl_hook::rendering = true;
	opengl_hook::o_gl_ctx = wglGetCurrentContext();
	opengl_hook::handle_device_ctx = hdc;
	static std::once_flag flag;
	std::call_once(flag, [&]
		{
			// 只执行一次
			opengl_hook::handle_window = WindowFromDC(opengl_hook::handle_device_ctx);
			opengl_hook::custom_gl_ctx = wglCreateContext(opengl_hook::handle_device_ctx);

			RECT area;
			GetClientRect(opengl_hook::handle_window, &area);

			opengl_hook::screen_size.x = area.right - area.left;
			opengl_hook::screen_size.y = area.bottom - area.top;

			opengl_hook::o_wndproc = (WNDPROC)SetWindowLongPtrW(opengl_hook::handle_window, GWLP_WNDPROC, (LONG_PTR)wndproc_hook);
			RAWINPUTDEVICE rid;
			rid.usUsagePage = 0x01;
			rid.usUsage = 0x02; // Mouse
			rid.dwFlags = RIDEV_INPUTSINK;
			rid.hwndTarget = opengl_hook::handle_window;
			RegisterRawInputDevices(&rid, 1, sizeof(rid));
			Fonts::init();

		});
	if (WindowFromDC(hdc) != opengl_hook::handle_window) return wgl_swap_buffers_hook.GetOrignalFunc()(hdc);
	//渲染代码
	if (!opengl_hook::gui.isInit)
	{
		opengl_hook::gui.init();
	}

	wglMakeCurrent(hdc, opengl_hook::custom_gl_ctx);
	if (!opengl_hook::gui.logoTexture.id) opengl_hook::gui.logoTexture.id = LoadTextureFromMemory(logo, logoSize, &opengl_hook::gui.logoTexture.width, &opengl_hook::gui.logoTexture.height);
	opengl_hook::gui.render();
	wglMakeCurrent(hdc, opengl_hook::o_gl_ctx);
	//glPopMatrix();
	opengl_hook::rendering = false;
	return wgl_swap_buffers_hook.GetOrignalFunc()(hdc);
}
