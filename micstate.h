#pragma once

#include <math.h>
#include <Windows.h>
#include <Tolk.h>
#include <bass.h>
#include <SimpleIni.h>

#define ID_EXIT 1002

int min_level; // The minimum level (in DB) to indicate muted state.

bool load_config();
void check_mic_state();
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
float linear_to_decibel(float linear);
void show_tray_icon(HWND hwnd, UINT id, HICON icon, LPSTR tooltip);
void destroy_tray_icon(HWND hwnd, UINT id);
void show_popup_menu(HWND hwnd);
