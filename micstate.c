#include <math.h>
#include <Windows.h>
#include <UniversalSpeech.h>
#include <bass.h>
#include <stdbool.h>
#include <stdio.h>

#define ID_TRAYICON 101
#define IDM_EXIT 102
#define IDH_CHECKHOTKEY 103

int min_level; // The minimum level (in dB) to indicate muted state.

bool LoadConfig();
void CheckMicState();
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
float LinearToDecibel(float linear);
void ShowTrayIcon(HWND hWnd, UINT uID, HICON hIcon, LPCSTR szToolTip);
void DestroyTrayIcon(HWND hWnd, UINT uID);
void ShowPopupMenu(HWND hWnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	if (!BASS_RecordInit(-1)) {
		MessageBox(NULL, "Failed to initialize BASS.", "Error", MB_ICONERROR);
		return 1;
	}
	if (!LoadConfig()) {
		MessageBox(NULL, "Couldn't load configuration.", "Error", MB_ICONERROR);
		return 1;
	}
	WNDCLASS wc = {0};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "MicStateWNDClass";
	RegisterClass(&wc);
	HWND hwnd = CreateWindow(wc.lpszClassName, NULL, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	HICON icon = LoadIcon(NULL, IDI_APPLICATION);
	ShowTrayIcon(hwnd, ID_TRAYICON, icon, "MicState");
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnregisterHotKey(hwnd, IDH_CHECKHOTKEY);
	DestroyTrayIcon(hwnd, ID_TRAYICON);
	return 0;
}

bool LoadConfig() {
	const char* filename = "config.ini";
	const char* section = "DEFAULT";
	const char* key = "min_level";
	char buffer[16];
	int min_level_default = -60;
	DWORD charsRead = GetPrivateProfileString(section, key, NULL, buffer, sizeof(buffer), filename);
	if (charsRead == 0) {
		char defaultValue[16];
		snprintf(defaultValue, sizeof(defaultValue), "%d", min_level_default);
		WritePrivateProfileString(section, key, defaultValue, filename);
		charsRead = GetPrivateProfileString(section, key, NULL, buffer, sizeof(buffer), filename);
		if (charsRead == 0) return false;
	}
	min_level = atoi(buffer);
	return true;
}

void CheckMicState() {
	HRECORD rec = BASS_RecordStart(0, 0, 0, NULL, NULL);
	if (rec == 0) {
		Beep(100, 100);
		return;
	}
	Sleep(50);
	float levels[2];
	BASS_ChannelGetLevelEx(rec, levels, 1.0, BASS_LEVEL_MONO);
	BASS_ChannelStop(rec);
	int level = (int)LinearToDecibel(levels[0]);
	speechSay(level <= min_level ? L"Muted" : L"Unmuted", TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:
		if (!RegisterHotKey(hWnd, IDH_CHECKHOTKEY, MOD_CONTROL | MOD_ALT | MOD_SHIFT, 'M')) {
			MessageBox(NULL, "Failed to register hotkey!", "Error", MB_ICONERROR | MB_OK);
			return -1;
		}
		break;
	case WM_HOTKEY:
		if (wParam == IDH_CHECKHOTKEY) CheckMicState();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case ID_TRAYICON:
		switch (lParam) {
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
			ShowPopupMenu(hWnd);
			break;
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDM_EXIT) PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

float LinearToDecibel(float linear) {
	if (linear != 0.0f) return 20.0f * log10f(linear);
	return -144.0f;  // Effectively minus infinity
}

void ShowTrayIcon(HWND hWnd, UINT uID, HICON hIcon, PCSTR szToolTip) {
	NOTIFYICONDATA nd = {0};
	nd.cbSize = sizeof(NOTIFYICONDATA);
	nd.hWnd = hWnd;
	nd.uID = uID;
	nd.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nd.uCallbackMessage = ID_TRAYICON;
	nd.hIcon = hIcon;
	strncpy(nd.szTip, szToolTip, sizeof(nd.szTip) - 1);
	Shell_NotifyIcon(NIM_ADD, &nd);
}

void DestroyTrayIcon(HWND hWnd, UINT uID) {
	NOTIFYICONDATA ndata = {0};
	ndata.cbSize = sizeof(NOTIFYICONDATA);
	ndata.hWnd = hWnd;
	ndata.uID = uID;
	Shell_NotifyIcon(NIM_DELETE, &ndata);
}

void ShowPopupMenu(HWND hWnd) {
	HMENU menu = CreatePopupMenu();
	AppendMenu(menu, MF_STRING, IDM_EXIT, "E&xit");
	POINT cursor;
	GetCursorPos(&cursor);
	SetForegroundWindow(hWnd);
	TrackPopupMenu(menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, cursor.x, cursor.y, 0, hWnd, NULL);
	DestroyMenu(menu);
}
