#include "ClickEffect.h"
#include "ImGuiStd.h"
#include "GameStateDetector.h"
void ClickEffect::Toggle()
{
	if (!isEnabled)
	{
		ClearClickEffects();
	}
}

void ClickEffect::Update()
{
	if(opengl_hook::handle_window != GetForegroundWindow()) return;
	GameState state = GameStateDetector::Instance().GetCurrentState();
	bool yes = false;
	if (enabledInGame && state == InGame)
		yes = true;
	else if (enabledInGameMenu && state == InGameMenu)
		yes = true;
	else if (enabledInMenu && state == InMenu)
		yes = true;
	if(!yes) return;
	//用windows.h函数来获取客户端中鼠标位置
	POINT point;
	if (state == InGame)
	{
		//获取窗口中心位置
		RECT rect;
		GetClientRect(GetForegroundWindow(), &rect);
		point.x = rect.left + rect.right / 2 - 1.0f;
		point.y = rect.top + rect.bottom / 2 + 1.0f;
	}
	else
	{
		GetCursorPos(&point);
		ScreenToClient(GetForegroundWindow(), &point);
	}

	ImVec2 mousePos = ImVec2(point.x, point.y);

	if (keyStateHelper.GetKeyClick(VK_LBUTTON) && enableLeftClick || keyStateHelper.GetKeyClick(VK_RBUTTON) && enableRightClick)
	{
		ClickCircle* clickCircle = new ClickCircle(mousePos, clickCircleSettings);
		AddClickEffect(clickCircle);
	}

	if (clickEffects.empty()) return;
	if(clickEffects.front()->IsFinished()) 
	{
		delete clickEffects.front();
		clickEffects.pop_front();
	}
}

void ClickEffect::Render()
{
	Draw();
}

void ClickEffect::Load(const nlohmann::json& j)
{
	LoadItem( j );
	if(j.contains("enabledInGame")) enabledInGame = j["enabledInGame"];
	if(j.contains("enabledInGameMenu")) enabledInGameMenu = j["enabledInGameMenu"];
	if(j.contains("enabledInMenu")) enabledInMenu = j["enabledInMenu"];
	if(j.contains("enableLeftClick")) enableLeftClick = j["enableLeftClick"];
	if(j.contains("enableRightClick")) enableRightClick = j["enableRightClick"];
	if(j.contains("clickCircleSettings.animSpeed")) clickCircleSettings.animSpeed = j["clickCircleSettings.animSpeed"];
    ImGuiStd::LoadImVec4(j, "clickCircleSettings.color", clickCircleSettings.color);
	if(j.contains("clickCircleSettings.radius")) clickCircleSettings.radius = j["clickCircleSettings.radius"];
}

void ClickEffect::Save(nlohmann::json& j) const
{
	SaveItem( j );
	j["enabledInGame"] = enabledInGame;
	j["enabledInGameMenu"] = enabledInGameMenu;
	j["enabledInMenu"] = enabledInMenu;
	j["enableLeftClick"] = enableLeftClick;
	j["enableRightClick"] = enableRightClick;
	j["clickCircleSettings.animSpeed"] = clickCircleSettings.animSpeed;
	ImGuiStd::SaveImVec4(j, "clickCircleSettings.color", clickCircleSettings.color);
	j["clickCircleSettings.radius"] = clickCircleSettings.radius;
}

void ClickEffect::DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
{
	//DrawItemSettings();
	ImGui::Checkbox(u8"游戏内启用", &enabledInGame);
	ImGui::Checkbox(u8"游戏内菜单启用", &enabledInGameMenu);
	ImGui::Checkbox(u8"无限GUI菜单内启用", &enabledInMenu);
	ImGui::Checkbox(u8"左键", &enableLeftClick);
	ImGui::Checkbox(u8"右键", &enableRightClick);
	ImGuiStd::TextShadow(u8"圆圈特效设置");
	ImGui::Separator();
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.3f);
	ImGui::SliderFloat(u8"动画速度", &clickCircleSettings.animSpeed, 3.0f, 30.0f, "%.1f");
	ImGui::SliderFloat(u8"半径", &clickCircleSettings.radius, 5.0f, 100.0f, "%.1f");
	ImGuiStd::EditColor(u8"颜色", clickCircleSettings.color, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::PopItemWidth();
}
