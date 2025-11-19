#include "App.h"
#include "HttpClient.h"
#include <nlohmann/json.hpp>
#include <string>

App& App::Instance()
{
    static App instance;
    return instance;
}

bool App::CheckUpdate()
{
    std::string response;
    bool ok = HttpClient::HttpGet(versionUrl, response);

    if (ok)
    {
        try {
            auto j = nlohmann::json::parse(response);
            cloudVersion.major = j["version"]["major"].get<int>();
            cloudVersion.minor = j["version"]["minor"].get<int>();
            cloudVersion.build = j["version"]["build"].get<int>();
        }
        catch (...) {
            // 解析失败，返回空对象
            MessageBox(NULL, L"版本信息解析失败，请联系作者", L"提示", MB_OK);
        }
    }
    else {
        // 网络请求失败，返回空对象
        MessageBox(NULL, L"网络请求失败，请检查网络连接", L"提示", MB_OK);
    }
    if (cloudVersion.major > appVersion.major || cloudVersion.minor > appVersion.minor || cloudVersion.build > appVersion.build)
    {
        // 有新版本，弹出提示框
        return false;
    }
    return true;
}

void App::GetAnnouncement()
{
    std::string response;
    bool ok = HttpClient::HttpGet(announcementUrl, response);
    if (ok)
        announcement = response;
    else
        MessageBox(NULL, L"获取公告失败，请检查网络连接", L"提示", MB_OK);
}