//This file is part of the dll-tookit by TerryS94 -> https://github.com/TerryS94/dll-toolkit

#include "DX9_Hooks.h"

namespace ProvidedDetours
{
	HRESULT __stdcall Reset_Detour(IDirect3DDevice9* pD3D9, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		if (pD3D9 != app.dxDevice) return app.GetOriginalFunction<tDX9_Reset>("Reset")(pD3D9, pPresentationParameters);

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
	HRESULT __stdcall EndScene_Detour(IDirect3DDevice9* pD3D9)
	{
		app.Call_UserRenderFunction();
		HRESULT result = app.GetOriginalFunction<tDX9_EndScene>("EndScene")(pD3D9);
		return result;
	}
	HRESULT __stdcall Present_Detour(IDirect3DSwapChain9* swapChain, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags)
	{
		//optionally, you can hide all your imgui rendering from game capturing software with this function.
		//Example: if some condition is false, you could render your imgui in EndScene.. otherwise you could render in here undetected.
		//Tip: you do not need to hot reload imgui and all your resources etc. itll just work when toggling back and forth between EndScene and Present :)
		//Side note: doesn't work when the capture software hooks Present (like discord for example) but works for Medal.tv and OBS.
		//i have not tested other capture software but i suspect itll work for most other capture software.

		//app.Call_UserRenderFunction();

		HRESULT result = app.GetOriginalFunction<tDX9_Present>("Present")(swapChain, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
		return result;
	}
	HRESULT __stdcall DrawIndexedPrimitive_Detour(IDirect3DDevice9* pD3D9, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
	{
		HRESULT result = app.GetOriginalFunction<tDX9_DrawIndexedPrimitive>("DrawIndexedPrimitive")(pD3D9, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
		return result;
	}
}