#pragma once
#include "App/App.h"

typedef BOOL(__stdcall* tOpenGL_SwapBuffers)(HDC hdc);
typedef BOOL(__stdcall* tWglMakeCurrent)(HDC hdc, HGLRC hglrc);

namespace ProvidedDetours
{
	BOOL WINAPI wglMakeCurrent_Detour(HDC hdc, HGLRC hglrc);
	BOOL WINAPI SwapBuffers_Detour(HDC hdc);
}