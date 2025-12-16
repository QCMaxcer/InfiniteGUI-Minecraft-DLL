#include "TimeItem.h"
#include <iomanip>
#include <sstream>
#include "ImGuiStd.h"

void TimeItem::Toggle()
{
}

void TimeItem::Update()
{
    // 获取系统当前时间
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &t);
#else
    localtime_r(&t, &localTime);
#endif

    // 转换成字符串
    std::stringstream ss;
    ss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");

    currentTimeStr = ss.str();
}

void TimeItem::DrawContent()
{
    ImVec4 targetTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    //获取io
    ImGuiIO& io = ImGui::GetIO();
    //计算速度
    float speed = 3.0f * io.DeltaTime;
    color.color = ImLerp(color.color, targetTextColor, speed);
    ImGuiStd::TextColoredShadow(color.color, (prefix + currentTimeStr + suffix).c_str());
}

void TimeItem::DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
{
    //DrawItemSettings();
    ImGui::Text(u8"时钟 无额外设置");
    DrawAffixSettings(bigPadding, centerX, itemWidth);
    DrawWindowSettings(bigPadding, centerX, itemWidth);
}

void TimeItem::Load(const nlohmann::json& j)
{
    LoadItem(j);
    LoadAffix(j);
    LoadWindow(j);
}

void TimeItem::Save(nlohmann::json& j) const
{
    SaveItem(j);
    SaveAffix(j);
    SaveWindow(j);
}