#pragma once
#include "CategoryBar.h"
#include "CounterItem.h"
#include "FileCountItem.h"
#include "ModuleButton.h"
#include "ModuleCard.h"
#include "ModuleSettings.h"
#include "ItemManager.h"
#include "TextItem.h"

class ModulesPanel
{
public:
	ModulesPanel()
	{
		m_categoryBar = new CategoryBar();
		m_moduleSettings = new ModuleSettings();
		//m_moduleButton = new ModuleButton("1", "TestModule", "TestModuleDesc", ImVec2(548, 60));
	}
	~ModulesPanel()
	{
		delete m_categoryBar;
		delete m_moduleSettings;
		for (auto moduleCard : m_moduleCard)
		{
			delete moduleCard;
		}
		for (auto moduleButton : m_moduleButtons)
		{
			delete moduleButton;
		}
		//delete m_moduleButton;
	}

	void Init()
	{
		for (auto moduleCard : m_moduleCard)
		{
			delete moduleCard;
		}
		for (auto item : ItemManager::Instance().GetAllItems())
		{
			if (item->type != Hidden)
				m_moduleCard.push_back(new ModuleCard(item));
		}
	}
	void Draw()
	{
		if (!isInModuleSettings)
		{
			if (DrawModuleList())
			{
				isInModuleSettings = true;
			}
		}
		else
		{
			if (DrawModuleSettings())
			{
				isInModuleSettings = false;
			}
		}
	}

private:

	bool DrawModuleList()
	{
		bool joinModuleSettings = false;
		//获取窗口是否在scroll
		ImVec2 startPos = ImGui::GetCursorPos();
		int typeIndex = m_categoryBar->Draw();
		startPos.y += m_categoryBar->GetButtonHeight() + 13;
		ImGui::SetCursorPos(startPos);
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;
		ImGui::BeginChild("ModuleList", ImVec2(-padding + ImGui::GetStyle().WindowPadding.x, -padding + ImGui::GetStyle().WindowPadding.y), true, flags);
		ImGui::SetCursorPos(ImVec2(padding, padding));
		for (auto moduleCard : m_moduleCard)
		{
			if (moduleCard->Draw())
			{
				joinModuleSettings = true;
				selectedItem = moduleCard->GetItem();
			}
		}
		ImGui::Dummy(ImVec2(0, 10));
		ImGui::EndChild();
		return joinModuleSettings;
	}

	bool DrawModuleSettings() const
	{
		bool exit = false;
		if(m_moduleSettings->Draw(selectedItem))
			exit = true;
		return exit;
	}

	static bool IsWindowScrollingY()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (!window)
			return false;

		static float lastScrollY = 0.0f;
		float curScrollY = window->Scroll.y;

		bool scrolling = (curScrollY != lastScrollY);
		lastScrollY = curScrollY;
		return scrolling;
	}
	static bool UsingMouseWheel()
	{
		return ImGui::GetIO().MouseWheel != 0.0f;
	}
	bool isInModuleSettings = false;
	Item *selectedItem = nullptr;
	CategoryBar* m_categoryBar;
	ModuleSettings* m_moduleSettings; //二级设置 
	std::vector<ModuleCard*> m_moduleCard;
	std::vector<ModuleButton*> m_moduleButtons;
};
