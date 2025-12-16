#pragma once
#include "Item.h"
#include "UpdateModule.h"
#include <Windows.h>

class GameStateDetector : public UpdateModule, public Item{
public:


    GameStateDetector() {
        type = Hidden; // 信息项类型
        name = u8"游戏状态检测";
        description = u8"检测游戏当前状态";
        icon = u8"\uE039";
        updateIntervalMs = 10;
        lastUpdateTime = std::chrono::steady_clock::now();
        Reset();
    }
    ~GameStateDetector() {}

    static GameStateDetector& Instance()
    {
        static GameStateDetector instance;
        return instance;
    }

    void Toggle() override;
    void Reset() override
    {
        isEnabled = true;
        bool hideItemInGui = true;
    }
    void Update() override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) override;

    //bool IsPaused() const;          // ESC菜单
    //bool IsInventoryOpen() const;   // 背包
    //bool IsChatTyping() const;      // 聊天输入
    //bool IsInGUI() const;           // 任意GUI
    bool IsInGame() const;          // 在游戏世界中
    bool IsNeedHide() const;        // 是否需要隐藏信息项;
    GameState GetCurrentState() const;
    void ProcessMouseMovement(int dx, int dy);
    bool IsCameraMoving() const;
    float GetCameraSpeed() const;
private:
    //bool DetWindowCenter() const;
    static bool IsMouseCursorVisible();

    bool hideItemInGui = true;
    //GameState currentState = GameState::Paused;
    GameState currentState = InGameMenu;
    int centerLevel = 1;

    float movementThreshold = 1.0f; // 小于这个视为静止，用于防抖动
    float cameraSpeed;    // 鼠标移动速度
    bool cameraMoving;
};