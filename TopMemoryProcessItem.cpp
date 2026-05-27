#include "pch.h"
#include "TopMemoryProcessItem.h"
#include "ProcessInfo.h"

CTopMemoryProcessItem::CTopMemoryProcessItem()
    : m_memoryUsage(0)
    , m_memoryPercent(0.0)
{
}

const wchar_t* CTopMemoryProcessItem::GetItemName() const
{
    return L"内存占用最高进程";
}

const wchar_t* CTopMemoryProcessItem::GetItemId() const
{
    return L"TpMemProc1";
}

const wchar_t* CTopMemoryProcessItem::GetItemLableText() const
{
    return L"";
}

const wchar_t* CTopMemoryProcessItem::GetItemValueText() const
{
    return m_valueText.c_str();
}

const wchar_t* CTopMemoryProcessItem::GetItemValueSampleText() const
{
    return L"chrome:8.5%";
}

int CTopMemoryProcessItem::IsDrawResourceUsageGraph() const
{
    return 1;
}

float CTopMemoryProcessItem::GetResourceUsageGraphValue() const
{
    return static_cast<float>(m_memoryPercent / 100.0);
}

void CTopMemoryProcessItem::UpdateData(const std::wstring& processName, DWORDLONG memoryUsage, double memoryPercent)
{
    m_processName = processName;
    m_memoryUsage = memoryUsage;
    m_memoryPercent = memoryPercent;

    if (processName.empty())
    {
        m_valueText = L"--";
        return;
    }

    // Remove .exe for display
    std::string name(processName.begin(), processName.end());
    size_t exePos = name.find(".exe");
    if (exePos != std::string::npos)
    {
        name = name.substr(0, exePos);
    }

    // Truncate if too long
    if (name.length() > 15)
    {
        name = name.substr(0, 12) + "...";
    }

    // Format result
    char buf[256];
    snprintf(buf, sizeof(buf), "%s:%.1f%%", name.c_str(), memoryPercent);
    m_valueText = std::wstring(buf, buf + strlen(buf));
}
