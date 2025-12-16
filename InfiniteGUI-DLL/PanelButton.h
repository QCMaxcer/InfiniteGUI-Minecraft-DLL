#pragma once
#include <string>

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

#include "opengl_hook.h"

#include "AnimButtonBase.h"

struct PanelButtonStateData
{
	ButtonAnimTarget button; // size + pos
	TextAnimTarget   icon;
	TextAnimTarget   label;
};

constexpr float textSpacing = 5.0f; //图标与文字的间距

class PanelButton : public AnimButtonBase
{
public:
	PanelButton(const char* iconText, const char* labelText, bool isExit = false, const ImVec2& size = panelButtonSize, const float& padding = 20.0f)
	{
		this->isExitButton = isExit;
		this->iconText.text = iconText;
		this->labelText.text = labelText;
		m_padding = padding;
		m_normal.button.size = size;
	}
	~PanelButton() = default;

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

		DrawBackground(m_current.button, flags);
		DrawBorder(m_current.button, flags);
		DrawLabel(m_current.icon, iconText);
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
private:
	bool isExitButton = false; //是否是退出按钮

	//图标
	ButtonText iconText;
	float iconFontSize = 24.0f;
	//文字
	ButtonText labelText;

	PanelButtonStateData m_normal; //未激活普通状态
	PanelButtonStateData m_selected; //激活的普通状态
	PanelButtonStateData m_hovered; //鼠标悬停状态
	PanelButtonStateData m_active; //鼠标按住状态
	PanelButtonStateData m_current; //当前状态
	PanelButtonStateData* m_target; //用这个指针设置目标状态


	void ApllyCenterPositionChange() override
	{
		ImVec2 value = CalPostionChangedValue(screenPos, lastScreenPos);
		ApllyButtonPositionChange(m_current.button, value);
		ApllyTextPositionChange(m_current.label, value);
		ApllyTextPositionChange(m_current.icon, value);
	}

	void SetNextCursorScreenPos() const //下一个控件位置一定由初始位置 + 初始大小 + 边距决定，否则会发生偏移
	{
		ImVec2 nextPos = screenPos;
		nextPos.y = screenPos.y + m_normal.button.size.y + m_padding;
		ImGui::SetCursorScreenPos(nextPos);
	}

	void SetStateData() override //设置状态数据
	{
		this->iconText.font = opengl_hook::gui.iconFont;
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
		bgColor.w = 0.35f;
		m_normal.button.color = bgColor;
		ImVec4 borderColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Border));
		borderColor.w = 0.0f;
		m_normal.button.borderColor = borderColor;

		//设置m_normal的图标
		//图标居中显示，灰色
		m_normal.icon.fontSize = iconFontSize;
		//位置在按钮中心对齐
		m_normal.icon.center = ImVec2(screenPos.x + m_normal.button.size.x / 2, screenPos.y + m_normal.button.size.y / 2);
		//m_normal.icon.CalculatePos();
		m_normal.icon.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TextDisabled));

		//设置m_normal的文字
		//文字居中显示，透明度为0
		m_normal.label.fontSize = ImGui::GetFontSize() * 1.0f;
		m_normal.label.center = ImVec2(screenPos.x + m_normal.button.size.x / 2, screenPos.y + m_normal.button.size.y / 2);
		//m_normal.label.CalculatePos();
		ImVec4 labelColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TextDisabled));
		labelColor.w = 0.0f;
		m_normal.label.color = labelColor;
	}

	void SetSelectedStateData(const PanelButtonStateData& normal)
	{
		//设置m_selected的按钮
		//按钮状态与普通状态相同
		m_selected.button.size = normal.button.size;
		m_selected.button.center = normal.button.center;
		m_selected.button.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Button));
		m_selected.button.borderColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Border));

		//设置m_selected的图标
		//图标向左移动，并正常显示
		m_selected.icon.fontSize = iconFontSize * 1.1f;

		m_selected.icon.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text));

		//设置m_selected的文字
		//文字向右移动，并正常显示
		m_selected.label.fontSize = ImGui::GetFontSize() * 1.0f;
		if (isExitButton)
			m_selected.label.color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
		else
			m_selected.label.color = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);

		ComputeTextLayout(
			m_selected.icon,
			m_selected.label,
			m_selected.button,
			opengl_hook::gui.iconFont,
			labelText.font ? labelText.font : ImGui::GetFont(),
			textSpacing
		);

	}

	void SetHoveredStateData(const PanelButtonStateData& normal)
	{
		//设置m_hovered的按钮
		//hovered 按钮稍稍抬起，大小稍稍变大，但是按钮依旧水平居中
		m_hovered.button.size = ImVec2(normal.button.size.x + buttonSizeOffset, normal.button.size.y + buttonSizeOffset);
		m_hovered.button.center = ImVec2(normal.button.center.x, normal.button.center.y - buttonHeightOffset);
		m_hovered.button.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_ButtonHovered));
		ImVec4 borderColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Border));
		borderColor.w = 0.5f;
		m_hovered.button.borderColor = borderColor;

		//图标向左移动，并正常显示
		m_hovered.icon.fontSize = iconFontSize * 1.1f;
		m_hovered.icon.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text));

		//设置m_hovered的文字
		//文字向右移动，并正常显示
		m_hovered.label.fontSize = ImGui::GetFontSize() * 1.0f;
		m_hovered.label.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text));

		ComputeTextLayout(
			m_hovered.icon,
			m_hovered.label,
			m_hovered.button,
			opengl_hook::gui.iconFont,
			labelText.font ? labelText.font : ImGui::GetFont(),
			textSpacing
		);

	}

	void SetActiveStateData(const PanelButtonStateData& normal)
	{
		//设置m_active的按钮
		//按钮变小，中心位置向下移动
		m_active.button.size = ImVec2(normal.button.size.x - buttonSizeOffset, normal.button.size.y - buttonSizeOffset);
		m_active.button.center = ImVec2(normal.button.center.x, normal.button.center.y + buttonHeightOffset);
		m_active.button.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_ButtonActive));
		ImVec4 borderColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Border));
		borderColor.w = 1.0f;
		m_active.button.borderColor = borderColor;

		//设置m_active的图标
		//图标向左移动，并正常显示
		m_active.icon.fontSize = iconFontSize * 0.9f;
		m_active.icon.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text));

		//设置m_active的文字
		//文字向右移动，并正常显示
		m_active.label.fontSize = ImGui::GetFontSize() * 0.9f;
		if (isExitButton)
			m_active.label.color = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
		else
			m_active.label.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text));

		ComputeTextLayout(
			m_active.icon,
			m_active.label,
			m_active.button,
			opengl_hook::gui.iconFont,
			labelText.font ? labelText.font : ImGui::GetFont(),
			2.0f
		);

	}

	void ComputeTextLayout(
		TextAnimTarget& icon,
		TextAnimTarget& label,
		const ButtonAnimTarget& button,
		ImFont* iconFont,
		ImFont* labelFont,
		float spacing) const
	{
		float icon_w = iconFont->CalcTextSizeA(icon.fontSize, FLT_MAX, 0, iconText.text).x;
		float label_w = labelFont->CalcTextSizeA(label.fontSize, FLT_MAX, 0, labelText.text).x;

		float total_w = icon_w + spacing + label_w;

		float center_x = button.center.x;
		float start_x = center_x - total_w * 0.5f;

		float icon_x = start_x;
		float label_x = start_x + icon_w + spacing;

		icon.center.x = icon_x + icon_w * 0.5f;
		label.center.x = label_x + label_w * 0.5f;

		// 垂直方向保持居中
		icon.center.y = button.center.y;
		label.center.y = button.center.y;
	}


	void LerpAll(PanelButtonStateData& cur, const PanelButtonStateData& target, float damping, float deltaTime)
	{
		LerpButton(cur.button, target.button, damping, deltaTime);
		LerpText(cur.icon, target.icon, damping, deltaTime);
		LerpText(cur.label, target.label, damping, deltaTime);
	}

	bool IsAnimating() const
	{
		return !IsAllEqual(m_current, *m_target);
	}

	static bool IsAllEqual(const PanelButtonStateData& a, const PanelButtonStateData& b)
	{
		return IsEqual(a.button, b.button)
			&& IsEqual(a.icon, b.icon)
			&& IsEqual(a.label, b.label);
	}


};
