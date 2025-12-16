#pragma once
#include "ModuleButton.h"
#include "MyButton.h"
#include "ItemManager.h"
#include <vector>

#include "SoundModule.h"
#include "WindowModule.h"

constexpr ImVec2 mainPanelSize = ImVec2(900.0f - 190.0f, 556.0f);
constexpr float moduleCardWidth = mainPanelSize.x - padding * 4.0f;

enum ModuleButtonType
{
	Button_Setting,
	Button_Lock,
	Button_Sound
};

struct ModuleSideButton
{
	MyButton* button;
	ModuleButtonType type;
};

class ModuleCard
{
public:
	ModuleCard(Item* item)
	{
		m_item = item;
		//分析item
		if (auto sound = dynamic_cast<SoundModule*>(item))
		{

			MyButton* soundButton = new MyButton(u8"\uE060", ImVec2(moduleButtonSize.y, moduleButtonSize.y));
			if (sound->IsPlaySound())
			{
				soundButton->SetSelected(true);
			}
			else
			{
				soundButton->SetLabelText(u8"\uE061");
				soundButton->SetSelected(false);
			}
			m_Buttons.push_back({ soundButton, Button_Sound });
		}
		if (auto win = dynamic_cast<WindowModule*>(item))
		{
			MyButton* lockButton = new MyButton(u8"\uE013", ImVec2(moduleButtonSize.y, moduleButtonSize.y));
			if (win->IsClickThrough())
			{
				lockButton->SetSelected(true);
			}
			else
			{
				lockButton->SetLabelText(u8"\uE014");
				lockButton->SetSelected(false);
			}
			m_Buttons.push_back({ lockButton, Button_Lock });
		}
		MyButton* settingButton = new MyButton("~", ImVec2(moduleButtonSize.y, moduleButtonSize.y));
		m_Buttons.push_back({ settingButton, Button_Setting });
		moduleButtonWidth = moduleCardWidth - (float)m_Buttons.size() * (moduleButtonSize.y + padding);
		m_moduleButton = new ModuleButton(item->icon.c_str(), item->name.c_str(), item->description.c_str(), ImVec2(moduleButtonWidth, moduleButtonSize.y));
	}

	~ModuleCard()
	{
		delete m_moduleButton;
		for (auto& b : m_Buttons)
		{
			delete b.button;
		}
	}

	bool Draw()
	{
		bool joinSetting = false;
		ImVec2 Pos = ImGui::GetCursorPos();
		startPos = Pos;
		m_moduleButton->SetSelected(m_item->isEnabled);
		if (m_moduleButton->Draw())
		{
			m_item->isEnabled = !m_item->isEnabled;
			m_item->Toggle();
		}
		if (m_moduleButton->IsRightClicked())
		{
			joinSetting = true;
		}
		ImGui::PushFont(opengl_hook::gui.iconFont, 24.0f);
		for (auto& b : m_Buttons) 
		{
			ImGui::SetCursorPos(ImVec2(Pos.x + moduleButtonWidth + padding, Pos.y));
			if (b.button->Draw())
			{
				if(b.type == Button_Setting)
					joinSetting = true;
				else if(b.type == Button_Lock)
				{
					auto win = dynamic_cast<WindowModule*>(m_item);
					if (win)
					{
						win->SetClickThrough(!win->IsClickThrough());
						b.button->SetSelected(win->IsClickThrough());
						if (win->IsClickThrough())
						{
							b.button->SetLabelText(u8"\uE013");
						}
						else
						{
							b.button->SetLabelText(u8"\uE014");
						}
					}
				}
				else if(b.type == Button_Sound)
				{
					auto sound = dynamic_cast<SoundModule*>(m_item);
					if (sound)
					{
						sound->SetPlaySound(!sound->IsPlaySound());
						b.button->SetSelected(sound->IsPlaySound());
						if (sound->IsPlaySound())
						{
							b.button->SetLabelText(u8"\uE060");
						}
						else
						{
							b.button->SetLabelText(u8"\uE061");
						}
					}
				}
			}
			Pos.x += moduleButtonSize.y + padding;
		}

		ImGui::PopFont();
		ImGui::SetCursorPos(ImVec2(startPos.x, startPos.y + moduleButtonSize.y + padding));
		return joinSetting;
	} //返回是否进入设置界面

	Item* GetItem() const { return m_item; }
private:
	ImVec2 startPos = ImVec2(0.0f, 0.0f);
	float moduleButtonWidth = 0.0f;
	Item* m_item;
	ModuleButton* m_moduleButton;
	std::vector<ModuleSideButton> m_Buttons;
};
