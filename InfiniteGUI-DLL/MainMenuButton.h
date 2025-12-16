#pragma once

#include "MyButton.h"

class MainMenuButton : public MyButton
{
public:
	MainMenuButton(const char* labelText, const ImVec2& size = panelButtonSize, const float& padding = 20.0f) :
		MyButton(labelText, size, padding) {}

	~MainMenuButton() = default;

	bool Draw(ImDrawFlags flags = ImDrawFlags_RoundCornersAll) override //返回是否被点击
	{

		if (!initialized)
		{
			screenPos = ImGui::GetCursorScreenPos(); //初始位置由ImGui自动计算
			lastScreenPos = screenPos;
			SetStateData();
			//设置m_current的状态
			m_current = m_normal;
			m_target = &m_normal;
			m_state = Normal;
			lastState = Normal;
			initialized = true;
		}

		screenPos = ImGui::GetCursorScreenPos(); //初始位置由ImGui自动计算
		if (IsPositionChanged(screenPos, lastScreenPos))
		{
			SetStateData();
			ApllyCenterPositionChange();
			lastScreenPos = screenPos;
		}

		bool pressed = DrawInvisibleButton(m_current.button);
		rightClicked = ImGui::IsItemClicked(1); //右键单击
		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();

		// -------------------------------------------------
		// 状态机（根据鼠标事件选择动画目标）
		// -------------------------------------------------

		if (m_state == Selected)
		{
			if (active) m_state = Active;
			else if (hovered) m_state = Selected; //覆盖hover
			else m_state = Selected;
		}
		else
		{
			//正常状态机
			if (active)
				m_state = Active;
			else if (hovered)
				m_state = Hovered;
			else
				m_state = Normal;
		}

		// 更新动画目标
		switch (m_state)
		{
		case Normal:   m_target = &m_normal;   break;
		case Selected: m_target = &m_selected; break;
		case Hovered:  m_target = &m_hovered;  break;
		case Active:   m_target = &m_active;   break;
		}
		// -------------------------------------------------
		// 动画 Lerp（每帧插值）
		// -------------------------------------------------
		ImGuiIO& io = ImGui::GetIO();
		LerpAll(m_current, *m_target, animSpeed, io.DeltaTime);
		// -------------------------------------------------
		// 绘制
		// -------------------------------------------------
		DrawWindowBackground(m_current.button);
		DrawBackground(m_current.button);
		DrawBorder(m_current.button);
		DrawLabel(m_current.label, labelText);

		// 为下一个控件设置 Cursor ScreenPos（按钮垂直堆叠）
		SetNextCursorScreenPos();

		// 返回是否被点击（按下 -> 松开的那一帧）
		return pressed;
	}
	static void DrawWindowBackground(const ButtonAnimTarget& current, const ImDrawFlags& flags = ImDrawFlags_RoundCornersAll)
	{
		ImDrawList* draw = ImGui::GetWindowDrawList();

		ImVec2 pos = current.CalculatePos();
		pos.x -= 3.0f;
		pos.y -= 3.0f;
		ImVec2 size = current.size;
		size.x += 6.0f;
		size.y += 6.0f;

		draw->AddRectFilled(
			pos,
			ImVec2(pos.x + size.x, pos.y + size.y),
			ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_WindowBg]),
			ImGui::GetStyle().FrameRounding, // 圆角
			flags
		);
	}

};