#pragma once
#include "Sprint.h"
#include "KeyState.h"
#include "AudioManager.h"
#include "GameStateDetector.h"
void Sprint::Toggle()
{
}
void Sprint::OnKeyEvent(bool state, bool isRepeat, WPARAM key)
{
    if (key == NULL || !GameStateDetector::Instance().IsInGame()) return;
    if (state && !isRepeat) //按键按下
    {
        if (key == keybinds.at(u8"激活键："))
        {
            isActivated = !isActivated;
            if (isActivated)
            {
                color.color = ImVec4(0.1f, 1.0f, 0.1f, 1.0f); //绿色
                if (isPlaySound) AudioManager::Instance().playSound("counter\\counter_up.wav", soundVolume);
            }
            else
            {
                color.color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f); //红色
                if (isPlaySound) AudioManager::Instance().playSound("counter\\counter_down.wav", soundVolume);
            }
        }

    }

}

void Sprint::GetSneaking()
{
    if (GetKeyDown(keybinds.at(u8"潜行键：")) && GameStateDetector::Instance().IsInGame())
    {
        color.color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); //灰色
        state = Sneaking;
        lastState = state;
    }
}

void Sprint::GetWalking()
{
    if (GetKeyDown(keybinds.at(u8"前进键：")) && GameStateDetector::Instance().IsInGame())
    {
        state = isActivated ? Sprinting : Walking;
        lastState = state;
    }
}

void Sprint::SetSprinting()
{
    if (state == Sprinting)
    {
        color.color = ImVec4(0.1f, 1.0f, 0.1f, 1.0f); //绿色
        SetKeyDown(keybinds.at(u8"疾跑键："), 1);
    }
    if (state != Sprinting && lastState == Sprinting)
    {
        SetKeyUp(keybinds.at(u8"疾跑键："), 1);
        lastState = state;
    }
}

void Sprint::Update()
{
    state = Idle;
    GetWalking();
    GetSneaking();
    if (!isActivated) return;
    SetSprinting();

}

void Sprint::DrawContent()
{
    ImVec4 targetTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

    //获取io
    ImGuiIO& io = ImGui::GetIO();

    //计算速度
    float speed = 3.0f * io.DeltaTime;
    color.color = ImLerp(color.color, targetTextColor, speed);

    std::string text;
    switch (state)
    {
    case Idle:
        text = u8"空闲中";
        break;
    case Sprinting:
        text = u8"疾跑中";
        break;
    case Sneaking:
        text = u8"潜行中";
        break;
    case Walking:
        text = u8"行走中";
        break;
    default:
        break;
    }

    ImGuiStd::TextColoredShadow(color.color, (prefix + text + suffix).c_str());
}

void Sprint::DrawSettings()
{
    DrawItemSettings();
    ImGui::Checkbox(u8"激活", &isActivated);
    ImGui::Checkbox(u8"显示窗口", &isWindowShow);

    DrawKeybindSettings();
    if (ImGui::CollapsingHeader(u8"通用设置"))
    {
        DrawWindowSettings();
        DrawAffixSettings();
        DrawSoundSettings();
    }
}

void Sprint::Load(const nlohmann::json& j)
{
    LoadItem(j);
    LoadAffix(j);
    LoadWindow(j);
    LoadKeybind(j);
    LoadSound(j);
    if (j.contains("isWindowShow")) isWindowShow = j["isWindowShow"];
    if (j.contains("isActivated")) isActivated = j["isActivated"];
}

void Sprint::Save(nlohmann::json& j) const
{
    SaveItem(j);
    SaveAffix(j);
    SaveWindow(j);
    SaveKeybind(j);
    SaveSound(j);
    j["isWindowShow"] = isWindowShow;
    j["isActivated"] = isActivated;
}
