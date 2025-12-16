#include <Windows.h>
#include "ItemManager.h"
#include "TimeItem.h"
#include "FpsItem.h"
#include "DanmakuItem.h"
#include "KeystrokesItem.h"
#include "CPSItem.h"
#include "BilibiliFansItem.h"
#include "FileCountItem.h"
#include "CounterItem.h"
#include "TextItem.h"

#include "Sprint.h"

#include "Motionblur.h"
#include "ClickEffect.h"

#include "CPSDetector.h"
#include "GameStateDetector.h"
#include "GlobalWindowStyle.h"

#include "Menu.h"
#include <algorithm>
#include <chrono>

// ------------------------------------------------
ItemManager::ItemManager()
{
    Init();
}

void ItemManager::Init()
{
    // 注册默认 Singleton
    AddItem(&Menu::Instance());

    AddItem(&Sprint::Instance());

    AddItem(&Motionblur::Instance());
    AddItem(&ClickEffect::Instance());

    AddItem(&TimeItem::Instance());
    AddItem(&FpsItem::Instance());
    AddItem(&DanmakuItem::Instance());
    AddItem(&KeystrokesItem::Instance());
    AddItem(&CPSItem::Instance());
    AddItem(&BilibiliFansItem::Instance());
    AddItem(&TextItem::Instance());
    AddItem(&FileCountItem::Instance());
    AddItem(&CounterItem::Instance());

    AddItem(&GlobalWindowStyle::Instance());
    AddItem(&GameStateDetector::Instance());
    AddItem(&CPSDetector::Instance());
}

// ------------------------------------------------
void ItemManager::AddItem(Item* item)
{
    Items.push_back(item);
}

// ------------------------------------------------
void ItemManager::UpdateAll() const
{
    for (auto item : Items)
    {
        if (!item->isEnabled) continue;
        if (auto upd = dynamic_cast<UpdateModule*>(item))
        {
            if (upd->ShouldUpdate())
            {
                upd->Update();
                upd->MarkUpdated();
            }
        }
    }
}

// ------------------------------------------------
void ItemManager::RenderAll() const
{
    bool isWindowNeedHide = false;
    if (GameStateDetector::Instance().IsNeedHide())
        isWindowNeedHide = true; // 隐藏所有窗口
    for (auto item : Items)
    {
        if (!item->isEnabled) continue;
        if (auto ren = dynamic_cast<RenderModule*>(item))
        {
            if (auto motionblur = dynamic_cast<Motionblur*>(ren))
                if (motionblur->applayOnMenu && Menu::Instance().isEnabled) continue;

            if (auto win = dynamic_cast<WindowModule*>(ren) && isWindowNeedHide)
                continue;

            ren->Render();
        }
    }
}

// ------------------------------------------------
void ItemManager::ProcessKeyEvents(bool state, bool isRepeat, WPARAM key) const
{
    for (auto item : Items)
    {
        if (auto kbd = dynamic_cast<KeybindModule*>(item))
        {
            if (auto menu = dynamic_cast<Menu*>(kbd))
                menu->OnKeyEvent(state, isRepeat, key);
            if (!item->isEnabled) continue;
            kbd->OnKeyEvent(state, isRepeat, key);
        }
    }
}

// ------------------------------------------------
// JSON Load / Save
// ------------------------------------------------
void ItemManager::Load(const nlohmann::json& j)
{
    // ---- 加载Item ----
    if (j.contains("Items"))
    {
        for (auto& node : j["Items"])
        {
            std::string type = node["type"];
            for (auto item : Items)
            {
                if (item->name == type)
                {
                    item->Load(node);
                    break;
                }
            }
        }
    }
}

// ------------------------------------------------
void ItemManager::Save(nlohmann::json& j) const
{
    j["Items"] = nlohmann::json::array();
    for (auto item : Items)
    {
        nlohmann::json node;
        item->Save(node);
        j["Items"].push_back(node);
    }

}

void ItemManager::Clear(bool resetSingletons) const
{
    // ---- 重置所有 Items ----
    if (resetSingletons)
    {
        for (auto* item : Items)
        {
            item->Reset();   //  要求 Item 提供 Reset() 或默认状态
        }
    }
}
