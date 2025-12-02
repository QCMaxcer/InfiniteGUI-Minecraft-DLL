#pragma once
class HttpModule
{
public:
    virtual void HttpAddTask() = 0;
    virtual void HttpRemoveTask() = 0; 
    void HttpResetTask()
    {
        HttpRemoveTask();
        HttpAddTask();
    }

protected:
    int httpUpdateIntervalMs;
    int httpTaskId;
};