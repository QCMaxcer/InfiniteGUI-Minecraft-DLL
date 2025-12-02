#include "BilibiliFansItem.h"
#include "HttpClient.h"
#include "ImGuiStd.h"
#include "ImGui\imgui_internal.h"
#include <nlohmann/json.hpp>
#include "AudioManager.h"
#include "HttpUpdateWorker.h"


void BilibiliFansItem::Toggle()
{
    if (isEnabled)
        HttpAddTask();
    else
        HttpRemoveTask();
}

void BilibiliFansItem::HttpAddTask()
{
    httpTaskId = HttpUpdateWorker::Instance().AddTask(
        L"https://api.bilibili.com/x/relation/stat?vmid=" + std::to_wstring(uid),
        httpUpdateIntervalMs,
        [this](const std::string& response) {
            try {
                auto j = nlohmann::json::parse(response);
                pendingFans = j["data"]["follower"].get<int>();
            }
            catch (...) {
                pendingFans = -1;
            }
        }
    );
}

void BilibiliFansItem::HttpRemoveTask()
{
    HttpUpdateWorker::Instance().RemoveTask(httpTaskId);
}

void BilibiliFansItem::Update()
{
    int newFans = pendingFans.load(); 

    if (newFans < 0)
        return;

    fansCount = newFans;

    if (fansCount > lastFansCount)
    {
        color.color = ImVec4(0.1f, 1.0f, 0.1f, 1.0f); //绿色
        if (isPlaySound) AudioManager::Instance().playSound("bilibilifans\\bilibilifans_up.wav", soundVolume);
    }
    else if (fansCount < lastFansCount)
    {
        color.color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f); //红色
        if (isPlaySound) AudioManager::Instance().playSound("bilibilifans\\bilibilifans_down.wav", soundVolume);
    }

    lastFansCount = fansCount;

}

void BilibiliFansItem::DrawContent()
{
    ImVec4 targetTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

    //获取io
    ImGuiIO& io = ImGui::GetIO();

    //计算速度
    float speed = 3.0f * io.DeltaTime;
    color.color = ImLerp(color.color, targetTextColor, speed);

    ImGuiStd::TextColoredShadow(color.color, (prefix + std::to_string(fansCount) + suffix).c_str());
}

void BilibiliFansItem::DrawSettings()
{
    DrawItemSettings();
    static std::string uidStr = std::to_string(uid);
    ImGuiStd::InputTextStd(u8"B站 UID", uidStr);
    ImGui::SameLine();
    if (ImGui::Button(u8"确定"))
    {
        if (uidStr.empty())
        {
            uidStr = u8"不能输入空值"; // 默认设置为你的 UID
        }
        else if (uidStr.find_first_not_of("0123456789") != std::string::npos)
        {
            uidStr = u8"只能输入数字"; // 输入非数字
        }
        else if (std::stoll(uidStr) <= 0)  //数字小于等于0
        {
            uidStr = u8"只能输入正整数"; // 输入负数
        }
        else
        {
            uid = std::stoll(uidStr);
            HttpResetTask();
        }
    }
    if (ImGui::CollapsingHeader(u8"通用设置"))
    {
        DrawWindowSettings();
        DrawAffixSettings();
        DrawSoundSettings();
    }
}

void BilibiliFansItem::Load(const nlohmann::json& j)
{
    LoadItem(j);
    LoadAffix(j);
    LoadWindow(j);
    LoadSound(j);
    if (j.contains("uid")) uid = j["uid"];
    if (j.contains("fansCount")) fansCount = j["fansCount"];
    lastFansCount = fansCount;
    if(isEnabled) HttpAddTask();
}

void BilibiliFansItem::Save(nlohmann::json& j) const
{
    SaveItem(j);
    SaveAffix(j);
    SaveWindow(j);
    SaveSound(j);

    j["uid"] = uid;
    j["fansCount"] = fansCount;
}