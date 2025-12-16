#include "Item.h"
#include "UpdateModule.h"
#include "WindowModule.h"
#include <vector>
#include <string>
#include <deque>
#include <mutex>

enum bottonMessageType {
    BTM_ENTRY,
    BTM_GIFT,
    BTM_LIKE,
    BTM_CAPTAIN
};

typedef int danmaku_id;

struct Danmaku {
    std::string username;
    std::string message;
    danmaku_id id;
};

struct danmaku_element {
    ImVec4 color;
    float curHeight = 0;
    float tarHeight = 0;
};



class DanmakuItem : public Item, public UpdateModule, public WindowModule {
public:
    DanmakuItem()
    {
        type = Hud; // 信息项类型
        name = u8"B站弹幕显示";
        description = u8"显示B站直播间的弹幕(需配合B站弹幕姬)";
        icon = u8"\uE021";
        updateIntervalMs = 50;
        lastUpdateTime = std::chrono::steady_clock::now();
        Reset();
    }

    virtual ~DanmakuItem();

    static DanmakuItem& Instance() {
        static DanmakuItem instance;
        return instance;
    }

    void Toggle() override;
    void Reset() override
    {
        ResetWindow();
        isCustomSize = true;
        isEnabled = false;
        maxDanmakuCount = 16;
        logPath = "lastrun.txt";
    }
    void Update() override;
    void DrawContent() override;
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;

private:

    // 更新弹幕内容
    void AddDanmaku(const std::string& username, const std::string& message);
    void AddCaptain(const std::string& username, const std::string& captainName, const std::string& captainCount);
    void AddGift(const std::string& username, const std::string& giftName, const std::string& giftCount);
    void AddUserEntry(const std::string& username);
    void AddUserLike(const std::string& username);


    // 弹幕内容
    std::deque<Danmaku> danmakuList;

    std::string bottomMessage = "";
    bottonMessageType bottomMessageType = BTM_ENTRY;

    danmaku_id id = 0;
    std::map<danmaku_id, danmaku_element> anim;
    //std::mutex danmakuMutex;
    FILETIME lastWriteTime = {};
    bool isScrollable = false;

    // 最大弹幕数量
    int maxDanmakuCount = 16;

    //弹幕日志位置
    std::string logPath = "lastrun.txt";



};