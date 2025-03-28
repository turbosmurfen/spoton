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

Copyright 2025 Turbosmurfen

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
vector<DWORD> pids;

char version_[] = "1.2.1";
char createdBy[] = "Created by: Turbosmurfen";

struct Info {
	int status = 0;
	string text = "";
	HWND windowHwnd = nullptr;
};
Info infoData;

enum MediaControl {
	Play = 3014656, //APPCOMMAND_MEDIA_PLAY * 0x10000
	Pause = 3080192, //APPCOMMAND_MEDIA_PAUSE * 0x10000 
	PlayPause = 917504, //APPCOMMAND_MEDIA_PLAY_PAUSE * 0x10000
	NextTrack = 720896, //APPCOMMAND_MEDIA_NEXTTRACK * 0x10000
	PreviousTrack = 786432, //APPCOMMAND_MEDIA_PREVIOUSTRACK * 0x10000
	RewindTrack = 3276800, //APPCOMMAND_MEDIA_REWIND * 0x10000
	ForwardTrack = 3211264, //APPCOMMAND_MEDIA_FAST_FORWARD * 0x10000
};

void ConvertData(wstring input, HWND hWnd) {

	//If input is not empty (Spotify met the conditions below)
	if (!input.empty()) {
		const wchar_t* wchar_0 = input.c_str();

		//if these conditions are met, Spotify is paused.
		if (wcscmp(wchar_0, L"Spotify Premium") == 0 || wcscmp(wchar_0, L"Spotify Free") == 0) {
			infoData.status = 1;
			infoData.windowHwnd = hWnd;
			infoData.text = "";
		}
		//if this condition is met, Spotify is playing an advertisement
		else if (wcscmp(wchar_0, L"Advertisement") == 0) {
			infoData.status = 2;
			infoData.windowHwnd = hWnd;
			infoData.text = "";
		}
		//If not the conditions before is met, Spotify is playing a song
		else {

			//Convert to UTF-8.
			int bufferSize = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, nullptr, 0, nullptr, nullptr);
			if (bufferSize != 0) {
				vector<char> multiByteStr(bufferSize);
				WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, multiByteStr.data(), bufferSize, nullptr, nullptr);

				//Sanitise the text
				replace(multiByteStr.begin(), multiByteStr.end(), '\r', ' ');
				replace(multiByteStr.begin(), multiByteStr.end(), '\n', ' ');

				//Convert UTF8 to string
				string stringOut = string(multiByteStr.data());

				infoData.status = 3;
				infoData.windowHwnd = hWnd;
				infoData.text = stringOut;

				multiByteStr.clear();
			}
			else {
				infoData.status = 4;
				infoData.windowHwnd = nullptr;
				infoData.text = "";
			}
		}
	}
	//If input is empty, there is no song playing in Spotify
	else {
		infoData.status = 0;
		infoData.windowHwnd = nullptr;
		infoData.text = "";
	}
}


static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {

	//Grab the text lenght of the Window (length)
	int length = GetWindowTextLength(hWnd);
	DWORD proc;
	//Get the processid from the Window (proc)
	GetWindowThreadProcessId(hWnd, &proc);
	wchar_t wText[1024];
	//Get the Window text of the Window (wText)
	GetWindowTextW(hWnd, wText, length + 1);

	/*
	If the condition is met:
	Window is not in tray,
	length of Windows text is not empty,
	Process ids is really the Spotify Window.
	*/
	if (IsWindowVisible(hWnd) && length != 0 && find(pids.begin(), pids.end(), proc) != pids.end()) {
		ConvertData(wText, hWnd);
		return FALSE;
	}
	//If no condition is met, ignore everything else.
	else {
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
			//If process is Spotify.exe, there could be more processes (6 of them),
			//Push the process id's into the vector for later checks.
			if (_tcsicmp(entry.szExeFile, _T("Spotify.exe")) == 0)
			{
				pids.push_back(entry.th32ProcessID);
			}
		}
	}
	//Close the handle for process listing
	CloseHandle(handle);
	//If the process id's is empty. Spotify is not running.
	if (pids.empty()) {
		infoData.status = 0;
	}
	else {
		//If the process id's is not empty (Spotify is running), run a check for running Windows.
		EnumWindows(enumWindowCallback, NULL);
		pids.clear();
	}
}

//Writes out Artist - Title
extern "C" int __stdcall song(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	ReadData();
	
	if (infoData.text.empty()) {
		return 0;
	}
	else {
		strcpy_s(data, infoData.text.size() + 1, infoData.text.c_str());
		return 3;
	}
}

//Only return the Artist if " - " exists.
extern "C" int __stdcall artist(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	ReadData();
	if (infoData.text.empty()) {
		return 0;
	}
	else {
		size_t findPos = infoData.text.find(" - ");
		if (findPos != std::string::npos) {
			string artist = infoData.text.substr(0, findPos);
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
	if (infoData.text.empty()) {
		return 0;
	}
	else {
		size_t findPos = infoData.text.find(" - ");
		if (findPos != std::string::npos) {
			string title = infoData.text.substr(findPos + 3);
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
		 
		//Plays or pauses a track
		if (cmd == "playpause") {
			if (infoData.windowHwnd != nullptr) SendMessage(infoData.windowHwnd, WM_APPCOMMAND, 0, MediaControl::PlayPause);
		}
		//Plays a paused track
		else if (cmd == "play") {
			if (infoData.windowHwnd != nullptr) SendMessage(infoData.windowHwnd, WM_APPCOMMAND, 0, MediaControl::Play);
		}
		//Pauses a track if playing
		else if (cmd == "pause") {
			if (infoData.windowHwnd != nullptr) SendMessage(infoData.windowHwnd, WM_APPCOMMAND, 0, MediaControl::Pause);
		}
		//Playing next track
		else if (cmd == "next") {
			if (infoData.windowHwnd != nullptr) SendMessage(infoData.windowHwnd, WM_APPCOMMAND, 0, MediaControl::NextTrack);
		}
		//Plays track from beginning or previous track
		else if (cmd == "previous") {
			if (infoData.windowHwnd != nullptr) SendMessage(infoData.windowHwnd, WM_APPCOMMAND, 0, MediaControl::PreviousTrack);
		}
		//Plays the track forward
		else if (cmd == "forward") {
			if (infoData.windowHwnd != nullptr) SendMessage(infoData.windowHwnd, WM_APPCOMMAND, 0, MediaControl::ForwardTrack);
		}
		//plays the track backward
		else if (cmd == "rewind") {
			if (infoData.windowHwnd != nullptr) SendMessage(infoData.windowHwnd, WM_APPCOMMAND, 0, MediaControl::RewindTrack);
		}

		//Open up Spotify window
		else if (cmd == "show") {
			if (infoData.windowHwnd != nullptr) ShowWindow(infoData.windowHwnd, 1);
		}
	}
	return 0;
}

//Sending out who made this project.
extern "C" int __stdcall creator(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	strcpy_s(data, strlen(createdBy) + 1, createdBy);
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
* 2 - Spotfy is playing advertisement
* 3 - Spotify is playing a song
*/
extern "C" int __stdcall status(HWND mWnd, HWND aWnd, CHAR * data, char* parms, BOOL show, BOOL nopause)
{
	ReadData();
	strcpy_s(data, std::to_string(infoData.status).size() + 1, std::to_string(infoData.status).c_str());
	return 3;
}
