//This file is part of the dll-tookit by TerryS94 -> https://github.com/TerryS94/dll-toolkit

#pragma once
#include "App/App.h"

typedef BOOL(__stdcall* tOpenGL_SwapBuffers)(HDC hdc);
typedef BOOL(__stdcall* tWglMakeCurrent)(HDC hdc, HGLRC hglrc);

namespace ProvidedDetours
{
	BOOL WINAPI wglMakeCurrent_Detour(HDC hdc, HGLRC hglrc);
	BOOL WINAPI SwapBuffers_Detour(HDC hdc);
}