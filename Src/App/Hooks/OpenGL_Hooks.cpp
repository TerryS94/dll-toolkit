#include "OpenGL_Hooks.h"

namespace ProvidedDetours
{
    BOOL WINAPI wglMakeCurrent_Detour(HDC hdc, HGLRC hglrc)
    {
        BOOL ok = app.GetOriginalFunction<tWglMakeCurrent>("wglMakeCurrent")(hdc, hglrc);
        if (!ok) return ok;
        app.Update_HWND(WindowFromDC(hdc));
        app.Set_ImGui_Reload(true);
        return ok;
    }
	BOOL WINAPI SwapBuffers_Detour(HDC hdc)
	{
        app.Call_UserRenderFunction();
		BOOL result = app.GetOriginalFunction<tOpenGL_SwapBuffers>("SwapBuffers")(hdc);
		return result;
	}
}