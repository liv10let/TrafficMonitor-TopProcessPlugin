#pragma once

#include "PluginInterface.h"
#include "TopCpuProcessItem.h"
#include "TopMemoryProcessItem.h"

class CTopProcessPlugin : public ITMPlugin
{
private:
    CTopProcessPlugin();

public:
    static CTopProcessPlugin& Instance();

    // ITMPlugin interface
    virtual IPluginItem* GetItem(int index) override;
    virtual void DataRequired() override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual void OnInitialize(ITrafficMonitor* pApp) override;

private:
    CTopCpuProcessItem m_topCpuItem;
    CTopMemoryProcessItem m_topMemoryItem;
    ITrafficMonitor* m_app{};

    static CTopProcessPlugin m_instance;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();
#ifdef __cplusplus
}
#endif
