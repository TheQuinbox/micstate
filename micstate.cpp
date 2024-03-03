#include <math.h>
#include <Windows.h>
#include <Tolk.h>
#include <bass.h>
#include <SimpleIni.h>
#include "micstate.h"

int WINAPI WinMain(HINSTANCE inst, HINSTANCE pinst, LPSTR cmdline, int show) {
	Tolk_Load();
	Tolk_TrySAPI(true);
	if (!BASS_RecordInit(-1)) {
		MessageBox(NULL, "Failed to initialize Bass.", "Error", MB_ICONERROR);
		return 1;
	}
	if (!load_config()) {
		MessageBox(NULL, "Couldn't load configuration.", "Error", MB_ICONERROR);
		return 1;
	}
	WNDCLASS wc = {0};
	wc.lpfnWndProc = wnd_proc;
	wc.hInstance = inst;
	wc.lpszClassName = "MicstateWNDClass";
	RegisterClass(&wc);
	HWND hwnd = CreateWindow(wc.lpszClassName, 0, 0, 0, 0, 0, 0, 0, 0, inst, 0);
	HICON icon = LoadIcon(NULL, IDI_APPLICATION);
	show_tray_icon(hwnd, 1, icon, "Micstate");
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	Tolk_Unload();
	UnregisterHotKey(hwnd, 1);
	destroy_tray_icon(hwnd, 1);
	return 0;
}

bool load_config() {
	const char* filename = "config.ini";
	CSimpleIniA ini;
	SI_Error rc = ini.LoadFile(filename);
	if (rc < 0) {
		ini.SetValue("DEFAULT", "min_level", "-60");
		ini.SaveFile(filename);
	}
	rc = ini.LoadFile(filename);
	if (rc < 0) return false;
	const char* min_level_str = ini.GetValue("DEFAULT", "min_level", "-60");
	min_level = std::atoi(min_level_str);
	return true;
}

void check_mic_state() {
	HRECORD rec = BASS_RecordStart(0, 0, 0, NULL, NULL);
	if (rec == 0) {
		Beep(100, 100);
		return;
	}
	Sleep(50);
	FLOAT levels[2];
	BASS_ChannelGetLevelEx(rec, levels, 1.0, BASS_LEVEL_MONO);
	BASS_ChannelFree(rec);
	int level = linear_to_decibel(levels[0]);
	Tolk_Output(level <= min_level ? L"Muted" : L"Unmuted", true);
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_CREATE:
		if (!RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_ALT | MOD_SHIFT, 'M')) {
			MessageBox(NULL, "Failed to register hotkey!", "Error", MB_ICONERROR | MB_OK);
			return -1;
		}
		break;
	case WM_HOTKEY:
		if (wp == 1)
			check_mic_state();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_USER + 1:
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
		switch (LOWORD(wp)) {
		case ID_EXIT:
			PostQuitMessage(0);
			break;
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	return 0;
}

// https://forum.juce.com/t/float-to-decibel-conversion/1841
float linear_to_decibel(float linear) {
	float db;
	if (linear != 0.0f)
		db = 20.0f * log10(linear);
	else
		db = -144.0f;  // effectively minus infinity
	return db;
}

void show_tray_icon(HWND hwnd, UINT id, HICON icon, LPSTR tooltip) {
	NOTIFYICONDATA ndata;
	ndata.cbSize = sizeof(NOTIFYICONDATA);
	ndata.hWnd = hwnd;
	ndata.uID = id;
	ndata.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	ndata.uCallbackMessage = WM_USER + 1;
	ndata.hIcon = icon;
	strcpy_s(ndata.szTip, sizeof(ndata.szTip) / sizeof(ndata.szTip[0]), tooltip);
	Shell_NotifyIcon(NIM_ADD, &ndata);
}

void destroy_tray_icon(HWND hwnd, UINT id) {
	NOTIFYICONDATA ndata;
	ndata.cbSize = sizeof(NOTIFYICONDATA);
	ndata.hWnd = hwnd;
	ndata.uID = id;
	Shell_NotifyIcon(NIM_DELETE, &ndata);
}

void show_popup_menu(HWND hwnd) {
	HMENU menu = CreatePopupMenu();
	AppendMenu(menu, MF_STRING, ID_EXIT, "Exit");
	POINT cursor;
	GetCursorPos(&cursor);
	// So this is really stupid, if we don't call this the menu will never take keyboard or mouse focus, even though our window is invisible.
	SetForegroundWindow(hwnd);
	TrackPopupMenu(menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, cursor.x, cursor.y, 0, hwnd, NULL);
	PostMessage(hwnd, WM_NULL, 0, 0);
	DestroyMenu(menu);
}
