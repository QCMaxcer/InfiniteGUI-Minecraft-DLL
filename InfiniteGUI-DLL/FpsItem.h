#include "Item.h"
#include "AffixModule.h"
#include "UpdateModule.h"
#include "WindowModule.h"

struct fps_element {
    ImVec4 color;
};

class FpsItem : public Item, public AffixModule, public UpdateModule, public WindowModule
{
public:
    FpsItem() {
        type = Hud; // 信息项类型
        name = u8"帧率显示";
        description = u8"显示当前帧率";
        icon = u8"\uE022";
        updateIntervalMs = 1000;
        lastUpdateTime = std::chrono::steady_clock::now();
        Reset();
    }

    static FpsItem& Instance() {
        static FpsItem instance;
        return instance;
    }

    void Toggle() override;
    void Reset() override
    {
        ResetAffix();
        ResetWindow();
        isEnabled = false;
        prefix = "[";
        suffix = "FPS]";
    }
    void Update() override;
    void DrawContent() override;
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;

private:
    // 帧率计数器
    int frameCount = 0;
    float FPS = 0.0f;
    fps_element color;
};