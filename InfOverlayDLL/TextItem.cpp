#include "TextItem.h"
#include "ImGuiStd.h"

void TextItem::Toggle()
{
}

void TextItem::DrawContent()
{
    ImGuiStd::TextShadow((prefix + text + suffix).c_str());
    //ImGui::Text((prefix + text + suffix).c_str());
}

void TextItem::DrawSettings()
{
    DrawItemSettings();
    ImGuiStd::InputTextStd(u8"文本内容", text);
    if (ImGui::CollapsingHeader(u8"通用设置"))
    {
        DrawWindowSettings();
        DrawAffixSettings();
    }
}

void TextItem::Load(const nlohmann::json& j)
{
    LoadItem(j);
    LoadAffix(j);
    LoadWindow(j);
    if (j.contains("text")) text = j["text"];
}

void TextItem::Save(nlohmann::json& j) const
{
    SaveItem(j);
    SaveWindow(j);
    SaveAffix(j);
    j["text"] = text;
}