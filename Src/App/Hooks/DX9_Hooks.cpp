#include "DX9_Hooks.h"

namespace ProvidedDetours
{
	HRESULT __stdcall Reset_Detour(LPDIRECT3DDEVICE9 pD3D9, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		if (pD3D9 != app.GetDirectXDevice()) return app.GetOriginalFunction<tDX9_Reset>("Reset")(pD3D9, pPresentationParameters);

		app.FreeResources();
		if (app.HasInitializedFirstTime()) app.ImGui_InvalidateDeviceObjects();

		HRESULT result = app.GetOriginalFunction<tDX9_Reset>("Reset")(pD3D9, pPresentationParameters);
		
		if (SUCCEEDED(result) && app.HasInitializedFirstTime())
		{
			app.Call_UserInitResources();
			app.ImGui_CreateDeviceObjects();
		}
		else
		{
			if (FAILED(result))
			{
				MessageBox(nullptr, "Problem restoring DirectX Device.", "DX Reset Error", 0);
				ExitProcess(0);
			}
		}
		return result;
	}
	HRESULT __stdcall BeginScene_Detour(IDirect3DDevice9* pDevice)
	{
		HRESULT result = app.GetOriginalFunction<tDX9_BeginScene>("BeginScene")(pDevice);
		return result;
	}
	HRESULT __stdcall EndScene_Detour(LPDIRECT3DDEVICE9 pD3D9)
	{
		HRESULT result = app.GetOriginalFunction<tDX9_EndScene>("EndScene")(pD3D9);
		return result;
	}
#pragma optimize("", off)
	__declspec(naked) HRESULT __stdcall Present_Detour(MAYBEUNUSED IDirect3DSwapChain9* swapChain, MAYBEUNUSED const RECT* pSourceRect, MAYBEUNUSED const RECT* pDestRect, MAYBEUNUSED HWND hDestWindowOverride, MAYBEUNUSED const RGNDATA* pDirtyRegion, MAYBEUNUSED DWORD dwFlags) noexcept
	{
		__asm sub esp, __LOCAL_SIZE
		tDX9_Present originalPresent;
		originalPresent = app.GetOriginalFunction<tDX9_Present>("Present");

		//optionally, you can hide all your imgui rendering from game capturing software with this function.
		//Example: if some condition is false, you could render your imgui in EndScene.. otherwise you could render in here undetected.
		//Tip: you do not need to hot reload imgui and all your resources etc. itll just work when toggling back and forth between EndScene and Present :)

		__asm add esp, __LOCAL_SIZE
		__asm jmp originalPresent
	}
#pragma optimize("", on)
	HRESULT __stdcall DrawIndexedPrimitive_Detour(LPDIRECT3DDEVICE9 pD3D9, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
	{
		HRESULT result = app.GetOriginalFunction<tDX9_DrawIndexedPrimitive>("DrawIndexedPrimitive")(pD3D9, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
		return result;
	}
}