#include "opengl_hook.h"
#include "FileUtils.h"
#include "ConfigManager.h"
#include "AudioManager.h"
#include <thread>
#include <atomic>
#include "HttpUpdateWorker.h"
HMODULE g_hModule = NULL;
std::thread g_updateThread;
static std::atomic_bool g_running = ATOMIC_VAR_INIT(true);
// 线程函数：更新所有 item 状态
void UpdateThread() {
	while (g_running.load()) {
		ItemManager::Instance().UpdateAll();  // 调用UpdateAll()来更新所有item
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
	g_running = false;
	if (g_updateThread.joinable()) {
		g_updateThread.join();
	}
}

//void Uninit() {
//	g_running = false;
//	AudioManager::Instance().Shutdown();
//	ConfigManager::Save(FileUtils::configPath);
//}


DWORD WINAPI InitApp(LPVOID)
{
    FileUtils::InitPaths(g_hModule);
    opengl_hook::init();
	while (!opengl_hook::gui.isInit)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	//加载配置文件
	ConfigManager::Load(FileUtils::configPath);
	//初始化音频管理器
	AudioManager::Instance().Init();
	HttpUpdateWorker::Instance().Start();
	StartThreads();

	//while (!opengl_hook::gui.done)
	//{
	//	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//}
	

    return 0;

}