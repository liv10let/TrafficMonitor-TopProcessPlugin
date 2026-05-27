#include "pch.h"
#include "TopCpuProcessItem.h"

CTopCpuProcessItem::CTopCpuProcessItem()
    : m_cpuUsage(0.0)
{
}

const wchar_t* CTopCpuProcessItem::GetItemName() const
{
    return L"CPU占用最高进程";
}

const wchar_t* CTopCpuProcessItem::GetItemId() const
{
    return L"TpCpuProc1";
}

const wchar_t* CTopCpuProcessItem::GetItemLableText() const
{
    return L"";
}

const wchar_t* CTopCpuProcessItem::GetItemValueText() const
{
    return m_valueText.c_str();
}

const wchar_t* CTopCpuProcessItem::GetItemValueSampleText() const
{
    return L"chrome:99.9%";
}

int CTopCpuProcessItem::IsDrawResourceUsageGraph() const
{
    return 1;
}

float CTopCpuProcessItem::GetResourceUsageGraphValue() const
{
    return static_cast<float>(m_cpuUsage / 100.0);
}

void CTopCpuProcessItem::UpdateData(const std::wstring& processName, double cpuUsage)
{
    m_processName = processName;
    m_cpuUsage = cpuUsage;

    if (processName.empty())
    {
        m_valueText = L"--";
        return;
    }

    // Convert to string and remove .exe
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
    snprintf(buf, sizeof(buf), "%s:%.1f%%", name.c_str(), cpuUsage);
    m_valueText = std::wstring(buf, buf + strlen(buf));
}
