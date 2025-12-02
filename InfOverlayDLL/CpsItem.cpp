#include "CPSItem.h"
#include "App.h"
#include "CPSDetector.h"
void CPSItem::Toggle()
{
}
void CPSItem::DrawContent()
{
    

    bool needMiddleFix = showLeft && showRight;
    std::string middleFix = needMiddleFix ? " | " : "";
    std::string left = showLeft ? std::to_string(CPSDetector::Instance().GetLeftCPS()) : "";
    std::string right = showRight ? std::to_string(CPSDetector::Instance().GetRightCPS()) : "";
    std::string text = left + middleFix + right;
    ImGuiStd::TextShadow((prefix + text + suffix).c_str());
}

void CPSItem::DrawSettings()
{
    DrawItemSettings();
    ImGui::Checkbox(u8"×ó¼ü", &showLeft);
    ImGui::Checkbox(u8"ÓÒ¼ü", &showRight);

    if (ImGui::CollapsingHeader(u8"Í¨ÓÃÉèÖÃ"))
    {
        DrawWindowSettings();
        DrawAffixSettings();
    }
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
