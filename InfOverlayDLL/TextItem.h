#include "Item.h"
#include "AffixModule.h"
#include "WindowModule.h"
#include "nlohmann/json.hpp"
#include <string>

class TextItem : public Item, public AffixModule, public WindowModule
{
public:
    TextItem() {
        type = Hud; // 信息项类型
        multiType = MultiInstance;    // 信息项是否可以多开
        name = u8"文本显示";
        description = u8"显示一段文本";
        text = u8"请输入文本";
    }

    void Toggle() override;
    void DrawContent() override;
    void DrawSettings() override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;
private:
    std::string text;
};