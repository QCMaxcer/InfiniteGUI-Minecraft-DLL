#pragma once
#include "AnimButtonBase.h"

struct ModuleButtonStateData
{
	ButtonAnimTarget button; // size + pos
	TextAnimTarget   icon;
	TextAnimTarget   label;
	TextAnimTarget   description;
};

constexpr float textSpacingY = 0.0f;
constexpr ImVec2 moduleButtonSize = ImVec2(548.0f, 60.0f);
class ModuleButton : public AnimButtonBase
{
public:
	ModuleButton(const char* iconText, const char* labelText, const char* descriptionText, const ImVec2& size = moduleButtonSize, const float& padding = 20.0f)
	{
		this->iconText.text = iconText;
		this->labelText.text = labelText;
		this->descriptionText.text = descriptionText;
		m_padding = padding;
		m_normal.button.size = size;
	}
	~ModuleButton() = default;

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
			else if (hovered) m_state = Hovered; //覆盖hover
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
		DrawBorder(m_current.button, flags, 2.0f);
		DrawLabel(m_current.icon, iconText);
		DrawLabel(m_current.label, labelText);
		DrawLabel(m_current.description, descriptionText);

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

	//文字
	ButtonText iconText;
	float iconFontSize = 24.0f;
	ButtonText labelText;
	ButtonText descriptionText; //描述文字

	ModuleButtonStateData m_normal; //未激活普通状态
	ModuleButtonStateData m_selected; //激活的普通状态
	ModuleButtonStateData m_hovered; //鼠标悬停状态
	ModuleButtonStateData m_active; //鼠标按住状态
	ModuleButtonStateData m_current; //当前状态
	ModuleButtonStateData* m_target; //用这个指针设置目标状态

	void ApllyCenterPositionChange() override
	{
		ImVec2 value = CalPostionChangedValue(screenPos, lastScreenPos);
		ApllyButtonPositionChange(m_current.button, value);
		ApllyTextPositionChange(m_current.label, value);
		ApllyTextPositionChange(m_current.description, value);
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
		ImVec4 borderColor = ImVec4(0.8f, 0.2f, 0.2f, 0.8f); 
		m_normal.button.borderColor = borderColor;//红色边框 代表关闭状态

		//设置m_normal的图标
		//图标靠左显示，灰色
		m_normal.icon.fontSize = iconFontSize;
		//位置在按钮靠左
		
		m_normal.icon.center = ImVec2(screenPos.x + m_normal.button.size.y / 2, screenPos.y + m_normal.button.size.y / 2);
		m_normal.icon.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TextDisabled));

		//设置m_normal的文字
		//文字在图标右边显示，上下居中
		m_normal.label.fontSize = ImGui::GetFontSize() * 1.0f;
		ImVec2 labelPos = ImVec2(screenPos.x + m_normal.button.size.y, screenPos.y + (m_normal.button.size.y - m_normal.label.fontSize) * 0.5f);
		m_normal.label.CalculateCenter(labelPos, labelText);
		ImVec4 labelColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text));
		m_normal.label.color = labelColor;

		//描述与文字高度重叠，但是颜色为灰色透明

		m_normal.description.fontSize = ImGui::GetFontSize() * 0.8f;
		ImVec2 descriptionPos = ImVec2(screenPos.x + m_normal.button.size.y, screenPos.y + (m_normal.button.size.y - m_normal.description.fontSize) * 0.5f);
		m_normal.description.CalculateCenter(descriptionPos, descriptionText);
		ImVec4 descriptionColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TextDisabled));
		descriptionColor.w = 0.0f;
		m_normal.description.color = descriptionColor;

	}

	void SetSelectedStateData(const ModuleButtonStateData& normal)
	{
		//设置m_selected的按钮
		//按钮状态与普通状态相同
		m_selected.button.size = normal.button.size;
		m_selected.button.center = normal.button.center;
		m_selected.button.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Button));
		ImVec4 borderColor = ImVec4(0.2f, 0.8f, 0.2f, 0.8f);
		m_selected.button.borderColor = borderColor; //绿色边框 代表开启状态

		//设置m_selected的图标
		//图标正常显示
		m_selected.icon.fontSize = iconFontSize * 1.0f;
		m_selected.icon.center = normal.icon.center;
		m_selected.icon.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text));

		//设置m_selected的文字
		//文字向上移动，并正常显示
		m_selected.label.fontSize = ImGui::GetFontSize() * 1.0f;
		m_selected.label.center = normal.label.center;
		m_selected.label.color = normal.label.color;

		m_selected.description.fontSize = ImGui::GetFontSize() * 0.8f;
		m_selected.description.center = normal.description.center;
		//ImVec4 descriptionColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TextDisabled));
		m_selected.description.color = normal.description.color;


	}
	

	void SetHoveredStateData(const ModuleButtonStateData& normal)
	{
		//设置m_hovered的按钮
		//hovered 按钮稍稍抬起，大小稍稍变大，但是按钮依旧水平居中
		m_hovered.button.size = ImVec2(normal.button.size.x + buttonSizeOffset, normal.button.size.y + buttonSizeOffset);
		m_hovered.button.center = ImVec2(normal.button.center.x, normal.button.center.y - buttonHeightOffset);
		m_hovered.button.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_ButtonHovered));
		ImVec4 borderColor = ImVec4(0.8f, 0.8f, 0.8f, 0.5f);
		m_hovered.button.borderColor = borderColor;

		//设置m_selected的图标
		//图标正常显示
		m_hovered.icon.fontSize = iconFontSize * 1.2f;
		m_hovered.icon.center = ImVec2(normal.icon.center.x, screenPos.y + m_hovered.button.size.y / 2 - buttonHeightOffset);
		m_hovered.icon.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text));

		//设置m_selected的文字
		//文字向上移动，并正常显示
		m_hovered.label.fontSize = ImGui::GetFontSize() * 1.0f;
		m_hovered.label.color = normal.label.color;

		m_hovered.description.fontSize = ImGui::GetFontSize() * 0.8f;
		ImVec4 descriptionColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TextDisabled));
		m_hovered.description.color = descriptionColor;

		m_hovered.label.center.x = normal.label.center.x;
		m_hovered.description.center.x = normal.description.center.x;

		ComputeTextLayout_Vertical(m_hovered.label, m_hovered.description, m_hovered.button, textSpacingY);

	}

	void SetActiveStateData(const ModuleButtonStateData& normal)
	{
		//设置m_active的按钮
		//按钮变小，中心位置向下移动
		m_active.button.size = ImVec2(normal.button.size.x - buttonSizeOffset, normal.button.size.y - buttonSizeOffset);
		m_active.button.center = ImVec2(normal.button.center.x, normal.button.center.y + buttonHeightOffset);
		m_active.button.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_ButtonActive));
		ImVec4 borderColor = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
		m_active.button.borderColor = borderColor;

		//设置m_active的图标
		//图标正常显示
		m_active.icon.fontSize = iconFontSize * 1.0f;
		m_active.icon.center = ImVec2(normal.icon.center.x, screenPos.y + m_active.button.size.y * 0.5f + buttonHeightOffset + buttonSizeOffset * 0.5f);
		m_active.icon.color = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text));

		//设置m_active的文字
		//文字向上移动，并正常显示
		m_active.label.fontSize = ImGui::GetFontSize() * 1.0f;
		m_active.label.color = normal.label.color;

		m_active.description.fontSize = ImGui::GetFontSize() * 0.8f;
		ImVec4 descriptionColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TextDisabled));
		m_active.description.color = descriptionColor;

		m_active.label.center.x = normal.label.center.x;
		m_active.description.center.x = normal.description.center.x;

		ComputeTextLayout_Vertical(m_active.label, m_active.description, m_active.button, -4.0f);
	}

	static void ComputeTextLayout_Vertical(
		TextAnimTarget& label,
		TextAnimTarget& description,
		const ButtonAnimTarget& button,
		float spacing)
	{
		// 计算高度（注意：用 fontSize 作为高度基准）
		float icon_h = label.fontSize;
		float label_h = description.fontSize;

		float total_h = icon_h + spacing + label_h;

		float center_y = button.center.y;
		float start_y = center_y - total_h * 0.5f;

		float icon_y = start_y;
		float label_y = start_y + icon_h + spacing;

		// 垂直方向中心
		label.center.y = icon_y + icon_h * 0.5f;
		description.center.y = label_y + label_h * 0.5f;
	}



	void LerpAll(ModuleButtonStateData& cur, const ModuleButtonStateData& target, float damping, float deltaTime)
	{
		LerpButton(cur.button, target.button, damping, deltaTime);
		LerpText(cur.icon, target.icon, damping, deltaTime);
		LerpText(cur.label, target.label, damping, deltaTime);
		LerpText(cur.description, target.description, damping, deltaTime);
	}

	bool IsAnimating() const
	{
		return !IsAllEqual(m_current, *m_target);
	}

	static bool IsAllEqual(const ModuleButtonStateData& a, const ModuleButtonStateData& b)
	{
		return IsEqual(a.button, b.button)
			&& IsEqual(a.icon, b.icon)
			&& IsEqual(a.label, b.label)
			&& IsEqual(a.description, b.description);
	}

};
