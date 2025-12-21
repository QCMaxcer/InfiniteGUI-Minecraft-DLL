#include "ConfigManager.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "FileUtils.h"
#include "GlobalConfig.h"
#include "ItemManager.h"
#include "Menu.h"
#include "Notification.h"

namespace Paths {
    std::string root = FileUtils::configPath;
    std::string global = root + "\\global.json";
    std::string profiles = root + "\\profiles";
}


// ---------------------------------------------------
// Helper Functions
// ---------------------------------------------------
std::string ConfigManager::GetProfilePath(const std::string& name) const
{
    return Paths::profiles + "\\" + name + ".json";
}

void ConfigManager::RefreshProfileList()
{
    profiles.clear();
    for (auto& entry : std::filesystem::directory_iterator(Paths::profiles))
    {
        if (entry.is_regular_file())
        {
            auto name = entry.path().stem().string(); // filename w/o extension
            profiles.push_back(name);
        }
    }
}



// ---------------------------------------------------
// Init - Load existing configs or create profile
// ---------------------------------------------------
void ConfigManager::Init()
{
	Paths::root = FileUtils::configPath;
	Paths::global = Paths::root + "\\global.json";
    Paths::profiles = Paths::root + "\\profiles";
    std::filesystem::create_directories(Paths::profiles);

    RefreshProfileList();
    if (profiles.empty())
    {
        CreateProfile("profile");
    }
}



// ---------------------------------------------------
// Profile Management
// ---------------------------------------------------
bool ConfigManager::CreateProfile(const std::string& name)
{
    std::string path = GetProfilePath(name);
    if (std::filesystem::exists(path))
        return false;

    // Create empty profile
    nlohmann::json j;
    std::ofstream f(path);
    f << j.dump(4);

    RefreshProfileList();
    return true;
}

bool ConfigManager::DeleteProfile(const std::string& name)
{
    if (profiles.size() <= 1)
        return false; // Must keep at least one profile

    if (name == currentProfile)
        return false; // Cannot delete active profile

    std::filesystem::remove(GetProfilePath(name));
    RefreshProfileList();
    return true;
}

bool ConfigManager::RenameProfile(const std::string& oldName, const std::string& newName)
{
    if (oldName == newName) return true;

    std::string oldPath = GetProfilePath(oldName);
    std::string newPath = GetProfilePath(newName);

    if (!std::filesystem::exists(oldPath)) return false;
    if (std::filesystem::exists(newPath)) return false;

    std::filesystem::rename(oldPath, newPath);

    if (currentProfile == oldName)
        currentProfile = newName;

    RefreshProfileList();
    return true;
}



// ---------------------------------------------------
// Switch Config
// ---------------------------------------------------
bool ConfigManager::SwitchProfile(const std::string& name, bool saveCurrent)
{
    if (name == currentProfile)
        return true;

    if (saveCurrent)
        Save();

    ItemManager::Instance().Clear(true);
    currentProfile = name;
    SaveGlobal(); // Save global config to reflect new profile
    return LoadProfile();
}



// ---------------------------------------------------
// Save / Load
// ---------------------------------------------------
bool ConfigManager::Save()
{
    std::filesystem::create_directories(Paths::root);
    std::filesystem::create_directories(Paths::profiles);

    // ---- Save Global ----
    {
        SaveGlobal();
    }

    // ---- Save Items ----
    {
        SaveProfile();
    }

    return true;
}


bool ConfigManager::Load()
{
    // ---- Load Global config ----
    LoadGlobal();
    LoadProfile();
    return true;
}

bool ConfigManager::SaveGlobal() const
{
    // ---- Save Global config ----
    GlobalConfig::Instance().currentProfile = currentProfile;
    nlohmann::json j;
    GlobalConfig::Instance().Save(j);
    std::ofstream f(Paths::global);
    if (!f.is_open()) return false;
    f << j.dump(4);
    return true;
}

bool ConfigManager::LoadGlobal()
{
    // ---- Load Global config ----
    if (std::filesystem::exists(Paths::global))
    {
        std::ifstream f(Paths::global);
        nlohmann::json j;
        f >> j;
        GlobalConfig::Instance().Load(j);
        currentProfile = GlobalConfig::Instance().currentProfile;
        return true;
    }
    return false;
}

bool ConfigManager::SaveProfile() const
{
    // ---- Save Item Profile ----
    nlohmann::json j;
    ItemManager::Instance().Save(j);

    std::ofstream f(GetProfilePath(currentProfile));
    if (!f.is_open()) return false;
    f << j.dump(4);
    return true;
}

bool ConfigManager::LoadProfile()
{
    // ---- Load Item Profile ----
    std::string profilePath = GetProfilePath(currentProfile);

    if (!std::filesystem::exists(profilePath))
    {
        SaveProfile();
        RefreshProfileList();
        return true;
    }

    std::ifstream f(profilePath);
    if (!f.is_open()) return false;

    nlohmann::json j;
    f >> j;
    ItemManager::Instance().Load(j);
    int key = Menu::Instance().GetKeyBind();
    std::string hotkeyStr = keys[key];
    std::string notify = u8"无限Gui启动成功！\n按\"" + hotkeyStr + u8"\"键打开菜单。";
    NotificationItem::Instance().AddNotification(NotificationType_Info, notify);
    ClickSound::PlayIntroSound();
    return true;
}
