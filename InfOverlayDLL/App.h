#pragma once

#include <Windows.h>
#include <string>

struct version
{
	int major;
	int minor;
	int build;
};

class App
{
public:
	std::string appName = "InfOverlay";
	version appVersion = { 1, 0, 0 };
	std::string appAuthor = "QC_Max";
	std::string appDescription = "InfOverlay is a library that allows you to create overlays for in-game information.";
	std::string announcement = "这里没有公告捏...";
	version cloudVersion;
	std::wstring versionUrl = L"https://gitee.com/qc_max/inf-overlay/raw/master/version.json";
	std::wstring announcementUrl = L"https://gitee.com/qc_max/inf-overlay/raw/master/announcement.txt";
	static App& Instance();

	void GetAnnouncement();

	bool CheckUpdate();
};