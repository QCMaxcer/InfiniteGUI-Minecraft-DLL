#include "Item.h"
#include "AffixModule.h"
#include "UpdateModule.h"
#include "WindowModule.h"

class FpsItem : public Item, public AffixModule, public UpdateModule, public WindowModule
{
public:
    FpsItem() {
        type = Hud; // 信息项类型
        multiType = Singleton;    // 信息项是否可以多开
        isEnabled = false;
        name = u8"帧率显示";
        description = u8"显示当前帧率";
        updateIntervalMs = 1000;
        lastUpdateTime = std::chrono::steady_clock::now();
        prefix = "[";
        suffix = "FPS]";
    }

    static FpsItem& Instance() {
        static FpsItem instance;
        return instance;
    }

    void Toggle() override;
    void Update() override;
    void DrawContent() override;
    void DrawSettings() override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;

private:
    // 帧率计数器
    int frameCount = 0;
    float FPS = 0.0f;
};