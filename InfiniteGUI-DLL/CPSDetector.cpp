#include "CPSDetector.h"
#include "App.h"
#include "opengl_hook.h"

//// 检查是否到了更新的时间
//bool CPSDetector::ShouldCpsUpdate() {
//    auto now = std::chrono::steady_clock::now();
//    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCpsTime).count();
//    return elapsedTime >= cpsIntervalMs;
//}
//
//// 更新操作
//void CPSDetector::MarkUpCPSdated() {
//    lastCpsTime = std::chrono::steady_clock::now();
//}

void CPSDetector::Toggle()
{
}

void CPSDetector::Update()
{
    cps.processClick();
    if (opengl_hook::handle_window != GetForegroundWindow())
        return;
    if (keyStateHelper.GetKeyClick(VK_LBUTTON))
    {
        cps.AddLeftClick();
    }
    if (keyStateHelper.GetKeyClick(VK_RBUTTON))
    {
        cps.AddRightClick();
    }
}

int CPSDetector::GetLeftCPS()
{
    return cps.GetLeftCPS();
}
int CPSDetector::GetRightCPS()
{
    return cps.GetRightCPS();
}


void CPSDetector::DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
{
}

void CPSDetector::Load(const nlohmann::json& j)
{
    LoadItem(j);
}

void CPSDetector::Save(nlohmann::json& j) const
{
    SaveItem(j);
}
