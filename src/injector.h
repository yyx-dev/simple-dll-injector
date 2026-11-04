#pragma once

#include <windows.h>
#include <string>

namespace inject {

inline bool RemoteThreadInject(DWORD pid, const std::string& dllPath) {
    // 1. 打开目标进程
    HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, pid
    );
    if (!hProcess) return false;

    // 2. 在目标进程中分配内存，大小包含结尾的 '\0'
    size_t pathSize = dllPath.size() + 1;
    LPVOID pRemoteMem = VirtualAllocEx(hProcess, NULL, pathSize,
                                       MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMem) {
        CloseHandle(hProcess);
        return false;
    }

    // 3. 将 DLL 路径写入目标进程内存
    if (!WriteProcessMemory(hProcess, pRemoteMem, dllPath.c_str(), pathSize, NULL)) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // 4. 获取 LoadLibraryA 的地址（在当前进程获取，地址通用）
    // 注意：如果 DLL 路径包含中文等宽字符，应使用 LoadLibraryW
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    LPTHREAD_START_ROUTINE pLoadLibrary =
        (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA");
    if (!pLoadLibrary) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // 5. 创建远程线程
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
                                        pLoadLibrary, pRemoteMem, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // 6. 等待线程结束并清理
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return true;
}

HHOOK g_hHook = NULL;

// 钩子回调函数：当目标线程发生消息事件时，系统会调用此函数
inline LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // 这里可以添加你的监控逻辑
    // 注意：必须调用 CallNextHookEx，否则会阻断消息传递，可能导致目标进程异常
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

// 导出函数：供注入器调用，安装指定类型的钩子到指定线程
extern "C" __declspec(dllexport) inline BOOL InstallHook(DWORD threadId, int hookType) {
    // hMod 传入当前 DLL 的模块句柄（通过 DllMain 保存，或直接用 GetModuleHandle）
    HMODULE hMod = NULL;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCTSTR)HookProc, &hMod);

    g_hHook = SetWindowsHookEx(hookType, HookProc, hMod, threadId);
    return (g_hHook != NULL);
}

// 导出函数：卸载钩子
extern "C" __declspec(dllexport) inline BOOL UninstallHook() {
    if (g_hHook) {
        BOOL ret = UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
        return ret;
    }
    return TRUE;
}

}
