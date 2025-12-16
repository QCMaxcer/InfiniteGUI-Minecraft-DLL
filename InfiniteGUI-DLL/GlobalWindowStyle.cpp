#include "GlobalWindowStyle.h"

void GlobalWindowStyle::Toggle()
{
}

void GlobalWindowStyle::Load(const nlohmann::json& j)
{
	LoadItem(j);
	LoadStyle(j);
}

void GlobalWindowStyle::Save(nlohmann::json& j) const
{
	SaveItem(j);
	SaveStyle(j);
}

void GlobalWindowStyle::DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
{
	DrawStyleSettings(bigPadding, centerX, itemWidth);
}

ItemStyle& GlobalWindowStyle::GetGlobeStyle()
{
	return itemStyle;
}