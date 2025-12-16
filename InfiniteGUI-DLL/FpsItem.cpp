#include "FpsItem.h"
#include "ImGuiStd.h"


void FpsItem::Toggle()
{
}

void FpsItem::Update()
{
	FPS = float(frameCount) / (float(updateIntervalMs) / 1000.0f);
	frameCount = 0;
}
void FpsItem::DrawContent()
{
	frameCount++;
	int FPS = int(this->FPS);
	ImVec4 targetTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
	//获取io
	ImGuiIO& io = ImGui::GetIO();
	//计算速度
	float speed = 3.0f * io.DeltaTime;
	color.color = ImLerp(color.color, targetTextColor, speed);
	ImGuiStd::TextColoredShadow(color.color, (prefix + std::to_string(FPS) + suffix).c_str());
}

void FpsItem::DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
{
	//DrawItemSettings();
	ImGui::Text(u8"FPS 无额外设置");
	DrawAffixSettings(bigPadding, centerX, itemWidth);
	DrawWindowSettings(bigPadding, centerX, itemWidth);
}

void FpsItem::Load(const nlohmann::json& j)
{
	LoadItem(j);
	LoadAffix(j);
	LoadWindow(j);
}
void FpsItem::Save(nlohmann::json& j) const
{
	SaveItem(j);
	SaveAffix(j);
	SaveWindow(j);
}