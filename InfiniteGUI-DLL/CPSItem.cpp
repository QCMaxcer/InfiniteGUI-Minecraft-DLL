#include "CPSItem.h"
#include "App.h"
#include "CPSDetector.h"
void CPSItem::Toggle()
{
}
void CPSItem::DrawContent()
{
    ImVec4 targetTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    //获取io
    ImGuiIO& io = ImGui::GetIO();
    //计算速度
    float speed = 3.0f * io.DeltaTime;
    color.color = ImLerp(color.color, targetTextColor, speed);
    bool needMiddleFix = showLeft && showRight;
    //std::string addon = std::to_string(Motionblur::Instance().cur_blurriness_value);
    std::string middleFix = needMiddleFix ? " | " : "";
    std::string left = showLeft ? std::to_string(CPSDetector::Instance().GetLeftCPS()) : "";
    std::string right = showRight ? std::to_string(CPSDetector::Instance().GetRightCPS()) : "";
    std::string text = left + middleFix + right;
    ImGuiStd::TextColoredShadow(color.color, (prefix + text + suffix).c_str());
}

void CPSItem::DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
{
    //DrawItemSettings();
    ImGui::Checkbox(u8"左键", &showLeft);
    ImGui::Checkbox(u8"右键", &showRight);
    DrawAffixSettings(bigPadding, centerX, itemWidth);
    DrawWindowSettings(bigPadding, centerX, itemWidth);
}

void CPSItem::Load(const nlohmann::json& j)
{
    LoadItem(j);
    LoadAffix(j);
    LoadWindow(j);
    if (j.contains("showLeft")) showLeft = j["showLeft"];
    if (j.contains("showRight")) showRight = j["showRight"];
}

void CPSItem::Save(nlohmann::json& j) const
{
    SaveItem(j);
    SaveAffix(j);
    SaveWindow(j);
    j["showLeft"] = showLeft;
    j["showRight"] = showRight;
}
