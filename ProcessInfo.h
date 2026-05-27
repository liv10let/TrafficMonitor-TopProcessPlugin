#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <tlhelp32.h>
#include <psapi.h>

struct ProcessInfo
{
    DWORD pid;
    std::wstring name;
    double cpuUsage;
    DWORDLONG memoryUsage;  // in bytes
};

class CProcessInfoHelper
{
public:
    static std::vector<ProcessInfo> GetAllProcesses()
    {
        std::vector<ProcessInfo> processes;

        // Get initial CPU times for all processes
        static std::map<DWORD, ULARGE_INTEGER> lastKernelTime;
        static std::map<DWORD, ULARGE_INTEGER> lastUserTime;
        static ULARGE_INTEGER lastSystemTime = { 0 };
        static bool firstCall = true;

        // Get current system time
        FILETIME ftSystemTime;
        GetSystemTimeAsFileTime(&ftSystemTime);
        ULARGE_INTEGER currentSystemTime;
        currentSystemTime.LowPart = ftSystemTime.dwLowDateTime;
        currentSystemTime.HighPart = ftSystemTime.dwHighDateTime;

        // Calculate system time delta
        ULONGLONG systemTimeDelta = 0;
        if (!firstCall)
        {
            systemTimeDelta = currentSystemTime.QuadPart - lastSystemTime.QuadPart;
        }
        lastSystemTime = currentSystemTime;

        // Get number of processors for CPU usage calculation
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
                // Explicitly copy process name using wcslen to ensure correct length
                info.name = std::wstring(pe32.szExeFile, wcslen(pe32.szExeFile));
                info.cpuUsage = 0.0;
                info.memoryUsage = 0;

                // Get process handle
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.pid);
                if (hProcess)
                {
                    // Get memory usage
                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
                    {
                        info.memoryUsage = pmc.WorkingSetSize;
                    }

                    // Get CPU usage
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
                                // CPU usage = (kernel + user time) / (system time * num processors)
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

        ProcessInfo topCpu;
        topCpu.cpuUsage = -1.0;

        for (const auto& proc : currentProcesses)
        {
            // Skip system idle process and system process
            if (proc.pid == 0 || proc.pid == 4)
                continue;

            if (proc.cpuUsage > topCpu.cpuUsage)
            {
                topCpu = proc;
            }
        }

        lastProcesses = currentProcesses;
        return topCpu;
    }

    static ProcessInfo GetTopMemoryProcess()
    {
        std::vector<ProcessInfo> processes = GetAllProcesses();

        ProcessInfo topMem;
        topMem.memoryUsage = 0;

        for (const auto& proc : processes)
        {
            // Skip system idle process and system process
            if (proc.pid == 0 || proc.pid == 4)
                continue;

            if (proc.memoryUsage > topMem.memoryUsage)
            {
                topMem = proc;
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
