#pragma once
#include "CategoryBar.h"
#include "ItemManager.h"
#include "menuRule.h"

class ModuleSettings
{
	public:
		static bool Draw(Item *item)
		{
			bool exit = false;
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 8.0f));
			//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 8.0f));
			ImGui::BeginChild("Settings", ImVec2(-padding + ImGui::GetStyle().WindowPadding.x, -padding + ImGui::GetStyle().WindowPadding.y), true, flags);
			//修改imgui控件与窗口边缘的间距
			ImGui::PushFont(opengl_hook::gui.iconFont);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f);
			if(ImGui::Button("i", ImVec2(35.0f, 35.0f)))
				exit = true;
			ImGui::PopStyleVar();
			ImGui::PopFont();
			ImVec2 separatorPos = ImGui::GetCursorPos();

			ImGui::SameLine();

			float buttonHeight = ImGui::GetCursorPosY();
			float titleHeight = ImGui::GetFontSize() * 1.2f;
			ImGui::PushFont(NULL, titleHeight);
	
			ImGui::SetCursorPosY((separatorPos.y - titleHeight) * 0.5f);

			ImGuiStd::TextShadow(item->name.c_str());
			ImGui::PopFont();

			ImGui::SameLine();

			ImGui::PushFont(opengl_hook::gui.iconFont);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f);
			static bool resetButtonHovered = false;
			float right = ImGui::GetWindowWidth();
			float buttonWidth =
				ImGui::GetStyle().ItemSpacing.x +
				35.0f;
			ImGui::SetCursorPos(ImVec2(right - buttonWidth, buttonHeight));
			if (!resetButtonHovered) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			if (ImGui::Button(u8"\uE050", ImVec2(35.0f, 35.0f)))
				item->Reset();
			if (!resetButtonHovered) ImGui::PopStyleColor();
			resetButtonHovered = ImGui::IsItemHovered();
				
			ImGui::PopStyleVar();
			ImGui::PopFont();
			ImGui::SetCursorPos(separatorPos);

			ImGui::Separator();

			ImGui::BeginDisabled();
			ImGui::PushFont(NULL, ImGui::GetFontSize() * 0.8f);
			ImGuiStd::TextShadow(item->description.c_str());
			ImGui::PopFont();
			ImGui::EndDisabled();

			ImGui::Separator();
			ImVec2 childPos = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(0.0f, childPos.y));
			//透明背景child
			ImGui::BeginChild("InnerSettings", ImVec2(ImGui::GetWindowWidth(),ImGui::GetWindowHeight() - childPos.y - ImGui::GetStyle().WindowPadding.y), true, ImGuiWindowFlags_NoBackground);

			// ===== 布局参数 =====
			ImGuiStyle& style = ImGui::GetStyle();
			float basePadding = style.WindowPadding.x;
			float bigPadding = basePadding * 3.0f;

			// 获取窗口可用宽度
			float contentWidth = ImGui::GetContentRegionAvail().x;
			float centerX = contentWidth * 0.5f;

			// 每个输入框宽度（留点余量，避免顶到边）
			float itemWidth = centerX - bigPadding * 4.0f;


			item->DrawSettings(bigPadding, centerX, itemWidth);

			ImGui::EndChild();
			ImGui::EndChild();
			ImGui::PopStyleVar();
			return exit;
		}//返回true退出设置界面，返回false继续运行
	private:

};
