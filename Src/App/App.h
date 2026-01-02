#pragma once

//comment/uncomment this line to toggle EnableDpiAwareness in our imgui init
#define ImGui_EnableDpiAwareness
//comment/uncomment this line to toggle #include "imgui_demo.cpp"
#define ImGui_IncludeDemo
//must be set to exactly one of DirectX9, DirectX10, DirectX11, OpenGL2, OpenGL3
#define DirectX9

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <functional>
#include <filesystem>
#include <mutex>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>

#ifndef NODISCARD
#define NODISCARD [[nodiscard]]
#endif
#ifndef MAYBEUNUSED
#define MAYBEUNUSED [[maybe_unused]]
#endif

#if ( (defined DirectX9   ? 1 : 0) + \
      (defined DirectX10  ? 1 : 0) + \
      (defined DirectX11  ? 1 : 0) + \
      (defined OpenGL2    ? 1 : 0) + \
      (defined OpenGL3    ? 1 : 0) ) != 1
#error "You must define exactly one of DirectX9, DirectX10, DirectX11, OpenGL2, OpenGL3!"
#endif

#if defined(DirectX9) && !defined(_M_IX86)
#error "If you use DirectX9, you must also build as x86!"
#endif

#if defined DirectX9 || defined DirectX10 || defined DirectX11 || defined DirectX12
#define AnyDirectXActive
#elif defined(OpenGL2) || defined(OpenGL3)
#define AnyOpenGLActive
#endif

#if defined DirectX9
#define DX9_Reset_Addr                 app.GetDirectXDeviceMethodByIndex(16)
#define DX9_BeginScene_Addr            app.GetDirectXDeviceMethodByIndex(41)
#define DX9_EndScene_Addr              app.GetDirectXDeviceMethodByIndex(42)
#define DX9_DrawIndexedPrimitive_Addr  app.GetDirectXDeviceMethodByIndex(82)
#define DX9_Present_Addr               ((char*)(app.GetDirectXSwapChainMethodByIndex(3)) + 5)
#elifdef DirectX10
//todo
#elifdef DirectX11					   
#define DX11_Present_Addr              app.GetDirectXSwapChainMethodByIndex(8)
#elifdef DirectX12
//todo
#elifdef AnyOpenGLActive			   
#define OpenGL_wglMakeCurrent_Addr     wglMakeCurrent					     
#define OpenGL_SwapBuffers_Addr        app.Get_ProcAddress("gdi32.dll",      "SwapBuffers")
#endif							       									     
									   									     
#define GetCursorPos_Addr              ::GetCursorPos					     
#define SetCursorPos_Addr              ::SetCursorPos					     
#define ClipCursor_Addr                ::ClipCursor						     
#define CreateWindowExA_Addr           app.Get_ProcAddress("user32",         "CreateWindowExA")
#define NtUserDestroyWindow_Addr       app.Get_ProcAddress("win32u.dll",     "NtUserDestroyWindow")
#define NtUserSetCursorPos_Addr        app.Get_ProcAddress("win32u.dll",     "NtUserSetCursorPos")
#define DeviceIoControl_Addr           app.Get_ProcAddress("kernelbase.dll", "DeviceIoControl")

//pull imgui from github into your project folder and keep the imgui folder structure however it comes then the following imgui includes should work without issues.
#define _USE_MATH_DEFINES
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
//#include "imconfig.h"
//#include "imstb_rectpack.h"
//#include "imstb_textedit.h"
//#include "imstb_truetype.h"
#include "backends/imgui_impl_win32.h"
#include "misc/cpp/imgui_stdlib.h"

#if defined OpenGL2
#pragma comment(lib, "opengl32.lib")
#include "backends/imgui_impl_opengl2.h"
#include <GL/gl.h>
#elif defined OpenGL3
#pragma comment(lib, "opengl32.lib")
#include "backends/imgui_impl_opengl3_loader.h"
#include "backends/imgui_impl_opengl3.h"
#include <GL/gl.h>
#elif defined DirectX9
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9tex.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#include "backends/imgui_impl_dx9.h"
#elif defined DirectX10
#include <d3d10_1.h>
#include "backends/imgui_impl_dx10.h"
#elif defined DirectX11
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include "backends/imgui_impl_dx11.h"
#elif defined DirectX12 //DirectX12 not supported yet and may not ever be
#include "backends/imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_5.h>
#endif

#include <Psapi.h>

#ifdef _M_IX86
#include "polyhook2/Detour/x86Detour.hpp"
#elifdef _WIN64
#include "polyhook2/Detour/x64Detour.hpp"
#endif

#ifdef DirectX9
#include "App/Hooks/DX9_Hooks.h"
#elifdef DirectX10
#include "App/Hooks/DX10_Hooks.h"
#elifdef DirectX11
#include "App/Hooks/DX11_Hooks.h"
#elifdef AnyOpenGLActive
#include "App/Hooks/OpenGL_Hooks.h"
#endif

//hooks that are installable regardless of the renderer
#include "App/Hooks/Universal_Hooks.h"

#ifdef AnyOpenGLActive
extern "C" GLboolean APIENTRY glIsTexture(GLuint texture);
struct GLTexture
{
	GLuint id = 0;
	int width = 0;
	int height = 0;
};
#endif

struct CustomTexture
{
#ifdef DirectX9
	IDirect3DTexture9* ptr;
#elifdef DirectX10
	ID3D10ShaderResourceView* ptr;
#elifdef DirectX11
	ID3D11ShaderResourceView* ptr;
#elifdef AnyOpenGLActive
	GLTexture texture;
#endif

	CustomTexture() = default;
	CustomTexture(const CustomTexture&) = delete;
	CustomTexture& operator=(const CustomTexture&) = delete;
	CustomTexture(CustomTexture&&) = default;
	CustomTexture& operator=(CustomTexture&&) = default;

	~CustomTexture() noexcept
	{
#ifdef AnyOpenGLActive
		if (texture.id)
		{
			glDeleteTextures(1, &texture.id);
			texture.id = 0;
		}
#else
		if (ptr) { ptr->Release(); ptr = nullptr; }
#endif
	}

	//can add more fields here that are not dependent on the backend (size for example)
};

enum class AppKeys : int
{
	INVALID = -1, LBUTTON = 0x01, RBUTTON = 0x02, CANCEL = 0x03, MIDDLEMOUSE = 0x04, MOUSE4 = 0x05, MOUSE5 = 0x06, BACK = 0x08, TAB = 0x09, CLEAR = 0x0C, RETURN = 0x0D, SHIFT = 0x10,
	CTRL = 0x11, ALT = 0x12, PAUSE = 0x13, CAPSLOCK = 0x14, ESCAPE = 0x1B, CONVERT = 0x1C, NONCONVERT = 0x1D, ACCEPT = 0x1E, MODECHANGE = 0x1F, SPACE = 0x20, PAGEUP = 0x21, PAGEDOWN = 0x22,
	END = 0x23, HOME = 0x24, LEFT = 0x25, UP_ARROW = 0x26, RIGHT_ARROW = 0x27, DOWN_ARROW = 0x28, SELECT = 0x29, PRINT = 0x2A, EXECUTE = 0x2B, PRINTSCREEN = 0x2C, INSERT = 0x2D, DEL = 0x2E,
	HELP = 0x2F, KEY_0 = 0x30, KEY_1 = 0x31, KEY_2 = 0x32, KEY_3 = 0x33, KEY_4 = 0x34, KEY_5 = 0x35, KEY_6 = 0x36, KEY_7 = 0x37, KEY_8 = 0x38, KEY_9 = 0x39, KEY_A = 0x41, KEY_B = 0x42,
	KEY_C = 0x43, KEY_D = 0x44, KEY_E = 0x45, KEY_F = 0x46, KEY_G = 0x47, KEY_H = 0x48, KEY_I = 0x49, KEY_J = 0x4A, KEY_K = 0x4B, KEY_L = 0x4C, KEY_M = 0x4D, KEY_N = 0x4E, KEY_O = 0x4F,
	KEY_P = 0x50, KEY_Q = 0x51, KEY_R = 0x52, KEY_S = 0x53, KEY_T = 0x54, KEY_U = 0x55, KEY_V = 0x56, KEY_W = 0x57, KEY_X = 0x58, KEY_Y = 0x59, KEY_Z = 0x5A, LWIN = 0x5B, RWIN = 0x5C,
	APPKEY = 0x5D, SLEEP = 0x5F, NUMPAD_0 = 0x60, NUMPAD_1 = 0x61, NUMPAD_2 = 0x62, NUMPAD_3 = 0x63, NUMPAD_4 = 0x64, NUMPAD_5 = 0x65, NUMPAD_6 = 0x66, NUMPAD_7 = 0x67, NUMPAD_8 = 0x68,
	NUMPAD_9 = 0x69, NUMPAD_MULTIPLY = 0x6A, NUMPAD_ADD = 0x6B, NUMPAD_SEPARATOR = 0x6C, NUMPAD_SUBTRACT = 0x6D, NUMPAD_DECIMAL = 0x6E, NUMPAD_DIVIDE = 0x6F, KEY_F1 = 0x70, KEY_F2 = 0x71,
	KEY_F3 = 0x72, KEY_F4 = 0x73, KEY_F5 = 0x74, KEY_F6 = 0x75, KEY_F7 = 0x76, KEY_F8 = 0x77, KEY_F9 = 0x78, KEY_F10 = 0x79, KEY_F11 = 0x7A, KEY_F12 = 0x7B, KEY_F13 = 0x7C, KEY_F14 = 0x7D,
	KEY_F15 = 0x7E, KEY_F16 = 0x7F, KEY_F17 = 0x80, KEY_F18 = 0x81, KEY_F19 = 0x82, KEY_F20 = 0x83, KEY_F21 = 0x84, KEY_F22 = 0x85, KEY_F23 = 0x86, KEY_F24 = 0x87, NUMLOCK = 0x90, SCROLL = 0x91,
	LEFT_SHIFT = 0xA0, RIGHT_SHIFT = 0xA1, LEFT_CTRL = 0xA2, RIGHT_CTRL = 0xA3, LEFT_ALT = 0xA4, RIGHT_ALT = 0xA5, BROWSER_BACK = 0xA6, BROWSER_FORWARD = 0xA7, BROWSER_REFRESH = 0xA8,
	BROWSER_STOP = 0xA9, BROWSER_SEARCH = 0xAA, BROWSER_FAVORITES = 0xAB, BROWSER_HOME = 0xAC, VOLUME_MUTE = 0xAD, VOLUME_DOWN = 0xAE, VOLUME_UP = 0xAF, MEDIA_NEXT_TRACK = 0xB0,
	MEDIA_PREV_TRACK = 0xB1, MEDIA_STOP = 0xB2, MEDIA_PLAY_PAUSE = 0xB3, LAUNCH_MAIL = 0xB4, LAUNCH_MEDIA_SELECT = 0xB5, LAUNCH_APP1 = 0xB6, LAUNCH_APP2 = 0xB7, OEM_1 = 0xBA, OEM_PLUS = 0xBB,
	OEM_COMMA = 0xBC, OEM_MINUS = 0xBD, OEM_PERIOD = 0xBE, OEM_2 = 0xBF, OEM_3 = 0xC0, GAMEPAD_A = 0xC3, GAMEPAD_B = 0xC4, GAMEPAD_X = 0xC5, GAMEPAD_Y = 0xC6, GAMEPAD_RIGHT_SHOULDER = 0xC7,
	GAMEPAD_LEFT_SHOULDER = 0xC8, GAMEPAD_LEFT_TRIGGER = 0xC9, GAMEPAD_RIGHT_TRIGGER = 0xCA, GAMEPAD_DPAD_UP = 0xCB, GAMEPAD_DPAD_DOWN = 0xCC, GAMEPAD_DPAD_LEFT = 0xCD, GAMEPAD_DPAD_RIGHT = 0xCE,
	GAMEPAD_MENU = 0xCF, GAMEPAD_VIEW = 0xD0, GAMEPAD_LEFT_THUMBSTICK_BUTTON = 0xD1, GAMEPAD_RIGHT_THUMBSTICK_BUTTON = 0xD2, GAMEPAD_LEFT_THUMBSTICK_UP = 0xD3, GAMEPAD_LEFT_THUMBSTICK_DOWN = 0xD4,
	GAMEPAD_LEFT_THUMBSTICK_RIGHT = 0xD5, GAMEPAD_LEFT_THUMBSTICK_LEFT = 0xD6, GAMEPAD_RIGHT_THUMBSTICK_UP = 0xD7, GAMEPAD_RIGHT_THUMBSTICK_DOWN = 0xD8, GAMEPAD_RIGHT_THUMBSTICK_RIGHT = 0xD9,
	GAMEPAD_RIGHT_THUMBSTICK_LEFT = 0xDA, OEM_4 = 0xDB, OEM_5 = 0xDC, OEM_6 = 0xDD, OEM_7 = 0xDE, OEM_8 = 0xDF, OEM_102 = 0xE2, PROCESSKEY = 0xE5, PACKET = 0xE7
};

struct Patch
{
	uint64_t address;
	std::vector<uint8_t> patchBytes;
	std::vector<uint8_t> originalBytes;
};
struct Hook
{
	uint64_t original;
#ifdef _M_IX86
	PLH::x86Detour* detour;
#elifdef _WIN64
	PLH::x64Detour* detour;
#endif
};
class HookingLayer
{
private:
	std::map<std::string_view, Hook> hooks;
	std::vector<Patch> patches;
	bool hooks_applied = false;
	bool patches_applied = false;
public:
	HookingLayer();
	~HookingLayer();
	//call the original function for a hook you installed by name
	template<typename T> inline T GetOriginalFunction(const std::string_view& name)
	{
		auto it = hooks.find(name);
		if (it == hooks.end())
		{
			MessageBox(nullptr, std::format("You tried calling GetOriginalFunction for \"{}\" but that name was not registered! You likely made a typo somewhere. Terminating process...", name).c_str(), "F", MB_OK);
			ExitProcess(1);
		}
		return (T)it->second.original;
	}
	//Register hook data to be installed for when InstallHooks is called.
	void RegisterHook(const std::string_view& name, uint64_t funcToHook, uint64_t detour);
	//Register patch data to be installed for when InstallPatches is called.
	//If singleByteCount is greater than 0 and patchBytes is size 1 then it will write the single byte singleByteCount times.
	//otherwise, if patchBytes is non-empty and if singleByteCount is 0 (Default), then its ignored and patchBytes will override the bytes that are currently at 'address'.
	void RegisterPatch(uint64_t address, const std::vector<uint8_t>& patchBytes, size_t singleByteCount = 0u);
	//replace a function call (5 bytes) with another call
	void ReplaceCall(uint64_t callAddress, uint64_t newFunction);
	//install any patches you registered with RegisterPatch/ReplaceCall
	void InstallPatches();
	//uninstall all registered patches and replaced calls
	void UninstallPatches();
	//install all registered hooks
	void InstallHooks();
	//uninstall all registered hooks
	void UninstallHooks();
	//returns a pointer to a location within a specified module using a pattern.
	NODISCARD BYTE* FindPattern(const std::string_view& pattern, const std::string_view& module);
	//in case you have something in another thread that needs to wait for this task to be done
	NODISCARD inline bool AreHooksInstalled() const { return hooks_applied; }
	//in case you have something in another thread that needs to wait for this task to be done
	NODISCARD inline bool ArePatchesInstalled() const { return patches_applied; }
	//check if a specific hook has been installed by name
	NODISCARD bool IsHookIntalled(const std::string_view& name);
};

static thread_local int g_allowMouseWarpDepth = 0;
struct AllowMouseWarpScope
{
	AllowMouseWarpScope() { ++g_allowMouseWarpDepth; }
	~AllowMouseWarpScope() { --g_allowMouseWarpDepth; }
};

struct MouseStateSnapshot
{
private:
	POINT savedPos {};
	RECT savedClip {};
	bool havePos = false;
	bool haveClip = false;
public:
	inline void MouseStateSnapshot_OnMenuOpened()
	{
		havePos = (GetCursorPos(&savedPos) != 0);
		haveClip = (GetClipCursor(&savedClip) != 0);
		ClipCursor(nullptr);
	}

	inline void MouseStateSnapshot_OnMenuClosed() const
	{
		if (haveClip) ClipCursor(&savedClip);
		if (havePos)
		{
			AllowMouseWarpScope scope;
			SetCursorPos(savedPos.x, savedPos.y);
		}
	}
};

class App : public HookingLayer, public MouseStateSnapshot
{
private:
	bool isMenuOpen = false;
	bool isRendererActive = false;
	bool imguiFirstInitDone = false;
	bool needImGuiReload = false;
	bool isEjectingDLL = false;
	bool isTargetWindowFocused = false;
	HWND hwnd = nullptr;
	WNDPROC original_WndProc = nullptr;
	std::unordered_map<std::string_view, ImFont*> fonts;
	std::unordered_map<std::string_view, std::unique_ptr<CustomTexture>> textures;
	HMODULE dllHandle = 0;
	std::string_view targetWindowTitle;
	std::string_view targetWindowClassName;
	std::string_view targetWindowTitleName;
	POINT lastCursorPoint{};

#ifdef AnyOpenGLActive
	ULONGLONG reloadTickCount = 0ull;
#endif

private:
#ifdef DirectX9
	PDIRECT3DTEXTURE9 DX9_LoadTextureFromFile(const char* filename);
	PDIRECT3DTEXTURE9 DX9_LoadTextureFromMemory(void* data, size_t size);
#elifdef DirectX10
	ID3D10ShaderResourceView* DX10_LoadTextureFromFile(const char* filename);
	ID3D10ShaderResourceView* DX10_LoadTextureFromMemory(void* data, size_t size);
#elifdef DirectX11
	ID3D11ShaderResourceView* DX11_LoadTextureFromFile(const char* filename);
	ID3D11ShaderResourceView* DX11_LoadTextureFromMemory(void* data, size_t size);
//#elifdef DirectX12
//	ID3D12Resource* DX12_LoadTextureFromFile(const char* filename);
//	ID3D12Resource* DX12_LoadTextureFromMemory(void* data, size_t size, int knownWidth = 0, int knownHeight = 0) const;
#elifdef AnyOpenGLActive
	GLTexture GL_LoadTextureFromFile(const char* filename);
	GLTexture GL_LoadTextureFromMemory(void* data, size_t size);
#endif

#ifdef AnyOpenGLActive
	void GL_DestroyTexture(GLTexture& t);
	bool GL_CreateTextureRGBA8(const unsigned char* rgba, int w, int h, GLTexture& out);
#endif

	void FreeTextures();
	void FreeFonts();
	void Update_IsTargetWindowFocused();

public:
	App();
	~App();
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	App(App&&) = delete;
	App& operator=(App&&) = delete;

    //initialize ImGui
	void InitRenderer();
	//reload ImGui
	void ReloadRenderer();
	//shut down ImGui (call before you FreeLibraryAndExitThread and everything else will be handled for you)
	void Shutdown();
	//a wrapper for all new frame calls. also sets whether or not ImGui should render a mouse cursor by argument.
	void BeginFrame(bool want_mouse_this_frame);
	//a wrapper for all ImGui_X_RenderDrawData calls and end frame calls.
	void EndFrame() const;
	//reset WndProc back to default. handled by backend, but can be called manually if you need to in specific scenarios.
	void Reset_WndProc() const;
	//high jack the target WndProc. handled by backend, but can be called manually if you need to in specific scenarios.
	void Override_WndProc();
	//returns the original WndProc (For backend use. Can ignore this)
	NODISCARD inline WNDPROC Get_Original_WndProc() const { return original_WndProc; }
	//set this in dllmain just before jumping to your main thread!
	inline void Set_DLLHandle(HMODULE mod) { dllHandle = mod; };
	//use this as first arg to FreeLibraryAndExitThread for ejecting (make sure to do Set_DLLHandle in dllmain!
	NODISCARD inline HMODULE Get_DLLHandle() const { return dllHandle; }
	//whether or not ImGui is initialized
	NODISCARD inline bool IsRendererActive() const { return isRendererActive; }
	//set whether or not ImGui should be considered active (BeginFrame/EndFrame shouldn't be reachable if this is false!)
	inline void Set_RendererActive(bool active) { isRendererActive = active; }
	//has ImGui been initialized for the very first time
	NODISCARD inline bool HasInitializedFirstTime() const { return imguiFirstInitDone; }
	//notify that on the next frame we would like to reload ImGui
	inline void Set_ImGui_Reload(bool need_reload)
	{ 
		if (need_reload) 
		{ 
#ifdef AnyOpenGLActive
			reloadTickCount = GetTickCount64() + 5000ull;
#endif
			Set_RendererActive(false); 
		} 
		needImGuiReload = need_reload; }
	//check this every frame and when it's true, reload ImGui
	NODISCARD inline bool Need_ImGui_Reload() const
	{
#ifdef AnyOpenGLActive
		return needImGuiReload && GetTickCount64() > reloadTickCount;
#else
		return needImGuiReload;
#endif
	}
	//update the internal hwnd that App is aware of
	inline void Update_HWND(HWND hwnd) { this->hwnd = hwnd; }
	//key press event only (use provided AppKeys enum or you can cast an integer to AppKeys for the same effect as "VK_X")
	NODISCARD bool IsKeyPressed(AppKeys key_code) const;
    //checks if 'key1' is held while 'key2' is pressed only.
    //if a 3rd key is passed in, then it checks if both 'key1' and 'key2' are held while 'key3' is pressed only.
	NODISCARD bool IsKeyChordPressed(AppKeys key1, AppKeys key2, AppKeys key3 = AppKeys::INVALID) const;
	//key down event only (use provided AppKeys enum or you can cast an integer to AppKeys for the same effect as "VK_X")
	NODISCARD bool IsKeyDown(AppKeys key_code) const;
	//is target window in focus
	NODISCARD inline bool IsTargetWindowFocused() const { return this->isTargetWindowFocused; };
	//return the current hwnd that our App is aware of right now
	NODISCARD inline HWND GetHWND() const { return this->hwnd; };
	//a wrapper for ImGui_X_InvalidateDeviceObjects/ImGui_X_DestroyDeviceObjects
	void ImGui_InvalidateDeviceObjects() const;
	//a wrapper for ImGui_X_CreateDeviceObjects
	void ImGui_CreateDeviceObjects() const;
	//free any fonts or textures we allocated after init
	void FreeResources();
	//on inject you can set this one time if you want to 'HIGH_PRIORITY_CLASS' for example
	inline void Set_PriorityClass(DWORD dwPriorityClass) const { SetPriorityClass(GetCurrentProcess(), dwPriorityClass); };
	//a helper function that automatically checks if the HMODULE is valid first so we can avoid the "GetModuleHandle(x) could be 0" intellisense error.
	NODISCARD inline FARPROC Get_ProcAddress(const char* mod, const char* function) { HMODULE m = GetModuleHandle(mod); if (!m) return nullptr; return GetProcAddress(m, function); }
	//are we in the process of unloading our dll?
	NODISCARD inline bool IsEjecting() const { return isEjectingDLL; }
	//make this the first thing you call when you flag your dll to begin ejecting so certain sections unload properly
	inline void Start_Eject() { this->Set_RendererActive(false); isEjectingDLL = true; }
	//toggle your main tool overlay on/off
	inline void ToggleMenu(bool open) { isMenuOpen = open; }
	//is our tool overlay open/visible?
	NODISCARD inline bool IsMenuOpen() const { return isMenuOpen; }
	//make App aware of the target window titlename and classname you're workin with
	inline void Set_TargetWindowInfo(const std::string_view& title_name, const std::string_view& class_name) { this->targetWindowTitleName = title_name; this->targetWindowClassName = class_name; }
	//get the current target window classname the user specified on inject with Set_targetWindowInfo
	NODISCARD inline const std::string_view& Get_TargetWindowClassName() const { return targetWindowClassName; }
	//get the current target window title name the user specified on inject with Set_targetWindowInfo
	NODISCARD inline const std::string_view& Get_TargetWindowTitleName() const { return targetWindowTitleName; }
	//for back-end mouse stuff. ignore this.
	NODISCARD inline bool AllowMouseWarpNow() { return g_allowMouseWarpDepth > 0; }
	//Automatically create hooks for CreateWindowExA, NtUserDestroyWindow, GetCursorPos, NtUserSetCursorPos and DeviceIoControl; maybe more will be added in the future.
	//These can be used for any backend, hence "Universal". Doesn't include WndProc because that's already handled in the Init etc.
	//Note: this will not install the hooks. Only registers them. you will need to do app.InstallHooks() after registering these and the rest of your hooks, if any!
	//Recommended: call this function one time on inject before your app.InstallHooks() call or else when you open your menu, the mouse will act weird and these hooks take care of that.
	void RegisterUniversalHooks();
	//registers all backend specific hooks for you. Example: EndScene, Present, Reset etc for DirectX9
	//or if you're building for OpenGL then itll register SwapBuffers etc for you.
	//just simply call this function before you invoke InstallHooks()
	void RegisterBackEndHooks();

	//register a font from memory
	void AddFontFromMemory(const std::string_view& fontName, const void* fontData, int data_size, float initialFontSize = 13.0f);
	//register a font from a file
	void AddFontFromFile(const std::string_view& fontName, const std::string_view& path, float initialFontSize);
	//get a pointer to any font by name that was registered via AddFontFromMemory/AddFontFromFile
	NODISCARD ImFont* GetFontByName(const std::string_view& fontName);

	//a wrapper that loads a texture from a file regardless of the desired provided backend renderer
	void AddTextureFromFile(const std::string_view& name, const std::string_view& path);
	//a wrapper that loads a texture from memory regardless of the desired provided backend renderer
	void AddTextureFromMemory(const std::string_view& name, void* data, const size_t data_size);
	//get a pointer to any texture by name that was loaded via AddTextureFromMemory/AddTextureFromFile
	NODISCARD CustomTexture* GetTextureByName(const std::string_view& textureName);

#if defined DirectX9 || defined DirectX10 || defined DirectX11
	//use vtable index to access specific functions within the provided table.
    NODISCARD void* GetDirectXDeviceMethodByIndex(int index);
	//use vtable index to access specific functions within the provided table.
    NODISCARD void* GetDirectXSwapChainMethodByIndex(int index);
#endif
#if defined DirectX11
	//use vtable index to access specific functions within the provided table.
	void* GetDirectXContextMethodByIndex(int index);
#endif

#if defined DirectX10 || defined DirectX11
private:
	void* context = nullptr;
	void** ContextVTable = nullptr;
public:
	inline void UpdateDirectXContext(void* context) { this->context = context; UpdateDirectX_VTables(); }
#endif

#ifdef AnyDirectXActive
private:
	void* device = nullptr;
	void* swapChain = nullptr;
	void** DeviceVTable = nullptr;
	void** SwapChainVTable = nullptr;
	void UpdateDirectX_VTables();
public:
	//return a pointer to the last known device that App knows about
    NODISCARD inline void* GetDirectXDevice() { return device; }
	//make App aware of the latest valid device pointer for your target. this will also update the directx vtables too.
	inline void UpdateDirectXDevice(void* device) { this->device = device; UpdateDirectX_VTables(); };
#ifdef DirectX11
	inline void UpdateDirectXSwapChain(void* swapChain) { this->swapChain = swapChain; UpdateDirectX_VTables(); };
#endif
#endif

#if defined AnyOpenGLActive
private:
	const char* glsl_version = nullptr;
public:
	//after inject, initialize this once time somewhere. example string would be "#version 330 core"
	inline void SetGLSLVersion(const char* version) { glsl_version = version; }
	//returns a string of the known GLSL version that was provided by the user
	NODISCARD inline const char* GetGLSLVersion() { return glsl_version; }
#endif
};
extern App app;

