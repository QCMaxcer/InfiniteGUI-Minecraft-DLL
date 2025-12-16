#pragma once
#include "AnimButtonBase.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

#include "opengl_hook.h"

struct MyButtonStateData
{
	ButtonAnimTarget button; // size + pos
	TextAnimTarget   label;
};

class MyButton : public AnimButtonBase
{
public:
	MyButton(const char* labelText, const ImVec2& size = panelButtonSize, const float& padding = 20.0f)
	{
		this->labelText.text = labelText;
		m_padding = padding;
		m_normal.button.size = size;
		m_target = &m_normal;
		m_state = Normal;
		lastState = Normal;
	}
	~MyButton() = default;

	bool Draw(ImDrawFlags flags = ImDrawFlags_RoundCornersAll) override //返回是否被点击
	{
		if (!initialized)
		{
			screenPos = ImGui::GetCursorScreenPos(); //初始位置由ImGui自动计算
			lastScreenPos = screenPos;
			SetStateData();
			//设置m_current的状态
			m_current = m_normal;
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
		if (lastState != m_state)
		{
			skipAnim = false;
			lastState = m_state;
		}
		if (!IsAnimating())
		{
			skipAnim = true;
		}
		if (!skipAnim)
		{
			ImGuiIO& io = ImGui::GetIO();
			LerpAll(m_current, *m_target, animSpeed, io.DeltaTime);
		}
		// -------------------------------------------------
		// 绘制
		// -------------------------------------------------

		DrawBackground(m_current.button);
		DrawBorder(m_current.button);
		DrawLabel(m_current.label, labelText);

		//if (!skipAnim)
		//{
		//	ImGui::SameLine();
		//	ImGui::Text(u8"Animation");
		//}

		// 为下一个控件设置 Cursor ScreenPos（按钮垂直堆叠）
		SetNextCursorScreenPos();

		// 返回是否被点击（按下 -> 松开的那一帧）
		return pressed;
	}

	void SetSelected(bool selected)
	{
		m_state = selected ? Selected : Normal;
		m_target = m_state == Selected ? &m_selected : &m_normal;
	}

	void SetLabelText(const char* labelText)
	{
		this->labelText.text = labelText;
	}

	bool IsSelected() const
	{
		return m_state == Selected;
	}

	ButtonState GetButtonState() const
	{
		return m_state;
	}
	void SetSize(const ImVec2 size);
	void SetPosition(const ImVec2 pos);
protected:

	//文字
	ButtonText labelText;

	MyButtonStateData m_normal; //未激活普通状态
	MyButtonStateData m_selected; //激活的普通状态
	MyButtonStateData m_hovered; //鼠标悬停状态
	MyButtonStateData m_active; //鼠标按住状态
	MyButtonStateData m_current; //当前状态
	MyButtonStateData* m_target; //用这个指针设置目标状态


	void ApllyCenterPositionChange() override
	{
		ImVec2 value = CalPostionChangedValue(screenPos, lastScreenPos);
		ApllyButtonPositionChange(m_current.button, value);
		ApllyTextPositionChange(m_current.label, value);
	}

	void SetNextCursorScreenPos() const //下一个控件位置一定由初始位置 + 初始大小 + 边距决定，否则会发生偏移
	{
		ImVec2 nextPos = screenPos;
		nextPos.y = screenPos.y + m_normal.button.size.y + m_padding;
		ImGui::SetCursorScreenPos(nextPos);
	}

	void SetStateData() override //设置状态数据
	{
		this->labelText.font = nullptr;

		SetNormalStateData();
		SetSelectedStateData(m_normal);
		SetHoveredStateData(m_normal);
		SetActiveStateData(m_normal);
	}

	void SetNormalStateData()
	{

		//计算button的中心位置
		m_normal.button.CalculateCenter(screenPos);

		//设置m_normal的按钮背景颜色
		ImVec4 bgColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Button));
		//bgColor.w = 0.35f;
		m_normal.button.color = bgColor;
		ImVec4 borderColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Border));
		borderColor.w = 0.3f;
		m_normal.button.borderColor = borderColor;

		//设置m_normal的文字
		//文字居中显示，透明度为0
		m_normal.label.fontSize = ImGui::GetFontSize() * 1.0f;
		m_normal.label.center = ImVec2(screenPos.x + m_normal.button.size.x / 2, screenPos.y + m_normal.button.size.y / 2);
		//m_normal.label.CalculatePos();
		ImVec4 labelColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TextDisabled));
		m_normal.label.color = labelColor;
	}

	void SetSelectedStateData(const MyButtonStateData& normal)
	{
		//设置m_selected的按钮
		//按钮状态与普通状态相同
		m_selected.button.size = normal.button.size;
		m_selected.button.center = normal.button.center;
		m_selected.button.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_ButtonHovered));
		m_selected.button.borderColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Border));

		//设置m_selected的文字
		//文字向右移动，并正常显示
		ImVec2 buttonPos = m_selected.button.CalculatePos();
		ImVec2 textCenter = ImVec2(buttonPos.x + m_selected.button.size.x / 2, buttonPos.y + m_selected.button.size.y / 2);		//设置m_hovered的图标

		m_selected.label.center = textCenter;
		m_selected.label.fontSize = ImGui::GetFontSize() * 1.0f;
		m_selected.label.color = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);

	}

	void SetHoveredStateData(const MyButtonStateData& normal)
	{
		//设置m_hovered的按钮
		//hovered 按钮稍稍抬起，大小稍稍变大，但是按钮依旧水平居中
		m_hovered.button.size = ImVec2(normal.button.size.x + buttonSizeOffset, normal.button.size.y + buttonSizeOffset);
		m_hovered.button.center = ImVec2(normal.button.center.x, normal.button.center.y - buttonHeightOffset);
		m_hovered.button.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_ButtonHovered));
		ImVec4 borderColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Border));
		borderColor.w = 0.5f;
		m_hovered.button.borderColor = borderColor;

		ImVec2 buttonPos = m_hovered.button.CalculatePos();
		ImVec2 textCenter = ImVec2(buttonPos.x + m_hovered.button.size.x / 2, buttonPos.y + m_hovered.button.size.y / 2);		//设置m_hovered的图标

		//设置m_hovered的文字
		//文字向右移动，并正常显示
		m_hovered.label.center = textCenter;
		m_hovered.label.fontSize = ImGui::GetFontSize() * 1.0f;
		m_hovered.label.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text));

	}

	void SetActiveStateData(const MyButtonStateData& normal)
	{
		//设置m_active的按钮
		//按钮变小，中心位置向下移动
		m_active.button.size = ImVec2(normal.button.size.x - buttonSizeOffset, normal.button.size.y - buttonSizeOffset);
		m_active.button.center = ImVec2(normal.button.center.x, normal.button.center.y + buttonHeightOffset);
		m_active.button.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_ButtonActive));
		ImVec4 borderColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Border));
		borderColor.w = 1.0f;
		m_active.button.borderColor = borderColor;

		ImVec2 buttonPos = m_active.button.CalculatePos();
		ImVec2 textCenter = ImVec2(buttonPos.x + m_active.button.size.x / 2, buttonPos.y + m_active.button.size.y / 2);		//设置m_hovered的图标

		//设置m_active的文字
		//文字向右移动，并正常显示
		m_active.label.center = textCenter;
		m_active.label.fontSize = ImGui::GetFontSize() * 0.9f;
		m_active.label.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text));

	}


	void LerpAll(MyButtonStateData& cur, const MyButtonStateData& target, float damping, float deltaTime)
	{
		LerpButton(cur.button, target.button, damping, deltaTime);
		LerpText(cur.label, target.label, damping, deltaTime);
	}

	bool IsAnimating() const
	{
		return !IsAllEqual(m_current, *m_target);
	}

	static bool IsAllEqual(const MyButtonStateData& a, const MyButtonStateData& b)
	{
		return IsEqual(a.button, b.button)
			&& IsEqual(a.label, b.label);
	}


};