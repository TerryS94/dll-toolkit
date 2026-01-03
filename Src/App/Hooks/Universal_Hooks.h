#pragma once
#include "Main.h"

//ready-to-go hooks that will work regardless of the backend/renderer hence "Universal"
//for backend specific hooks, check out Hooks/{backend_name}_Hooks.h

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef HWND(WINAPI* tCreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD  dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
typedef BOOL(WINAPI* tNtUserDestroyWindow)(HWND hWnd);
typedef BOOL(WINAPI* tGetCursorPos)(LPPOINT point);
typedef BOOL(WINAPI* tNtUserSetCursorPos)(int x, int y);
typedef BOOL(WINAPI* tSetCursorPos)(int x, int y);
typedef BOOL(WINAPI* tClipCursor)(const RECT* rc);
typedef BOOL(WINAPI* tDeviceIoControl)(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
typedef BOOL(WINAPI* tScreenToClient)(HWND hWnd, LPPOINT lpPoint);

namespace ProvidedDetours
{
	//WndProc lives here cause makes sense. However you do NOT need to worry about high-jacking/restoring it as that is already automated for you.
	LRESULT WINAPI WndProc_Detour(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND WINAPI CreateWindowExA_Detour(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD  dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	BOOL WINAPI NtUserDestroyWindow_Detour(HWND hWnd);
	BOOL WINAPI GetCursorPos_Detour(LPPOINT point);
	BOOL WINAPI SetCursorPos_Detour(int x, int y);
	BOOL WINAPI NtUserSetCursorPos_Detour(int x, int y);
	BOOL WINAPI ClipCursor_Detour(const RECT* rc);
	BOOL WINAPI DeviceIoControl_Detour(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
	BOOL WINAPI ScreenToClient_Detour(HWND hWnd, LPPOINT lpPoint);
}