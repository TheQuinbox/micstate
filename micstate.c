#include <math.h>
#include <Windows.h>
#include <UniversalSpeech.h>
#include <bass.h>
#include <stdbool.h>
#include <stdio.h>
#include <shlwapi.h>

#define ID_TRAY_ICON 101
#define ID_EXIT 102
#define ID_CHECK_HOTKEY 103

int min_level; // The minimum level (in dB) to indicate muted state.

bool load_config();
void check_mic_state();
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
float linear_to_decibel(float linear);
void show_tray_icon(HWND hwnd, UINT id, HICON icon, LPCSTR message);
void destroy_tray_icon(HWND hwnd, UINT id);
void show_popup_menu(HWND hwnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow) {
	if (!BASS_RecordInit(-1)) {
		MessageBox(NULL, "Failed to initialize BASS.", "Error", MB_ICONERROR);
		return 1;
	}
	if (!load_config()) {
		MessageBox(NULL, "Couldn't load configuration.", "Error", MB_ICONERROR);
		return 1;
	}
	WNDCLASS wc = {0};
	wc.lpfnWndProc = wnd_proc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "MicStateWNDClass";
	RegisterClass(&wc);
	HWND hwnd = CreateWindow(wc.lpszClassName, NULL, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	HICON icon = LoadIcon(NULL, IDI_APPLICATION);
	show_tray_icon(hwnd, ID_TRAY_ICON, icon, "MicState");
	MSG msg = {0};;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnregisterHotKey(hwnd, ID_CHECK_HOTKEY);
	destroy_tray_icon(hwnd, ID_TRAY_ICON);
	return 0;
}

bool load_config() {
	char min_level_str[16] = {0};
	char config_path[MAX_PATH] = {0};
	if (GetCurrentDirectory(sizeof(config_path), config_path) == 0) return false;
	if (!PathCombine(config_path, config_path, "config.ini")) return false;
	if (GetPrivateProfileString("settings", "min_level", NULL, min_level_str, sizeof(min_level_str), config_path) == 0) return false;
	min_level = atoi(min_level_str);
	return true;
}

void check_mic_state() {
	HRECORD rec = BASS_RecordStart(0, 0, 0, NULL, NULL);
	if (rec == 0) {
		Beep(100, 100);
		return;
	}
	Sleep(50);
	float levels[2];
	BASS_ChannelGetLevelEx(rec, levels, 1.0, BASS_LEVEL_MONO);
	BASS_ChannelStop(rec);
	int level = (int)linear_to_decibel(levels[0]);
	speechSay(level <= min_level ? L"Muted" : L"Unmuted", TRUE);
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_CREATE:
		if (!RegisterHotKey(hwnd, ID_CHECK_HOTKEY, MOD_CONTROL | MOD_ALT | MOD_SHIFT, 'M')) {
			MessageBox(NULL, "Failed to register hotkey!", "Error", MB_ICONERROR | MB_OK);
			return -1;
		}
		break;
	case WM_HOTKEY:
		if (wp == ID_CHECK_HOTKEY) check_mic_state();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case ID_TRAY_ICON:
		switch (lp) {
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
			show_popup_menu(hwnd);
			break;
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wp) == ID_EXIT) PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	return 0;
}

float linear_to_decibel(float linear) {
	if (linear != 0.0f) return 20.0f * log10f(linear);
	return -144.0f;  // Effectively minus infinity
}

void show_tray_icon(HWND hwnd, UINT id, HICON icon, PCSTR message) {
	NOTIFYICONDATA nd = {0};
	nd.cbSize = sizeof(NOTIFYICONDATA);
	nd.hWnd = hwnd;
	nd.uID = id;
	nd.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nd.uCallbackMessage = ID_TRAY_ICON;
	nd.hIcon = icon;
	strncpy(nd.szTip, message, sizeof(nd.szTip) - 1);
	Shell_NotifyIcon(NIM_ADD, &nd);
}

void destroy_tray_icon(HWND hwnd, UINT id) {
	NOTIFYICONDATA nd = {0};
	nd.cbSize = sizeof(NOTIFYICONDATA);
	nd.hWnd = hwnd;
	nd.uID = id;
	Shell_NotifyIcon(NIM_DELETE, &nd);
}

void show_popup_menu(HWND hwnd) {
	HMENU menu = CreatePopupMenu();
	AppendMenu(menu, MF_STRING, ID_EXIT, "E&xit");
	POINT cursor;
	GetCursorPos(&cursor);
	SetForegroundWindow(hwnd);
	TrackPopupMenu(menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, cursor.x, cursor.y, 0, hwnd, NULL);
	DestroyMenu(menu);
}
