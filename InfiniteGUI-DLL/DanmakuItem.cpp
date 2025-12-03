#include "DanmakuItem.h"
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "ConfigManager.h"
#include "StringConverter.h"
#include "ImGuiStd.h"

#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <unordered_set>
#include <windows.h>
DanmakuItem::~DanmakuItem()
{
}

std::unordered_set<danmaku_id> pendingErase;
void DanmakuItem::AddDanmaku(const std::string& username, const std::string& message)
{
    Danmaku danmaku;
    danmaku.username = username;
    danmaku.message = message;
    danmaku.id = id++;
    // 如果弹幕数量超过最大值，删除最早的一条
    while (danmakuList.size() >= maxDanmakuCount) {
        pendingErase.insert(danmakuList.back().id);
        danmakuList.pop_back();
    }
    danmakuList.push_front(danmaku);
}

void DanmakuItem::AddCaptain(const std::string& username, const std::string& captainName, const std::string& captainCount)
{
    //std::lock_guard<std::mutex> lock(danmakuMutex);
    std::string giftMessage = username + u8" 开通了 " + captainName + " x " + captainCount;

    bottomMessage = giftMessage;
    bottomMessageType = BTM_CAPTAIN;

}

void DanmakuItem::AddGift(const std::string& username, const std::string& giftName, const std::string& giftCount)
{
    //std::lock_guard<std::mutex> lock(danmakuMutex);
    std::string giftMessage = username + u8" 赠送了 " + giftName + " x " + giftCount;

    bottomMessage = giftMessage;
    bottomMessageType = BTM_GIFT;
}

void DanmakuItem::AddUserEntry(const std::string& username)
{
    //std::lock_guard<std::mutex> lock(danmakuMutex);
    bottomMessage = username + u8" 进入了直播间";
    bottomMessageType = BTM_ENTRY;
}

void DanmakuItem::AddUserLike(const std::string& username)
{
    //std::lock_guard<std::mutex> lock(danmakuMutex);
    bottomMessage = username + u8" 点赞了";
    bottomMessageType = BTM_LIKE;
}

namespace fs = std::filesystem;

std::string ReadLastLine(std::wstring & filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary); // 从末尾打开
    if (!file.is_open())
        return ""; // 打开失败

    std::streamoff fileSize = file.tellg();
    if (fileSize <= 0)
        return "";

    std::string line;
    line.reserve(256); // 预分配内存，避免频繁扩容

    char ch;
    bool hasContent = false;

    // 从文件末尾往前找 '\n'
    for (std::streamoff i = fileSize - 1; i >= 0; --i)
    {
        file.seekg(i);
        file.get(ch);

        if (ch == '\n')
        {
            if (hasContent) break; // 已找到最后一行的起点
            else continue;         // 跳过文件末尾多余的换行
        }

        hasContent = true;
        line.insert(line.begin(), ch); // 从头插入字符
        if (i == 0) break;             // 到文件头了也要停
    }

    return line;
}


bool HasFileChanged(const std::wstring& filePath, FILETIME& lastWriteTime)
{
    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false; // 无法打开文件
    }

    FILETIME fileTime;
    if (GetFileTime(hFile, NULL, NULL, &fileTime)) {
        if (CompareFileTime(&fileTime, &lastWriteTime) != 0) {
            lastWriteTime = fileTime;  // 更新修改时间
            CloseHandle(hFile);
            return true;  // 文件有变化
        }
    }

    CloseHandle(hFile);
    return false; // 文件没有变化
}

void DanmakuItem::Toggle()
{
}

void DanmakuItem::Update()
{

    std::wstring Wstr_logpath = StringConverter::Utf8ToWstring(logPath);

    if(!HasFileChanged(Wstr_logpath, lastWriteTime)) return;

    std::string newLine = ReadLastLine(Wstr_logpath);

    if (!newLine.empty()) {
        // 解析日志内容
        // 18:12:30 : 收到彈幕:YunXiao7韵星 說: 各位点点举报
        // 如果第12~15个字符是 收到彈幕: 则认为是弹幕信息

        const static std::string gift_prefix = u8"收到道具:";
        const static std::string danmaku_prefix = u8"收到彈幕:";
        const static std::string danmaku_middle = u8" 說: ";
        const static std::string captain_prefix = u8"上船:";
        const static std::string captain_middle = u8" 購買了 ";
        size_t pos = newLine.find(danmaku_prefix);
        if (pos != std::string::npos) {
            std::string message = newLine.substr(pos + danmaku_prefix.length()); // 提取出 "YunXiao7韵星 說: 各位点点举报" 部分

            // 然后提取用户名
            size_t username_end = message.find(danmaku_middle);  // 查找 " 說:" 作为用户名的结束标志
            if (username_end == std::string::npos) return; // 或跳过
            std::string username = message.substr(0, username_end);

            // 提取弹幕内容
            std::string content = message.substr(username_end + danmaku_middle.length()); // 从 " 說:" 后面的内容开始

            AddDanmaku(username, content);
            return;
        }


        //18:31:45 : 收到道具:lyoffical 贈送的: 粉丝团灯牌 x 1
        // 如果第12~15个字符是 收到道具: 则认为是礼物信息 且不能找到收到彈幕
        pos = newLine.find(gift_prefix);
        if (pos != std::string::npos) {
            std::string message = newLine.substr(pos + gift_prefix.length()); // 提取出 "lyoffical 贈送的: 粉丝团灯牌 x 1" 部分

            // 提取用户名
            size_t username_end = message.find(u8" 贈送的:");
            if (username_end == std::string::npos) return; // 或跳过
            std::string username = message.substr(0, username_end);

            // 提取礼物名称
            size_t gift_start = message.find(u8"贈送的:") + 11; // 从 "贈送的:" 之后开始
            if (gift_start == std::string::npos) return;
            size_t gift_end = message.find(" x "); // 查找 " x" 的位置
            std::string gift_name = message.substr(gift_start, gift_end - gift_start);

            // 提取礼物数量
            size_t quantity_start = gift_end + 3; // 从 " x " 后面的位置开始
            if (quantity_start >= message.size()) return;
            std::string quantity = message.substr(quantity_start);

            AddGift(username, gift_name, quantity);  // 转换为整数
            return;
        }
        //18:36:45 : 上船:YunXiao7韵星 購買了 舰长 x 1
        pos = newLine.find(captain_prefix);
        if (pos != std::string::npos) {
            std::string message = newLine.substr(pos + captain_prefix.length()); // 提取出 "YunXiao7韵星 購買了 舰长 x 1" 部分

            // 然后提取用户名
            size_t username_end = message.find(captain_middle);  // 查找 " 購買了" 作为用户名的结束标志
            if (username_end == std::string::npos) return; // 或跳过
            std::string username = message.substr(0, username_end);

            // 提取礼物名称
            size_t caption_start = message.find(captain_middle) + captain_middle.length(); // 从 "購買了" 之后开始
            if (caption_start == std::string::npos) return;
            size_t caption_end = message.find(" x "); // 查找 " x" 的位置
            if (caption_end == std::string::npos) return;
            std::string caption_name = message.substr(caption_start, caption_end - caption_start);

            // 提取礼物数量
            size_t quantity_start = caption_end + 3; // 从 " x " 后面的位置开始
            std::string quantity = message.substr(quantity_start);

            AddCaptain(username, caption_name, quantity);  // 转换为整数
            return;
        }
        //18:25:44 : 没有qiqi 進入了直播間
        //如果倒数6个字符是 進入了直播間 则认为是进场信息
        //第12个字符以后到 " 進入了直播間"之前是用户名
        pos = newLine.find(u8"進入了直播間");
        if (pos != std::string::npos) {
            size_t start = newLine.find(" : ") + 3;
            size_t end = newLine.find(u8" 進入了直播間");
            if (start != std::string::npos && end != std::string::npos) {
                std::string username = newLine.substr(start, end - start);
                AddUserEntry(username);
            }
            return;
        }

        //18:25:48 : 罗总啊_a 點了讚
        //如果倒数3个字符是 點了讚 则认为是点赞信息
        //第12个字符以后到 " 點了讚"之前是用户名
        pos = newLine.find(u8"點了讚");
        if (pos != std::string::npos) {
            size_t start = newLine.find(" : ") + 3;
            size_t end = newLine.find(u8" 點了讚");
            if (start != std::string::npos && end != std::string::npos) {
                std::string username = newLine.substr(start, end - start);
                AddUserLike(username);
            }
            return;
        }
    }
}

void DanmakuItem::DrawContent()
{
    ImGui::PopFont(); // 把之前的字体弹出栈
    std::deque<Danmaku> copy_danmakuList;
    std::string copy_bottomMessage;

    {
        copy_danmakuList = danmakuList;
        copy_bottomMessage = bottomMessage;
    } //复制数据，避免崩溃

    //渲染层中定期清理
    for (auto id : pendingErase) {
        anim.erase(id);
    }
    pendingErase.clear();


    //设置背景透明度
    ImGui::PushStyleColor(ImGuiCol_ChildBg, *itemStylePtr.bgColor);

    ImGuiChildFlags child_flags = 0;
    if (!isScrollable) {
        child_flags |= ImGuiWindowFlags_NoScrollbar;
        child_flags |= ImGuiWindowFlags_NoScrollWithMouse;
    }

    // 绘制弹幕信息
    ImGui::BeginChild("DanmakuList", ImVec2(0, -*itemStylePtr.fontSize - 10), true, child_flags);

    if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
    {
        isScrollable = true;
    }
    else
    {
        isScrollable = false;
    } //检测是否滚动
    ImGui::PushTextWrapPos(0.0f); // 设置自动换行
    //获取io
    ImGuiIO& io = ImGui::GetIO();

    float heightSum = 0.0f;
    for (auto& it_anim : anim) {
        heightSum += it_anim.second.curHeight;
    }
    float curPosY = ImGui::GetWindowHeight() - heightSum; // 计算弹幕位置

    if (!isScrollable && !copy_danmakuList.empty())
    {
        ImGui::SetCursorPosY(curPosY); //设置弹幕位置
    }

    for (auto it = copy_danmakuList.rbegin(); it != copy_danmakuList.rend(); ++it) {
        auto& danmaku = *it;
        float textHeight = 0.0f;
        auto it_anim = anim.find(danmaku.id);
        if (it_anim == anim.end()) 
        {
            anim.insert({ danmaku.id,{ ImVec4(0.1f, 1.0f, 0.1f, 1.0f) } });
            it_anim = anim.find(danmaku.id);
        }

        float colorSpeed = io.DeltaTime * 3.0f;
        float scrollSpeed = io.DeltaTime * 5.0f;
        ImVec4 endColor = ImGui::ColorConvertU32ToFloat4( ImGui::GetColorU32(ImGuiCol_Text) );
        it_anim->second.color = ImLerp(it_anim->second.color, endColor, colorSpeed);                  //颜色动画

        ImGui::Separator();
        textHeight += ImGui::GetItemRectSize().y + ImGui::GetStyle().ItemSpacing.y;
        ImGui::SetCursorPosX(10);
        ImGui::PushFont(NULL, *itemStylePtr.fontSize * 0.8f);
        ImGuiStd::TextColoredShadow(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), (danmaku.username + " : ").c_str());
        textHeight += ImGui::GetItemRectSize().y + ImGui::GetStyle().ItemSpacing.y;
        ImGui::PopFont();
        ImGui::PushFont(NULL, *itemStylePtr.fontSize);
        ImGui::SetCursorPosX(10 + *itemStylePtr.fontSize);
        ImGuiStd::TextColoredShadow(it_anim->second.color, danmaku.message.c_str());
        textHeight += ImGui::GetItemRectSize().y + ImGui::GetStyle().ItemSpacing.y - 1.5f;
        ImGui::PopFont();
        it_anim->second.tarHeight = textHeight;
        if (it_anim->second.curHeight < it_anim->second.tarHeight)
            it_anim->second.curHeight = ImLerp(it_anim->second.curHeight, it_anim->second.tarHeight, scrollSpeed) + 1.0f; // 弹幕高度动画
    }
    ImGui::PopTextWrapPos();

    ImGui::EndChild();
    ImGui::PopStyleColor(); //弹出背景颜色

    ImGui::Separator();

    // 绘制底部信息

    ImGui::SetCursorPosX(10);
    ImGui::PushFont(NULL, *itemStylePtr.fontSize);
    switch (bottomMessageType) {
    case BTM_GIFT:
        ImGuiStd::TextColoredShadow(ImVec4(1.0f, 84.31f, 0.0f, 1.0f), bottomMessage.c_str()); //金色
        break;
    case BTM_ENTRY:
        ImGuiStd::TextColoredShadow(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), bottomMessage.c_str()); //灰色
        break;
    case BTM_LIKE:
        ImGuiStd::TextColoredShadow(ImVec4(0.1f, 1.0f, 0.1f, 1.0f), bottomMessage.c_str()); //绿色
        break;
    case BTM_CAPTAIN:
        ImGuiStd::TextColoredShadow(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), bottomMessage.c_str()); //红色
    default:
        break;
    }

}

void DanmakuItem::DrawSettings()
{
    DrawItemSettings();
    ImGuiStd::InputTextStd(u8"弹幕日志文件路径", logPath);
    ImGui::InputInt(u8"最大弹幕数", &maxDanmakuCount);
    if (ImGui::CollapsingHeader(u8"通用设置"))
    {
        DrawWindowSettings();
    }
}

void DanmakuItem::Load(const nlohmann::json& j)
{
    LoadItem(j);
    LoadWindow(j);
    if (j.contains("logPath"))  logPath = j["logPath"];
    if (j.contains("maxDanmakuCount")) maxDanmakuCount = j["maxDanmakuCount"];
}

void DanmakuItem::Save(nlohmann::json& j) const
{
    SaveItem(j);
    SaveWindow(j);
    j["logPath"] = logPath;
    j["maxDanmakuCount"] = maxDanmakuCount;

}