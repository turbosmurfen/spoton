/* Play/Pause, Next, Previous

Copyright 2010 Marcus L�nnberg

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

Copyright 2023 Turbosmurfen

Licensed under the Creative Commons License, version 4.0
https://creativecommons.org/licenses/by/4.0/

*/


//#include <cstdio>
#include <windows.h>
#include <WinUser.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <tlhelp32.h>
#include <tchar.h>
#define WIN32_LEAN_AND_MEAN
#pragma comment(linker, "/EXPORT:song=_song@24")
#pragma comment(linker, "/EXPORT:artist=_artist@24")
#pragma comment(linker, "/EXPORT:title=_title@24")
#pragma comment(linker, "/EXPORT:creator=_creator@24")
#pragma comment(linker, "/EXPORT:version=_version@24")
#pragma comment(linker, "/EXPORT:status=_status@24")
#pragma comment(linker, "/EXPORT:control=_control@24")
using namespace std;
string text;
vector<DWORD> pids;
int status_ = 0;
HWND hWNd;

char version_[] = "1.1.9";
char createdby[] = "Created by: Turbosmurfen";

//Media Control
enum MediaControl {
	Play = 3014656, //APPCOMMAND_MEDIA_PLAY * 0x10000
	Pause = 3080192, //APPCOMMAND_MEDIA_PAUSE * 0x10000 
	PlayPause = 917504, //APPCOMMAND_MEDIA_PLAY_PAUSE * 0x10000
	NextTrack = 720896, //APPCOMMAND_MEDIA_NEXTTRACK * 0x10000
	PreviousTrack = 786432, //APPCOMMAND_MEDIA_PREVIOUSTRACK * 0x10000
	RewindTrack = 3276800, //APPCOMMAND_MEDIA_REWIND * 0x10000
	ForwardTrack = 3211264, //APPCOMMAND_MEDIA_FAST_FORWARD * 0x10000
};


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
			replace(str.begin(), str.end(), '\n', ' ');
			replace(str.begin(), str.end(), '\r', ' ');
			text = str;
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
static void ReadData() {
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

//Writes out Artist - Title
extern "C" int __stdcall song(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	ReadData();
	if (text.empty()) {
		return 0;
	}
	else {
		strcpy_s(data, text.size() + 1, text.c_str());
		return 3;
	}
}

//Only return the Artist if " - " exists.
extern "C" int __stdcall artist(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	ReadData();
	if (text.empty()) {
		return 0;
	}
	else {
		size_t findPos = text.find(" - ");
		if (findPos != std::string::npos) {
			string artist = text.substr(0, findPos);
			strcpy_s(data, artist.size() + 1, artist.c_str());
			return 3;
		}
		else {
			return 0;
		}
	}
}

//Only return the Title if " - " exists.
extern "C" int __stdcall title(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	ReadData();
	if (text.empty()) {
		return 0;
	}
	else {
		size_t findPos = text.find(" - ");
		if (findPos != std::string::npos) {
			string title = text.substr(findPos + 3);
			strcpy_s(data, title.size() + 1, title.c_str());
			return 3;
		}
		else {
			return 0;
		}
	}
}


//Media Controls
extern "C" int __stdcall control(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{

	string cmd(data);
	if (!cmd.empty()) {
		ReadData();

		//Play or pauses a track
		if (cmd == "playpause") {
			if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, MediaControl::PlayPause);
		}
		//Plays a paused track
		else if (cmd == "play") {
			if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, MediaControl::Play);
		}
		//Pause a track if playing
		else if (cmd == "pause") {
			if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, MediaControl::Pause);
		}
		//Playing next track
		else if (cmd == "next") {
			if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, MediaControl::NextTrack);
		}
		//Plays track from beginning or previous track
		else if (cmd == "previous") {
			if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, MediaControl::PreviousTrack);
		}
		//Plays the track forward
		else if (cmd == "forward") {
			if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, MediaControl::ForwardTrack);
		}
		//plays the track backward
		else if (cmd == "rewind") {
			if (hWNd) SendMessage(hWNd, WM_APPCOMMAND, 0, MediaControl::RewindTrack);
		}

		//Open up Spotify window
		else if (cmd == "show") {
			if (hWNd) ShowWindow(hWNd, 1);
		}
	}
	return 0;
}

//Sending out who made this project.
extern "C" int __stdcall creator(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	strcpy_s(data, strlen(createdby) + 1, createdby);
	return 3;
}

//Sending out which version of Spoton that you use.
extern "C" int __stdcall version(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	strcpy_s(data, strlen(version_) + 1, version_);
	return 3;
}

/* Sending out the numbers of status (0-3)
* 0 - Spotify is not running
* 1 - Spotify is paused
* 2 - Spotfy is playing Advertisement
* 3 - Spotify is playing a song
*/
extern "C" int __stdcall status(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	ReadData();
	strcpy_s(data, std::to_string(status_).size() + 1, std::to_string(status_).c_str());
	return 3;
}
