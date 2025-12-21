#include "opengl_hook.h"
#include "FileUtils.h"
#include "ConfigManager.h"
#include "AudioManager.h"
#include <thread>
#include <atomic>

#include "App.h"
#include "ClickSound.h"
#include "HttpUpdateWorker.h"
#include "ItemManager.h"
#include "GuiFrameLimiter.h"
HMODULE g_hModule = NULL;
std::thread g_updateThread;
static std::atomic_bool g_running = ATOMIC_VAR_INIT(true);
// 线程函数：更新所有 item 状态
void UpdateThread() {
	while (g_running.load()) {
		if(opengl_hook::gui.isInit) ItemManager::Instance().UpdateAll();  // 调用UpdateAll()来更新所有item
		std::this_thread::sleep_for(std::chrono::milliseconds(1));  // 休眠1ms，可以根据实际需求调整
	}
}

// 启动更新线程
void StartThreads() {
	g_updateThread = std::thread(UpdateThread);
	g_updateThread.detach();  // 将线程设为后台线程
}

// 停止更新线程
void StopThreads() {
	if (g_updateThread.joinable()) {
		g_updateThread.join();
	}
}

void Uninit() {
	opengl_hook::remove_hook();
	opengl_hook::clean();
	while(opengl_hook::gui.isInit) //等hook退出
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	g_running = false;
	StopThreads();
	HttpUpdateWorker::Instance().Stop();
	AudioManager::Instance().Shutdown();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	FreeLibraryAndExitThread(g_hModule, 0);

}


DWORD WINAPI InitApp(LPVOID)
{
    FileUtils::InitPaths(g_hModule);
	//加载配置文件
	ConfigManager::Instance().Init();

	ConfigManager::Instance().LoadGlobal();

	GuiFrameLimiter::Instance().Init();
    opengl_hook::init();
	while (!opengl_hook::gui.isInit)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	ConfigManager::Instance().LoadProfile();
	//初始化音频管理器
	AudioManager::Instance().Init();
	ClickSound::PlayIntroSound();
	HttpUpdateWorker::Instance().Start();
	StartThreads();

	static std::thread announcementThread;
	// 启动后台线程
	announcementThread = std::thread([]()
		{
			App::Instance().GetAnnouncement();
		});
	announcementThread.detach();

	while (!opengl_hook::gui.done)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (opengl_hook::handle_window != WindowFromDC(opengl_hook::handle_device_ctx))
			opengl_hook::lwjgl2FullscreenHandler();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	Uninit();

    return 0;

}