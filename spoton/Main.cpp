#include "pch.h"
#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <tchar.h>
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#pragma comment(linker, "/EXPORT:announce=_announce@24")
#pragma comment(linker, "/EXPORT:creator=_creator@24")
using namespace std;
vector<DWORD> pids;
string title{};
static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
    DWORD proc;
    GetWindowThreadProcessId(hWnd, &proc);
    int length = GetWindowTextLength(hWnd);
    char* buffer = new char[length + 1];
    GetWindowTextA(hWnd, buffer, length + 1);
    string windowTitle(buffer);

    if (length != 0 && find(pids.begin(), pids.end(), proc) != pids.end()) {
        if (IsWindowVisible(hWnd)) {
            title = windowTitle;

        }

    }
    return TRUE;
}
void readData() {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(handle, &entry) == TRUE)
    {
        while (Process32Next(handle, &entry) == TRUE)
        {
            if (_tcsicmp(entry.szExeFile, _T("Spotify.exe")) == 0)
            {
                pids.push_back(entry.th32ProcessID);
            }
        }
    }
    CloseHandle(handle);
    if (!pids.empty()) {
        EnumWindows(enumWindowCallback, NULL);
        pids.clear();
        if (title == "Spotify Premium") title = "1";
        else if (title.empty()) {
            title = "2";
        }
    }
    else {
        title = "0";
    }
}

extern "C" int __stdcall announce(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
    readData();
    char* cstr = new char[title.size() + 1];
    title.copy(cstr, title.size() + 1);
    cstr[title.size()] = '\0';
    strcpy_s(data, strlen(cstr) + 1, cstr);
    return 3;
}
extern "C" int __stdcall creator(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
    char cby[] = "Created By: Turbosmurfen - Site: Https://www.turbosmurfen.com - Contact: contact@turbosmurfen.com";
    strcpy_s(data, strlen(cby) + 1, cby);
    return 3;
}