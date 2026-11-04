#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>

namespace utils {

struct ProcessInfo {
    DWORD pid;
    std::wstring name;
};

inline std::vector<ProcessInfo> GetAllProcesses() {
    std::vector<ProcessInfo> list;
    PROCESSENTRY32W pe32{};
    pe32.dwSize = sizeof(pe32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return list;

    if (Process32FirstW(hSnap, &pe32)) {
        do {
            ProcessInfo info;
            info.pid         = pe32.th32ProcessID;
            info.name        = pe32.szExeFile;
            list.push_back(std::move(info));
        } while (Process32NextW(hSnap, &pe32));
    }
    CloseHandle(hSnap);
    return list;
}

inline bool IsProcess32Bits(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        // 打开失败，可能是权限不足或进程不存在
        return false;
    }

    USHORT processMachine = 0;
    USHORT nativeMachine = 0;

    if (IsWow64Process2(hProcess, &processMachine, &nativeMachine)) {
        // 如果processMachine不是UNKNOWN，说明进程运行在WOW64下，即32位
        return processMachine != IMAGE_FILE_MACHINE_UNKNOWN;
    } else {
        // 调用失败，需要根据GetLastError()处理
    }

    CloseHandle(hProcess);
    return false;
}

}