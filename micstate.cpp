#include <Windows.h>
#include <Mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <Tolk.h>

// Yes, globals, I know, sorry about that.
IMMDeviceEnumerator* enumerator;
IMMDevice* default_device;
IAudioEndpointVolume* endpoint_volume;

void check_mic_state();
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

int WINAPI WinMain(HINSTANCE inst, HINSTANCE pinst, LPSTR cmdline, int show) {
	HRESULT hr;
	CoInitialize(NULL);
	Tolk_Load();
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&enumerator);
	if (FAILED(hr)) {
		CoUninitialize();
		return 1;
	}
	hr = enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &default_device);
	if (FAILED(hr)) {
		enumerator->Release();
		CoUninitialize();
		return 1;
	}
	hr = default_device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpoint_volume);
	if (FAILED(hr)) {
		default_device->Release();
		enumerator->Release();
		CoUninitialize();
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
	endpoint_volume->Release();
	default_device->Release();
	enumerator->Release();
	Tolk_Unload();
	UnregisterHotKey(hwnd, 1);
	CoUninitialize();
	return 0;
}

void check_mic_state() {
	BOOL is_muted;
	endpoint_volume->GetMute(&is_muted);
	Tolk_Output(is_muted ? L"Muted" : L"Unmuted");
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
		endpoint_volume->Release();
		default_device->Release();
		enumerator->Release();
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
