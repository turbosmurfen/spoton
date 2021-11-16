#include <cstdio>
#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#pragma comment(linker, "/EXPORT:announce=_announce@24")
#pragma comment(linker, "/EXPORT:creator=_creator@24")
#pragma comment(linker, "/EXPORT:pause=_pause@24")
#pragma comment(linker, "/EXPORT:next=_next@24")
#pragma comment(linker, "/EXPORT:prev=_prev@24")
using namespace std;
string title{"0"};
HWND hWNd{};
static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
    string wClass{};
    int length = GetWindowTextLength(hWnd);
    char* buffer = new char[length + 1];
    GetWindowTextA(hWnd, buffer, length + 1);
    string wTitle(buffer);
    char* buffer0 = new char[256];
    GetClassNameA(hWnd, buffer0, 256);
    wClass = buffer0;

    if (IsWindowVisible(hWnd) && length != 0 && wClass == "Chrome_WidgetWin_0") {
        title = wTitle;
        hWNd = hWnd;
    }

    return TRUE;
}

extern "C" int __stdcall announce(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
    EnumWindows(enumWindowCallback, NULL);
    string output{ "0" };
    if (title == "Advertisement" || title.find("-") == 0) {
        output = "2";
    }
    else if (!title.empty()) {

        if (title == "Spotify Premium" || title == "Spotify Free") {
            output = "1";
        }
        else {
            output = title;
        }
    }
    else {
        output = "0";
    }
    
    char* cstr = new char[output.size() + 1];
    output.copy(cstr, output.size() + 1);
    cstr[output.size()] = '\0';
    strcpy_s(data, strlen(cstr) + 1, cstr);
    return 3;
}
extern "C" int __stdcall creator(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
    char cby[] = "Created By: Turbosmurfen";
    strcpy_s(data, strlen(cby) + 1, cby);
    return 3;
}

extern "C" int __stdcall pause(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
    EnumWindows(enumWindowCallback, NULL);
    if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PLAY_PAUSE * 0x10000);
    return 2;
}

extern "C" int __stdcall next(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
    EnumWindows(enumWindowCallback, NULL);
    if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_NEXTTRACK * 0x10000);
    return 2;
}

extern "C" int __stdcall prev(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
    EnumWindows(enumWindowCallback, NULL);
    if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PREVIOUSTRACK * 0x10000);
    return 2;
}