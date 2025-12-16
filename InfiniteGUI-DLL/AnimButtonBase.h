#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <cmath>
#include "Anim.h"
struct ButtonText
{
	const char* text;
	ImFont* font;
};

enum ButtonState
{
	Normal, //普通状态
	Selected, //选中状态
	Hovered, //悬停状态
	Active //按住状态
};

struct TextAnimTarget
{
	float fontSize;
	ImVec2 center;
	ImVec4 color;
	ImVec2 CalculatePos(const ButtonText& text) const //pos不应当作为动画的目标变量。动画只由中心位置、大小、颜色决定
	{
		ImGui::PushFont(text.font, fontSize);
		ImVec2 textSize = ImGui::CalcTextSize(text.text);  // 获取文本实际像素宽高
		ImGui::PopFont();

		ImVec2 pos = ImVec2(
			center.x - textSize.x * 0.5f,  // 左上角 = 中心 - 一半宽度
			center.y - textSize.y * 0.5f   // 左上角 = 中心 - 一半高度
		);
		return pos;
	}
	void CalculateCenter(const ImVec2& pos, const ButtonText& text)
	{
		ImGui::PushFont(text.font, fontSize);
		ImVec2 textSize = ImGui::CalcTextSize(text.text);  // 获取文本实际像素宽高
		ImGui::PopFont();

		center = ImVec2(pos.x + textSize.x * 0.5f, pos.y + fontSize / 2);
	}
};

struct ButtonAnimTarget
{
	ImVec2 size;
	ImVec2 center;
	ImVec4 color;
	ImVec4 borderColor;
	ImVec2 CalculatePos() const
	{
		ImVec2 pos = ImVec2(center.x - size.x / 2, center.y - size.y / 2);
		return pos;
	}
	void CalculateCenter(const ImVec2& pos)
	{
		center = ImVec2(pos.x + size.x / 2, pos.y + size.y / 2);
	}
};

constexpr ImVec2 panelButtonSize = ImVec2(150.0f, 50.0f); //按钮大小
constexpr float buttonSizeOffset = 5.0f; //按钮大小缩小/放大的距离
constexpr float buttonHeightOffset = 3.0f; //按钮高度抬起/下降的距离

class AnimButtonBase : public Anim
{
public:
    virtual bool Draw(ImDrawFlags flags = ImDrawFlags_RoundCornersAll) = 0;
	bool IsRightClicked() const
	{
		return rightClicked; //右键单击
	}
protected:
    // 必须由子类实现
    virtual void SetStateData() = 0;

	ImVec2 screenPos;
	ImVec2 lastScreenPos;

    ButtonState m_state = Normal;
    ButtonState lastState = Normal;

    bool initialized = false;

	bool rightClicked = false; //右键单击

    bool skipAnim = false;

    float animSpeed = 15.0f;

    float m_padding = 8.0f;

	bool DrawInvisibleButton(const ButtonAnimTarget& current, const ImDrawFlags& flags = ImGuiButtonFlags_PressedOnClickRelease) const
	{
		// 获取当前按钮区域（使用 m_current 来绘制）
		ImVec2 buttonPos = current.CalculatePos();
		ImVec2 buttonSize = current.size;

		// 创建不可见的按钮用于接受鼠标事件
		ImGui::SetCursorScreenPos(buttonPos);
		ImGui::PushID(this);
		bool pressed = ImGui::InvisibleButton("##panelBtn", buttonSize, ImGuiButtonFlags_PressedOnClickRelease);
		ImGui::PopID();
		return pressed;
	}


	static void DrawLabel(const TextAnimTarget& current, const ButtonText& labelText)
	{
		if (labelText.text == nullptr) return;
		ImVec2 pos = current.CalculatePos(labelText);
		ImU32 col = ImGui::ColorConvertFloat4ToU32(current.color);

		DrawShadowText(labelText.font, current.fontSize, pos, col, labelText.text);
	}

	static void DrawBackground(const ButtonAnimTarget &current, const ImDrawFlags& flags = ImDrawFlags_RoundCornersAll)
	{
		ImDrawList* draw = ImGui::GetWindowDrawList();

		ImVec2 pos = current.CalculatePos();
		ImVec2 size = current.size;

		draw->AddRectFilled(
			pos,
			ImVec2(pos.x + size.x, pos.y + size.y),
			ImGui::ColorConvertFloat4ToU32(current.color),
			ImGui::GetStyle().FrameRounding, // 圆角
			flags
		);
	}

	static void DrawBorder(const ButtonAnimTarget& current, const ImDrawFlags& flags = ImDrawFlags_RoundCornersAll, const float borderSize = 1.0f)
	{
		ImDrawList* draw = ImGui::GetWindowDrawList();

		ImVec2 pos = current.CalculatePos();
		ImVec2 size = current.size;

		draw->AddRect(
			pos,
			ImVec2(pos.x + size.x, pos.y + size.y),
			ImGui::ColorConvertFloat4ToU32(current.borderColor),
			ImGui::GetStyle().FrameRounding, // 圆角
			flags,
			borderSize // 线宽
		);
	}

	static void DrawShadowText(ImFont* font, float fontSize, const ImVec2& pos, const ImU32& color, const char* text)
	{
		ImDrawList* draw = ImGui::GetWindowDrawList();
		float shadowOffset =/* fontSize * 0.08f*/1.0f;
		float alpha = ImGui::ColorConvertU32ToFloat4(color).w * 0.5f;
		ImU32 ShadowColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, alpha));
		draw->AddText(
			font,
			fontSize,
			ImVec2(pos.x + shadowOffset, pos.y + shadowOffset),
			ShadowColor,
			text
		);
		draw->AddText(
			font,
			fontSize,
			pos,
			color,
			text
		);
	}

	static void SetNextCursorScreenPos(const ButtonAnimTarget& normal, float padding = 8.0f, ImVec2 screenPos = ImVec2(0, 0)) //下一个控件位置一定由初始位置 + 初始大小 + 边距决定，否则会发生偏移
	{
		ImVec2 nextPos = screenPos;
		nextPos.y = screenPos.y + normal.size.y + padding;
		ImGui::SetCursorScreenPos(nextPos);
	}

	static bool IsPositionChanged(const ImVec2& pos, const ImVec2& lastPos)
	{
		return pos.x!= lastPos.x || pos.y!= lastPos.y;
	}

	static ImVec2 CalPostionChangedValue(const ImVec2& pos, const ImVec2& lastPos)
	{
		return ImVec2(pos.x - lastPos.x, pos.y - lastPos.y);
	}

	static void ApllyButtonPositionChange(ButtonAnimTarget& button, const ImVec2& posChange)
	{
		button.center.x += posChange.x;
		button.center.y += posChange.y;
	}

	static void ApllyTextPositionChange(TextAnimTarget& text, const ImVec2& posChange)
	{
		text.center.x += posChange.x;
		text.center.y += posChange.y;
	}

	virtual void ApllyCenterPositionChange() = 0;

	static void LerpButton(ButtonAnimTarget& cur, const ButtonAnimTarget& target, float damping, float deltaTime)
	{
		SmoothLerp(cur.size, target.size, damping, deltaTime);
		SmoothLerp(cur.center, target.center, damping, deltaTime);
		SmoothLerp(cur.color, target.color, damping, deltaTime);
		SmoothLerp(cur.borderColor, target.borderColor, damping, deltaTime);
	}

	static void LerpText(TextAnimTarget& cur, const TextAnimTarget& target, float damping, float deltaTime)
	{
		SmoothLerp(cur.fontSize, target.fontSize, damping, deltaTime);
		SmoothLerp(cur.center, target.center, damping, deltaTime);
		SmoothLerp(cur.color, target.color, damping, deltaTime);
	}

	static bool IsEqual(const ButtonAnimTarget& a, const ButtonAnimTarget& b)
	{
		return AlmostEqual(a.size, a.size)
			&& AlmostEqual(a.center, b.center)
			&& AlmostEqual(a.color, b.color)
			&& AlmostEqual(a.borderColor, b.borderColor);
	}

	static bool IsEqual(const TextAnimTarget& a, const TextAnimTarget& b)
	{
		return AlmostEqual(a.fontSize, b.fontSize)
			&& AlmostEqual(a.center, b.center)
			&& AlmostEqual(a.color, b.color);
	}
};
