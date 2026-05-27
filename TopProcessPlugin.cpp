#include "pch.h"
#include "TopProcessPlugin.h"
#include "ProcessInfo.h"

CTopProcessPlugin CTopProcessPlugin::m_instance;

CTopProcessPlugin::CTopProcessPlugin()
{
}

CTopProcessPlugin& CTopProcessPlugin::Instance()
{
    return m_instance;
}

IPluginItem* CTopProcessPlugin::GetItem(int index)
{
    switch (index)
    {
    case 0:
        return &m_topCpuItem;
    case 1:
        return &m_topMemoryItem;
    default:
        return nullptr;
    }
}

void CTopProcessPlugin::DataRequired()
{
    // Get total physical memory for percentage calculation
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG totalPhysMem = memInfo.ullTotalPhys;

    // Get top CPU process
    ProcessInfo topCpu = CProcessInfoHelper::GetTopCpuProcess();
    m_topCpuItem.UpdateData(topCpu.name, topCpu.cpuUsage);

    // Get top memory process
    ProcessInfo topMem = CProcessInfoHelper::GetTopMemoryProcess();
    double memPercent = 0.0;
    if (totalPhysMem > 0)
    {
        memPercent = (double)topMem.memoryUsage / (double)totalPhysMem * 100.0;
    }
    m_topMemoryItem.UpdateData(topMem.name, topMem.memoryUsage, memPercent);
}

const wchar_t* CTopProcessPlugin::GetInfo(PluginInfoIndex index)
{
    switch (index)
    {
    case TMI_NAME:
        return L"Top Process Monitor";
    case TMI_DESCRIPTION:
        return L"Displays the process with highest CPU and memory usage";
    case TMI_AUTHOR:
        return L"TrafficMonitor User";
    case TMI_COPYRIGHT:
        return L"Copyright (C) 2026";
    case TMI_VERSION:
        return L"1.0.0";
    case TMI_URL:
        return L"";
    default:
        return L"";
    }
}

void CTopProcessPlugin::OnInitialize(ITrafficMonitor* pApp)
{
    m_app = pApp;
}

ITMPlugin* TMPluginGetInstance()
{
    return &CTopProcessPlugin::Instance();
}
