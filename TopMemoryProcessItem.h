#pragma once

#include "PluginInterface.h"
#include <string>

class CTopMemoryProcessItem : public IPluginItem
{
public:
    CTopMemoryProcessItem();

    // IPluginItem interface
    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;
    virtual int IsDrawResourceUsageGraph() const override;
    virtual float GetResourceUsageGraphValue() const override;

    // Data update
    void UpdateData(const std::wstring& processName, DWORDLONG memoryUsage, double memoryPercent);

private:
    std::wstring m_processName;
    DWORDLONG m_memoryUsage;
    double m_memoryPercent;
    std::wstring m_valueText;
};
