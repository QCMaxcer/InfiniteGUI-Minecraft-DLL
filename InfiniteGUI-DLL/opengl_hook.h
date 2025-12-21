#pragma once
#include <Windows.h>
//#include "math.hpp"
#include "gui.h"

struct screen
{
	int x;
	int y;
};

namespace opengl_hook
{
	void lwjgl2FullscreenHandler();
	void init();
	void remove_hook();
	bool clean();
	inline bool rendering = false;
	inline HDC handle_device_ctx;
	inline HGLRC o_gl_ctx;//mc原来的gl上下文
	inline HGLRC custom_gl_ctx;//我们自己创建的gl上下文
	inline HWND handle_window;
	inline WNDPROC o_wndproc;
	inline screen screen_size;
	inline Gui gui;
}

