#include "GameStateDetector.h"
#include "App.h"
#include "KeyState.h"
#include "Menu.h"
void GameStateDetector::Toggle()
{
}
void GameStateDetector::Update()
{
	
	if (Menu::Instance().isEnabled)
		currentState = InMenu;
	else if (IsMouseCursorVisible())
		currentState = InGameMenu;
	else
		currentState = InGame;


}

void GameStateDetector::Load(const nlohmann::json& j)
{
	LoadItem(j);
	if (j.contains("hideItemInGui")) hideItemInGui = j["hideItemInGui"].get<bool>();
}

void GameStateDetector::Save(nlohmann::json& j) const
{
	SaveItem(j);
	j["hideItemInGui"] = hideItemInGui;
}

void GameStateDetector::DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
{
	ImGui::SetCursorPosX(bigPadding);
	ImGui::PushItemWidth(itemWidth);
	ImGui::Checkbox(u8"仅在游戏内显示窗口", &hideItemInGui);
}

bool GameStateDetector::IsNeedHide() const 
{
	return currentState == InGameMenu && hideItemInGui;
}

GameState GameStateDetector::GetCurrentState() const
{
	return currentState;
}

bool GameStateDetector::IsInGame() const
{
	return (currentState == InGame);
}

bool GameStateDetector::IsMouseCursorVisible()
{
	CURSORINFO ci;
	ci.cbSize = sizeof(ci);

	if (GetCursorInfo(&ci))
	{
		return (ci.flags & CURSOR_SHOWING) != 0;
	}
	return false;
}

void GameStateDetector::ProcessMouseMovement(int dx, int dy)
{
	// 鼠标移动速度 = 模长
	cameraSpeed = sqrtf(dx * dx + dy * dy);

	// 视角是否移动（为了给你逻辑判断）
	if (cameraSpeed > movementThreshold)
		cameraMoving = true;
	else
		cameraMoving = false;
	if(Menu::Instance().isEnabled || !IsInGame())
	{
		cameraMoving = false;
		cameraSpeed = 1.0f;
	}
}

bool GameStateDetector::IsCameraMoving() const
{
	return cameraMoving;
}

float GameStateDetector::GetCameraSpeed() const
{
	return cameraSpeed;
}
