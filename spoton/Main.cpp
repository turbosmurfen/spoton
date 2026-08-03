/* Play/Pause, Next, Previous

Copyright 2010 Marcus Lonnberg

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

Copyright 2026 Turbosmurfen

Licensed under the Creative Commons License, version 4.0
https://creativecommons.org/licenses/by/4.0/

*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>

#pragma comment(linker, "/EXPORT:song=_song@24")
#pragma comment(linker, "/EXPORT:artist=_artist@24")
#pragma comment(linker, "/EXPORT:title=_title@24")
#pragma comment(linker, "/EXPORT:creator=_creator@24")
#pragma comment(linker, "/EXPORT:version=_version@24")
#pragma comment(linker, "/EXPORT:status=_status@24")
#pragma comment(linker, "/EXPORT:control=_control@24")

constexpr char version_[] = "1.2.2";
constexpr char createdBy[] = "Created by: Turbosmurfen";

// Size of the buffer mIRC hands us in the `data` parameter. Everything written
// back to mIRC is bounded by this.
constexpr size_t mircDataSize = 900;

// A window title longer than this is treated as garbage rather than a song.
constexpr int maxTitleBytes = 1024;

// How long a single process/window scan stays valid. A script asking for
// $song, $artist and $title in a row then costs one scan instead of three.
constexpr ULONGLONG cacheLifetimeMs = 200;

enum class SpotifyStatus {
	NotRunning = 0,
	Paused = 1,
	Advertisement = 2,
	Playing = 3,
	DJ = 4,
	Error = 5
};

struct Info {
	SpotifyStatus status = SpotifyStatus::NotRunning;
	std::string text = "";
	HWND windowHwnd = nullptr;
};

struct EnumData {
	std::vector<DWORD> spotifyPIDs;
	Info result;         // filled in by the enum callback
	bool found = false;  // stop at the first Spotify window we can read
};

enum MediaControl {
	Play = 3014656, //APPCOMMAND_MEDIA_PLAY * 0x10000
	Pause = 3080192, //APPCOMMAND_MEDIA_PAUSE * 0x10000
	PlayPause = 917504, //APPCOMMAND_MEDIA_PLAY_PAUSE * 0x10000
	NextTrack = 720896, //APPCOMMAND_MEDIA_NEXTTRACK * 0x10000
	PreviousTrack = 786432, //APPCOMMAND_MEDIA_PREVIOUSTRACK * 0x10000
	RewindTrack = 3276800, //APPCOMMAND_MEDIA_REWIND * 0x10000
	ForwardTrack = 3211264, //APPCOMMAND_MEDIA_FAST_FORWARD * 0x10000
};

// Runs an export body and swallows anything thrown. A C++ exception escaping
// through an extern "C" boundary into mIRC is undefined behaviour, and every
// std::string operation below can throw std::bad_alloc.
template <typename Fn>
static int SafeCall(Fn&& fn) noexcept {
	try {
		return fn();
	}
	catch (...) {
		return 0;
	}
}

// Largest prefix of `text` that fits in maxBytes without splitting a UTF-8
// sequence. Continuation bytes match 10xxxxxx, so we back up past any of them
// to land on a character boundary.
static size_t Utf8SafeLength(const std::string& text, size_t maxBytes) {
	if (text.size() <= maxBytes) {
		return text.size();
	}
	size_t len = maxBytes;
	while (len > 0 && (static_cast<unsigned char>(text[len]) & 0xC0) == 0x80) {
		--len;
	}
	return len;
}

// Copies text back into mIRC's buffer, truncating rather than overflowing it,
// and returns the value mIRC expects from a $dll() identifier.
static int ReturnToMirc(char* data, const std::string& text) {
	if (data == nullptr || text.empty()) {
		return 0;
	}
	const size_t len = Utf8SafeLength(text, mircDataSize - 1); //reserve the null terminator
	if (len == 0) {
		return 0;
	}
	memcpy_s(data, mircDataSize, text.data(), len);
	data[len] = '\0';
	return 3;
}

static void ConvertData(const wchar_t* input, HWND hWnd, Info& out) {

	//If input is empty, there is no song playing in Spotify
	if (input == nullptr || *input == L'\0') {
		out = { SpotifyStatus::NotRunning, "", nullptr };
		return;
	}

	//if these conditions are met, Spotify is paused.
	if (wcscmp(input, L"Spotify Premium") == 0 || wcscmp(input, L"Spotify Free") == 0) {
		out = { SpotifyStatus::Paused, "", hWnd };
		return;
	}
	//if this condition is met, Spotify is playing an advertisement
	if (wcscmp(input, L"Advertisement") == 0) {
		out = { SpotifyStatus::Advertisement, "", hWnd };
		return;
	}
	//if the title starts with this, Spotify is running a DJ session
	constexpr wchar_t djPrefix[] = L"DJ X - ";
	if (wcsncmp(input, djPrefix, std::size(djPrefix) - 1) == 0) {
		out = { SpotifyStatus::DJ, "", hWnd };
		return;
	}

	//If not the conditions before is met, Spotify is playing a song

	//Convert to UTF-8. The returned length includes the terminating null.
	const int bufferSize = WideCharToMultiByte(CP_UTF8, 0, input, -1, nullptr, 0, nullptr, nullptr);
	if (bufferSize <= 1 || bufferSize > maxTitleBytes) {
		//If conversion failed, set status to error
		out = { SpotifyStatus::Error, "", hWnd };
		return;
	}

	std::string text(static_cast<size_t>(bufferSize), '\0');
	if (WideCharToMultiByte(CP_UTF8, 0, input, -1, text.data(), bufferSize, nullptr, nullptr) == 0) {
		out = { SpotifyStatus::Error, "", hWnd };
		return;
	}
	text.resize(static_cast<size_t>(bufferSize) - 1);

	//Sanitise the text.
	//Not just CR/LF. That covers CTCP (\x01) and the mIRC formatting codes as well.
	//UTF-8 continuation bytes are all >= 0x80, so multi-byte characters survive.
	std::replace_if(text.begin(), text.end(),
		[](char c) {
			const unsigned char uc = static_cast<unsigned char>(c);
			return uc < 0x20 || uc == 0x7F;
		},
		' ');

	out = { SpotifyStatus::Playing, std::move(text), hWnd };
}

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
	EnumData* data = reinterpret_cast<EnumData*>(lparam);

	if (!IsWindowVisible(hWnd)) {
		return TRUE;
	}

	DWORD procId = 0;
	GetWindowThreadProcessId(hWnd, &procId);
	if (std::find(data->spotifyPIDs.begin(), data->spotifyPIDs.end(), procId) == data->spotifyPIDs.end()) {
		return TRUE;
	}

	wchar_t title[1024];
	if (GetWindowTextW(hWnd, title, static_cast<int>(std::size(title))) > 0) {
		ConvertData(title, hWnd, data->result);
		data->found = true;
		return FALSE; //first readable Spotify window wins; don't let later ones overwrite it
	}

	return TRUE;
}

static std::vector<DWORD> FindSpotifyPIDs() {
	std::vector<DWORD> spotifyPIDs;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (snap == INVALID_HANDLE_VALUE) {
		return spotifyPIDs;
	}

	if (Process32First(snap, &entry)) {
		do {
			if (_tcsicmp(entry.szExeFile, _T("Spotify.exe")) == 0) {
				spotifyPIDs.push_back(entry.th32ProcessID);
			}
		} while (Process32Next(snap, &entry));
	}

	CloseHandle(snap);
	return spotifyPIDs;
}

static Info ReadData() {
	static Info cached;
	static ULONGLONG cachedAt = 0;

	const ULONGLONG now = GetTickCount64();
	if (cachedAt != 0 && now - cachedAt < cacheLifetimeMs) {
		return cached;
	}

	EnumData data;
	data.spotifyPIDs = FindSpotifyPIDs();
	if (!data.spotifyPIDs.empty()) {
		EnumWindows(enumWindowCallback, reinterpret_cast<LPARAM>(&data));
	}

	cached = data.result; //default Info when Spotify is gone or has no readable window
	cachedAt = now;
	return cached;
}

// The handle in Info can be up to cacheLifetimeMs old, and Windows recycles
// window handles. Re-check that it is still alive and still owned by Spotify
// before acting on it, so we never send input to an unrelated window.
static bool IsStillSpotifyWindow(HWND hWnd) {
	if (hWnd == nullptr || !IsWindow(hWnd)) {
		return false;
	}
	DWORD procId = 0;
	GetWindowThreadProcessId(hWnd, &procId);
	if (procId == 0) {
		return false;
	}
	const std::vector<DWORD> pids = FindSpotifyPIDs();
	return std::find(pids.begin(), pids.end(), procId) != pids.end();
}

// Sends a media key to Spotify. SendMessageTimeout so a hung Spotify cannot
// freeze mIRC, which a plain SendMessage would do indefinitely.
static void SendMediaCommand(HWND hWnd, MediaControl command) {
	if (!IsStillSpotifyWindow(hWnd)) {
		return;
	}
	DWORD_PTR result = 0;
	SendMessageTimeout(hWnd, WM_APPCOMMAND, 0, command,
		SMTO_ABORTIFHUNG | SMTO_NORMAL, 1000, &result);
}

//Writes out Artist - Title
extern "C" int __stdcall song(HWND mWnd, HWND aWnd, CHAR* data, char* parms, BOOL show, BOOL nopause)
{
	return SafeCall([&] {
		const Info info = ReadData();
		return ReturnToMirc(data, info.text);
	});
}

//Only return the Artist if " - " exists.
extern "C" int __stdcall artist(HWND mWnd, HWND aWnd, CHAR* data, char* parms, BOOL show, BOOL nopause)
{
	return SafeCall([&] {
		const Info info = ReadData();
		const size_t findPos = info.text.find(" - ");
		if (findPos == std::string::npos) {
			return 0;
		}
		return ReturnToMirc(data, info.text.substr(0, findPos));
	});
}

//Only return the Title if " - " exists.
extern "C" int __stdcall title(HWND mWnd, HWND aWnd, CHAR* data, char* parms, BOOL show, BOOL nopause)
{
	return SafeCall([&] {
		const Info info = ReadData();
		const size_t findPos = info.text.find(" - ");
		if (findPos == std::string::npos) {
			return 0;
		}
		return ReturnToMirc(data, info.text.substr(findPos + 3));
	});
}


//Media Controls
extern "C" int __stdcall control(HWND mWnd, HWND aWnd, CHAR* data, char* parms, BOOL show, BOOL nopause)
{
	return SafeCall([&] {
		constexpr size_t MAX_CMD_LEN = 10; // Maximum length of command
		// Check if data is null
		if (data == nullptr) return 0;
		// Ensure data is null-terminated and within the maximum length
		size_t inputLen = strnlen(data, MAX_CMD_LEN + 1);
		if (inputLen == 0 || inputLen > MAX_CMD_LEN) {
			// Input is empty or too long
			return 0;
		}
		// Check if the input contains only alphabetic characters
		std::string cmd(data, inputLen);
		if (!std::all_of(cmd.begin(), cmd.end(), [](char c) {
			return isalpha(static_cast<unsigned char>(c)) != 0;
			})) {
			return 0;
		}

		const Info info = ReadData();

		//Plays or pauses a track
		if (cmd == "playpause") {
			SendMediaCommand(info.windowHwnd, MediaControl::PlayPause);
		}
		//Plays a paused track
		else if (cmd == "play") {
			SendMediaCommand(info.windowHwnd, MediaControl::Play);
		}
		//Pauses a track if playing
		else if (cmd == "pause") {
			SendMediaCommand(info.windowHwnd, MediaControl::Pause);
		}
		//Playing next track
		else if (cmd == "next") {
			SendMediaCommand(info.windowHwnd, MediaControl::NextTrack);
		}
		//Plays track from beginning or previous track
		else if (cmd == "previous") {
			SendMediaCommand(info.windowHwnd, MediaControl::PreviousTrack);
		}
		//Plays the track forward
		else if (cmd == "forward") {
			SendMediaCommand(info.windowHwnd, MediaControl::ForwardTrack);
		}
		//plays the track backward
		else if (cmd == "rewind") {
			SendMediaCommand(info.windowHwnd, MediaControl::RewindTrack);
		}
		//Open up Spotify window
		else if (cmd == "show") {
			if (IsStillSpotifyWindow(info.windowHwnd)) {
				ShowWindow(info.windowHwnd, SW_SHOWNORMAL);
			}
		}

		//If no command is matched, return 0 as well.
		return 0;
	});
}

//Sending out who made this project.
extern "C" int __stdcall creator(HWND mWnd, HWND aWnd, CHAR* data, char* parms, BOOL show, BOOL nopause)
{
	return SafeCall([&] { return ReturnToMirc(data, createdBy); });
}

//Sending out which version of Spoton that you use.
extern "C" int __stdcall version(HWND mWnd, HWND aWnd, CHAR* data, char* parms, BOOL show, BOOL nopause)
{
	return SafeCall([&] { return ReturnToMirc(data, version_); });
}

/* Sending out the numbers of status (0-5)
* 0 - Spotify is not running
* 1 - Spotify is paused
* 2 - Spotfy is playing advertisement
* 3 - Spotify is playing a song
* 4 - Spotify is playing a DJ session
* 5 - Spotify is in error state
*/
extern "C" int __stdcall status(HWND mWnd, HWND aWnd, CHAR* data, char* parms, BOOL show, BOOL nopause)
{
	return SafeCall([&] {
		const Info info = ReadData();
		return ReturnToMirc(data, std::to_string(static_cast<int>(info.status)));
	});
}
