#include "Universal_Hooks.h"

//ready-to-go hooks that will work regardless of the backend/renderer hence "Universal"
//for backend specific hooks, check out Hooks/{backend_name}_Hooks.h

namespace ProvidedDetours
{
	//in case you didn't look at the header. Do NOT worry about high-jacking/restoring WndProc cause that's already automated for you.
	//Simply just add whatever logic you want in here and you're good to go :)
	LRESULT WINAPI WndProc_Detour(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		ImGuiContext* ctx = ImGui::GetCurrentContext();
		const bool menuOpen = (ctx && app.HasMouseCursor());
		bool blockMouse = false;
		bool blockKeyboard = false;

		if (ctx)
		{
			ImGuiIO& io = ImGui::GetIO();
			blockMouse = menuOpen ? true : io.WantCaptureMouse;
			blockKeyboard = menuOpen ? true : io.WantCaptureKeyboard;
		}
		if (blockMouse)
		{
			switch (uMsg)
			{
				case WM_MOUSEMOVE:
				case WM_LBUTTONDOWN: case WM_LBUTTONUP:
				case WM_RBUTTONDOWN: case WM_RBUTTONUP:
				case WM_MBUTTONDOWN: case WM_MBUTTONUP:
				case WM_XBUTTONDOWN: case WM_XBUTTONUP:
				case WM_MOUSEWHEEL:
				case WM_MOUSEHWHEEL:
					return 0;
			}
		}
		if (blockKeyboard)
		{
			switch (uMsg)
			{
				case WM_KEYDOWN: case WM_KEYUP:
				case WM_SYSKEYDOWN: case WM_SYSKEYUP:
				case WM_CHAR: case WM_SYSCHAR:
					return 0;
			}
		}
		if (menuOpen && uMsg == WM_INPUT)
			return 0;

		//add stuff here

		return CallWindowProc(app.Get_Original_WndProc(), hWnd, uMsg, wParam, lParam);
	}

	HWND WINAPI CreateWindowExA_Detour(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
	{
		HWND windowHandle = app.GetOriginalFunction<tCreateWindowExA>("CreateWindowExA")(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		const std::string_view& appClassName = app.Get_TargetWindowClassName();
		const std::string_view& appTitleName = app.Get_TargetWindowTitleName();
		if ((!appClassName.empty() && std::string_view(lpClassName) == appClassName) ||
			(!appTitleName.empty() && std::string_view(lpWindowName) == appTitleName))
		{
			app.Override_WndProc();
		}
		return windowHandle;
	}

	BOOL WINAPI NtUserDestroyWindow_Detour(HWND hwnd)
	{
		if (hwnd && hwnd == app.GetHWND()) app.Reset_WndProc();
		BOOL result = app.GetOriginalFunction<tNtUserDestroyWindow>("NtUserDestroyWindow")(hwnd);
		return result;
	}

	BOOL WINAPI GetCursorPos_Detour(LPPOINT point)
	{
		const BOOL result = app.GetOriginalFunction<tGetCursorPos>("GetCursorPos")(point);
		return result;
	}

	BOOL WINAPI SetCursorPos_Detour(int x, int y)
	{
		if (app.HasMouseCursor() && !app.AllowMouseWarpNow())
			return TRUE;
		return app.GetOriginalFunction<tSetCursorPos>("SetCursorPos")(x, y);
	}

	BOOL WINAPI NtUserSetCursorPos_Detour(int x, int y)
	{
		if (app.HasMouseCursor() && !app.AllowMouseWarpNow())
			return TRUE;
		return app.GetOriginalFunction<tNtUserSetCursorPos>("NtUserSetCursorPos")(x, y);
	}

	BOOL WINAPI ClipCursor_Detour(const RECT* rc)
	{
		if (app.HasMouseCursor()) return TRUE;
		return app.GetOriginalFunction<tClipCursor>("ClipCursor")(rc);
	}

	BOOL WINAPI DeviceIoControl_Detour(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
	{
		BOOL success = app.GetOriginalFunction<tDeviceIoControl>("DeviceIoControl")(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
		return success;
	}
	BOOL WINAPI ScreenToClient_Detour(HWND hWnd, LPPOINT lpPoint)
	{
		BOOL result = app.GetOriginalFunction<tScreenToClient>("ScreenToClient")(hWnd, lpPoint);
		return result;
	}
}