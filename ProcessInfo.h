#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <tlhelp32.h>
#include <psapi.h>
#include <map>

struct ProcessInfo
{
    DWORD pid;
    std::wstring name;
    double cpuUsage;
    DWORDLONG memoryUsage;  // in bytes
};

class CProcessInfoHelper
{
private:
    static DWORDLONG GetProcessMemory(HANDLE hProcess)
    {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
        {
            return (DWORDLONG)pmc.WorkingSetSize;
        }
        return 0;
    }

public:
    static std::vector<ProcessInfo> GetAllProcesses()
    {
        std::vector<ProcessInfo> processes;

        static std::map<DWORD, ULARGE_INTEGER> lastKernelTime;
        static std::map<DWORD, ULARGE_INTEGER> lastUserTime;
        static ULARGE_INTEGER lastSystemTime = { 0 };
        static bool firstCall = true;

        FILETIME ftSystemTime;
        GetSystemTimeAsFileTime(&ftSystemTime);
        ULARGE_INTEGER currentSystemTime;
        currentSystemTime.LowPart = ftSystemTime.dwLowDateTime;
        currentSystemTime.HighPart = ftSystemTime.dwHighDateTime;

        ULONGLONG systemTimeDelta = 0;
        if (!firstCall)
        {
            systemTimeDelta = currentSystemTime.QuadPart - lastSystemTime.QuadPart;
        }
        lastSystemTime = currentSystemTime;

        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        DWORD numProcessors = sysInfo.dwNumberOfProcessors;

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE)
            return processes;

        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(pe32);

        if (Process32FirstW(hSnapshot, &pe32))
        {
            do
            {
                ProcessInfo info;
                info.pid = pe32.th32ProcessID;
                info.name = std::wstring(pe32.szExeFile, wcslen(pe32.szExeFile));
                info.cpuUsage = 0.0;
                info.memoryUsage = 0;

                // Try full access first, then limited
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.pid);
                if (!hProcess)
                    hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, info.pid);

                if (hProcess)
                {
                    info.memoryUsage = GetProcessMemory(hProcess);

                    FILETIME ftCreation, ftExit, ftKernel, ftUser;
                    if (GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser))
                    {
                        ULARGE_INTEGER kernelTime, userTime;
                        kernelTime.LowPart = ftKernel.dwLowDateTime;
                        kernelTime.HighPart = ftKernel.dwHighDateTime;
                        userTime.LowPart = ftUser.dwLowDateTime;
                        userTime.HighPart = ftUser.dwHighDateTime;

                        auto itKernel = lastKernelTime.find(info.pid);
                        auto itUser = lastUserTime.find(info.pid);

                        if (!firstCall && itKernel != lastKernelTime.end() && itUser != lastUserTime.end())
                        {
                            ULONGLONG kernelDelta = kernelTime.QuadPart - itKernel->second.QuadPart;
                            ULONGLONG userDelta = userTime.QuadPart - itUser->second.QuadPart;

                            if (systemTimeDelta > 0)
                            {
                                info.cpuUsage = ((double)(kernelDelta + userDelta) / (double)systemTimeDelta) * 100.0 / numProcessors;
                            }
                        }

                        lastKernelTime[info.pid] = kernelTime;
                        lastUserTime[info.pid] = userTime;
                    }

                    CloseHandle(hProcess);
                }

                processes.push_back(info);
            } while (Process32NextW(hSnapshot, &pe32));
        }

        CloseHandle(hSnapshot);
        firstCall = false;

        return processes;
    }

    static ProcessInfo GetTopCpuProcess()
    {
        static std::vector<ProcessInfo> lastProcesses;
        std::vector<ProcessInfo> currentProcesses = GetAllProcesses();

        std::map<std::wstring, double> cpuByProcess;
        for (const auto& proc : currentProcesses)
        {
            if (proc.pid == 0 || proc.pid == 4)
                continue;
            if (proc.name == L"Memory Compression")
                continue;
            cpuByProcess[proc.name] += proc.cpuUsage;
        }

        ProcessInfo topCpu;
        topCpu.cpuUsage = -1.0;
        for (const auto& pair : cpuByProcess)
        {
            if (pair.second > topCpu.cpuUsage)
            {
                topCpu.name = pair.first;
                topCpu.cpuUsage = pair.second;
                topCpu.pid = 0;
            }
        }

        lastProcesses = currentProcesses;
        return topCpu;
    }

    static ProcessInfo GetTopMemoryProcess(DWORDLONG* pTotalMemory = nullptr)
    {
        std::vector<ProcessInfo> processes = GetAllProcesses();

        std::map<std::wstring, DWORDLONG> memoryByProcess;
        DWORDLONG totalMemory = 0;
        for (const auto& proc : processes)
        {
            if (proc.pid == 0 || proc.pid == 4)
                continue;
            if (proc.name == L"Memory Compression")
                continue;
            memoryByProcess[proc.name] += proc.memoryUsage;
            totalMemory += proc.memoryUsage;
        }

        // Use physical memory in use as denominator (matches Task Manager)
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        if (pTotalMemory)
            *pTotalMemory = memInfo.ullTotalPhys - memInfo.ullAvailPhys;

        ProcessInfo topMem;
        topMem.memoryUsage = 0;
        for (const auto& pair : memoryByProcess)
        {
            if (pair.second > topMem.memoryUsage)
            {
                topMem.name = pair.first;
                topMem.memoryUsage = pair.second;
                topMem.pid = 0;
            }
        }

        return topMem;
    }

    static std::wstring FormatMemorySize(DWORDLONG bytes)
    {
        wchar_t buf[64];
        if (bytes >= 1024ULL * 1024 * 1024)
        {
            double gb = bytes / (1024.0 * 1024.0 * 1024.0);
            swprintf(buf, 64, L"%.1fGB", gb);
        }
        else if (bytes >= 1024ULL * 1024)
        {
            double mb = bytes / (1024.0 * 1024.0);
            swprintf(buf, 64, L"%.0fMB", mb);
        }
        else
        {
            double kb = bytes / 1024.0;
            swprintf(buf, 64, L"%.0fKB", kb);
        }
        return std::wstring(buf);
    }
};
