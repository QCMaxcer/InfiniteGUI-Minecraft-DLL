#include "CounterItem.h"
#include "AudioManager.h"
#include "imgui\imgui.h"
#include "ImGuiStd.h"
#include "imgui\imgui_internal.h"
#include <string>
#include <windows.h>

void CounterItem::Toggle()
{
}

void CounterItem::OnKeyEvent(bool state, bool isRepeat, WPARAM key)
{
    if(key == NULL) return;
    if(state) //按键按下
    {
        if (key == keybinds.at(u8"增加快捷键："))
        {
            count++;
        }
        else if (key == keybinds.at(u8"减少快捷键："))
        {
            count--;
        }
        else if (key == keybinds.at(u8"清空快捷键："))
        {
            count = 0;
        }
        if (count > lastCount)
        {
            color.color = ImVec4(0.1f, 1.0f, 0.1f, 1.0f); //绿色
            lastCount = count;
            if (isPlaySound) AudioManager::Instance().playSound("counter\\counter_up.wav", soundVolume);
        }
        else if (count < lastCount)
        {
            color.color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f); //红色
            lastCount = count;
            if (isPlaySound) AudioManager::Instance().playSound("counter\\counter_down.wav", soundVolume);
        }
    }
}

void CounterItem::DrawContent()
{

    ImVec4 targetTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

    //获取io
    ImGuiIO& io = ImGui::GetIO();

    //计算速度
    float speed = 3.0f * io.DeltaTime;
    color.color = ImLerp(color.color, targetTextColor, speed);

    ImGuiStd::TextColoredShadow(color.color, (prefix + std::to_string(count) + suffix).c_str());

}

void CounterItem::DrawSettings()
{
    DrawItemSettings();
    ImGui::InputInt(u8"计数值", &count);
    DrawKeybindSettings();
    if (ImGui::CollapsingHeader(u8"通用设置"))
    {
        DrawWindowSettings();
        DrawAffixSettings();
        DrawSoundSettings();
    }
}

void CounterItem::Load(const nlohmann::json& j)
{
    LoadItem(j);
    LoadAffix(j);
    LoadWindow(j);
    LoadSound(j);
    LoadKeybind(j);
    if (j.contains("count")) count = j["count"];
    lastCount = count;

}

void CounterItem::Save(nlohmann::json& j) const
{
    SaveItem(j);
    SaveAffix(j);
    SaveWindow(j);
    SaveSound(j);
    SaveKeybind(j);
    j["count"] = count;
}