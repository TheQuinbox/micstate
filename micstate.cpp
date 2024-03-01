#include <math.h>
#include <Windows.h>
#include <Tolk.h>
#include <bass.h>
#include <SimpleIni.h>

int min_level; // The minimum level (in DB) to indicate muted state.

bool load_config();
void check_mic_state();
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
float linear_to_decibel(float linear);

int WINAPI WinMain(HINSTANCE inst, HINSTANCE pinst, LPSTR cmdline, int show) {
	HRESULT hr;
	CoInitialize(NULL);
	Tolk_Load();
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
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
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
	Tolk_Output(level <= min_level ? L"Muted" : L"Unmuted");
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
		Tolk_Unload();
		UnregisterHotKey(hwnd, 1);
		CoUninitialize();
		PostQuitMessage(0);
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
