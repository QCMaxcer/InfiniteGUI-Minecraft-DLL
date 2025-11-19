#pragma once

#include <string>
#include <shlobj.h>
#include "StringConverter.h"
namespace FileManager {

    static std::string GetRunPath()
    {
        PWSTR path = nullptr;
        SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);

        char buffer[MAX_PATH];
        wcstombs(buffer, path, MAX_PATH);
        CoTaskMemFree(path);

        std::string p = std::string(buffer) + "\\InfOverlay";
        CreateDirectoryA(p.c_str(), NULL);

        return p;
    }

    static std::string GetConfigPath()
    {
        //如果没有Configs文件夹，则创建
        std::string p = GetRunPath() + "\\Configs";
        CreateDirectoryA(p.c_str(), NULL);
        return GetRunPath() + "\\Configs\\config.json";
    }

    static std::string GetSoundPath()
    {
        return GetRunPath() + "\\Sounds";
    }

    static std::string GetSoundPath(std::string soundName)
    {
        return GetRunPath() + "\\Sounds\\" + soundName;
    }


};