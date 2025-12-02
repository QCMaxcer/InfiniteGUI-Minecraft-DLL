#include "GameStateDetector.h"
#include "App.h"
#include "KeyState.h"
void GameStateDetector::Toggle()
{
}
void GameStateDetector::Update()
{
	isInGame = !IsMouseCursorVisible();

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

void GameStateDetector::DrawSettings()
{
	ImGui::Checkbox(u8"仅在游戏内显示窗口", &hideItemInGui);
}

//bool GameStateDetector::IsPaused() const
//{
//	return false;
//}
//
//bool GameStateDetector::IsInventoryOpen() const
//{
//	return false;
//}
//
//bool GameStateDetector::IsChatTyping() const
//{
//	return false;
//}
//
//bool GameStateDetector::IsInGUI() const
//{
//	return false;
//}

bool GameStateDetector::IsNeedHide() const 
{
	return !isInGame && hideItemInGui;
}

bool GameStateDetector::IsInGame() const
{
	return isInGame;
}

bool GameStateDetector::IsMouseCursorVisible() const
{
	CURSORINFO ci;
	ci.cbSize = sizeof(ci);

	if (GetCursorInfo(&ci))
	{
		return (ci.flags & CURSOR_SHOWING) != 0;
	}
	return false;
}