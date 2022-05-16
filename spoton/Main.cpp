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
#define WIN32_LEAN_AND_MEAN
#pragma comment(linker, "/EXPORT:song=_song@24")
#pragma comment(linker, "/EXPORT:creator=_creator@24")
#pragma comment(linker, "/EXPORT:version=_version@24")
#pragma comment(linker, "/EXPORT:status=_status@24")
#pragma comment(linker, "/EXPORT:control=_control@24")
using namespace std;
string title{};
string lastTitle{};
int status_{ 0 };
int statusLast{ 0 };
HWND hWNd{};
HWND lasthWNd{};

char vers[] = "1.1.4";
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
			std::replace(str.begin(), str.end(), '\n', '\0');
			std::replace(str.begin(), str.end(), '\r', '\0');
			title = str;
			status_ = 3;
			hWNd = hWnd;
		}
	}
	else {
		status_ = 0; //This is never going to execute (only if Spotify detection get updated)
	}
}

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
	string wClass{};
	int length = GetWindowTextLength(hWnd);
	wchar_t wText[1024];
	GetWindowTextW(hWnd, wText, length + 1);
	char* buffer0 = new char[256];
	GetClassNameA(hWnd, buffer0, 256);
	wClass = buffer0;
	

	if (IsWindowVisible(hWnd) && length != 0 && wClass == "Chrome_WidgetWin_0") {
		readData(wText, hWnd);
	}

	return TRUE;
}


//Writes out title of the song
extern "C" int __stdcall song(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	if (title != lastTitle || title.empty()) {
		EnumWindows(enumWindowCallback, NULL);
		lastTitle = title;
	}
	char* cstr = new char[title.size() + 1];
	title.copy(cstr, title.size() + 1);
	cstr[title.size()] = '\0';
	strcpy_s(data, strlen(cstr) + 1, cstr);

	return 3;
}



extern "C" int __stdcall control(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	std::string cmd(data);
	if (cmd == "pause") {
		EnumWindows(enumWindowCallback, NULL);
		if (hWNd && status_ == 3) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PLAY_PAUSE * 0x10000);
		}
	}
	else if (cmd == "play") {
		EnumWindows(enumWindowCallback, NULL);
		if (hWNd && status_ == 1) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PLAY_PAUSE * 0x10000);
		}
	}
	else if (cmd == "next") {
		if (hWNd != lasthWNd || !hWNd) {
			EnumWindows(enumWindowCallback, NULL);
			lasthWNd = hWNd;
		}
		if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_NEXTTRACK * 0x10000);
	}
	else if (cmd == "rplay") {
		if (hWNd != lasthWNd || !hWNd) {
			EnumWindows(enumWindowCallback, NULL);
			lasthWNd = hWNd;
		}
		if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PREVIOUSTRACK * 0x10000);
	}
	else if (cmd == "prev") {
		if (hWNd != lasthWNd || !hWNd) {
			EnumWindows(enumWindowCallback, NULL);
			lasthWNd = hWNd;
		}
		if (hWNd) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PREVIOUSTRACK * 0x10000);
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_MEDIA_PREVIOUSTRACK * 0x10000);
		}
	}
	else if (cmd == "volup") {
		if (hWNd != lasthWNd || !hWNd) {
			EnumWindows(enumWindowCallback, NULL);
			lasthWNd = hWNd;
		}
		if (hWNd) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_UP * 0x10000);
		}
	}
	else if (cmd == "voldown") {
		if (hWNd != lasthWNd || !hWNd) {
			EnumWindows(enumWindowCallback, NULL);
			lasthWNd = hWNd;
		}
		if (hWNd) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_DOWN * 0x10000);
		}
	}
	else if (cmd == "volmute") {
		if (hWNd != lasthWNd || !hWNd) {
			EnumWindows(enumWindowCallback, NULL);
			lasthWNd = hWNd;
		}
		if (hWNd) {
			SendMessage(hWNd, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_MUTE * 0x10000);
		}
	}
	return 0;
}

extern "C" int __stdcall creator(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	strcpy_s(data, strlen(by) + 1, by);
	return 3;
}

extern "C" int __stdcall version(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	strcpy_s(data, strlen(vers) + 1, vers);
	return 3;
}

/* Check for status
* 1 - Spotify is paused
* 2 - Spotfy is playing Advertisement
* 3 - Spotify is playing (detect this for not running)
*/
extern "C" int __stdcall status(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	if (status_ != statusLast || !status_) {
		EnumWindows(enumWindowCallback, NULL);
		statusLast = status_;
	}
	string input = std::to_string(status_);
	char* cstr = new char[input.size() + 1];
	input.copy(cstr, input.size() + 1);
	cstr[input.size()] = '\0';
	strcpy_s(data, strlen(cstr) + 1, cstr);
	return 3;
}