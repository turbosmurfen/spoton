/* Play/Pause, Next, Previous

Copyright 2010 Marcus Lönnberg

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
https://github.com/marcuslonnberg/G930-Spotify-Controller
*/

/* Everything Else

Copyright 2022 Turbosmurfen

Licensed under the Creative Commons License, version 4.0
https://creativecommons.org/licenses/by/4.0/

*/


//#include <cstdio>
#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <tlhelp32.h>
#include <tchar.h>
#define WIN32_LEAN_AND_MEAN
#pragma comment(linker, "/EXPORT:song=_song@24")
#pragma comment(linker, "/EXPORT:creator=_creator@24")
#pragma comment(linker, "/EXPORT:version=_version@24")
#pragma comment(linker, "/EXPORT:status=_status@24")
#pragma comment(linker, "/EXPORT:control=_control@24")
using namespace std;
string title{};
vector<DWORD> pids;
int status_{0};
HWND hWNd{};

char vers[] = "1.1.5";
char by[] = "Created by: Turbosmurfen";

void readData(wstring input, HWND hWnd) {

	if (!input.empty()) {
		const wchar_t* wchar_0 = input.c_str();
		if (wcscmp(wchar_0, L"Spotify Premium") == 0 || wcscmp(wchar_0, L"Spotify Free") == 0) {
			status_ = 1;
			hWNd = hWnd;
		}
		else if (wcscmp(wchar_0, L"Advertisement") == 0) {
			status_ = 2;
			hWNd = hWnd;
		}
		else {
			int count = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), input.length(), NULL, 0, NULL, NULL);
			string str(count, 0);
			WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, &str[0], count, NULL, NULL);
			std::replace(str.begin(), str.end(), '\n', ' ');
			std::replace(str.begin(), str.end(), '\r', ' ');
			title = str;
			status_ = 3;
			hWNd = hWnd;
		}
	}
	else {
		status_ = 0;
	}
}


static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
	int length = GetWindowTextLength(hWnd);
	DWORD proc;
	GetWindowThreadProcessId(hWnd, &proc);
	wchar_t wText[1024];
	GetWindowTextW(hWnd, wText, length + 1);
	char* buffer0 = new char[256];
	GetClassNameA(hWnd, buffer0, 256);
	string wClass = buffer0;

	if (IsWindowVisible(hWnd) && length != 0 && wClass == "Chrome_WidgetWin_0" && find(pids.begin(), pids.end(), proc) != pids.end()) {
		readData(wText, hWnd);
		delete[] buffer0;
		return FALSE;
	}
	else {
		delete[] buffer0;
		return TRUE;
	}
}

static void Run() {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(handle, &entry))
	{
		while (Process32Next(handle, &entry))
		{
			if (_tcsicmp(entry.szExeFile, _T("Spotify.exe")) == 0)
			{
				pids.push_back(entry.th32ProcessID);
			}
		}
	}
	CloseHandle(handle);
	if (pids.empty()) {
		status_ = 0;
	}
	else {
		EnumWindows(enumWindowCallback, NULL);
		pids.clear();
	}
}

//Writes out title of the song
extern "C" int __stdcall song(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	Run();
	if (title.empty()) {
		return 0;
	}
	else {
		strcpy_s(data, title.size() + 1, title.c_str());
		return 3;
	}
}


//Media Controls
extern "C" int __stdcall control(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	std::string cmd(data);
	if (cmd == "pause") {
		Run();
		if (hWNd && status_ == 3) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PLAY_PAUSE * 0x10000);
		}
	}
	else if (cmd == "play") {
		Run();
		if (hWNd && status_ == 1) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PLAY_PAUSE * 0x10000);
		}
	}
	else if (cmd == "next") {
		Run();
		if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_NEXTTRACK * 0x10000);
	}
	else if (cmd == "rplay") {
		Run();
		if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PREVIOUSTRACK * 0x10000);
	}
	else if (cmd == "prev") {
		Run();
		if (hWNd) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PREVIOUSTRACK * 0x10000);
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PREVIOUSTRACK * 0x10000);
		}
	}
	else if (cmd == "volup") {
		Run();
		if (hWNd) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_UP * 0x10000);
		}
	}
	else if (cmd == "voldown") {
		Run();
		if (hWNd) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_DOWN * 0x10000);
		}
	}
	else if (cmd == "volmute") {
		Run();
		if (hWNd) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_MUTE * 0x10000);
		}
	}
	return 0;
}

//Created By
extern "C" int __stdcall creator(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	strcpy_s(data, strlen(by) + 1, by);
	return 3;
}

//Current Version
extern "C" int __stdcall version(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	strcpy_s(data, strlen(vers) + 1, vers);
	return 3;
}

/* Check for status
* 0 - Spotify is not running
* 1 - Spotify is paused
* 2 - Spotfy is playing Advertisement
* 3 - Spotify is playing a song
*/
extern "C" int __stdcall status(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	Run();
	strcpy_s(data, std::to_string(status_).size() + 1, std::to_string(status_).c_str());
	return 3;
}