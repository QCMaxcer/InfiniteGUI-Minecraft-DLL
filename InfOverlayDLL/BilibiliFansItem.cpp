#include "BilibiliFansItem.h"
#include "HttpClient.h"
#include "ImGuiStd.h"
#include "ImGui\imgui_internal.h"
#include <nlohmann/json.hpp>

void BilibiliFansItem::Update()
{
    if (uid <= 0) {
        fansCount = -1;
        return;
    }

    // 生成 URL（注意使用 wstring 版）
    std::wstring url =
        L"https://api.bilibili.com/x/relation/stat?vmid=" +
        std::to_wstring(uid);

    std::string response;
    bool ok = HttpClient::HttpGet(url, response);

    int resultFans = -1;

    if (ok)
    {
        try {
            auto j = nlohmann::json::parse(response);
            resultFans = j["data"]["follower"].get<int>();
        }
        catch (...) {
            resultFans = -1;
        }
    }

    fansCount = resultFans;
}

void BilibiliFansItem::DrawContent()
{
    if (fansCount < 0) {
        ImGuiStd::TextShadow(u8"粉丝数获取失败");
        return;
    }
    if (fansCount > lastFansCount)
    {
        color.color = ImVec4(0.1f, 1.0f, 0.1f, 1.0f); //绿色
        lastFansCount = fansCount;
    }
    else if (fansCount < lastFansCount)
    {
        color.color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f); //红色
        lastFansCount = fansCount;
    }

    ImVec4 targetTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

    //获取io
    ImGuiIO& io = ImGui::GetIO();

    //计算速度
    float speed = 3.0f * io.DeltaTime;
    color.color = ImLerp(color.color, targetTextColor, speed);

    ImGuiStd::TextColoredShadow(color.color, (prefix + std::to_string(fansCount) + suffix).c_str());
}

void BilibiliFansItem::Load(const nlohmann::json& j)
{
    if (j.contains("uid")) uid = j["uid"];

    LoadInfoItemConfig(j);
}

void BilibiliFansItem::Save(nlohmann::json& j) const
{
    j["type"] = "bilibili_fans";
    j["uid"] = uid;
    SaveInfoItemConfig(j);
}