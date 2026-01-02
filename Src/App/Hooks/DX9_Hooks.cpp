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
			InitResources();
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
		auto original = app.GetOriginalFunction<tDX9_BeginScene>("BeginScene");
		//Call of Duty 4 specific address. idea behind this is to make sure only the games beginscene ran and not a overlay which can make our-
		//code in here run 2+ times a frame depending how many overlays the user is using.
		if (reinterpret_cast<uintptr_t>(_ReturnAddress()) != 0x61537A)
			return original(pDevice);

		HRESULT result = original(pDevice);
		return result;
	}
	HRESULT __stdcall EndScene_Detour(LPDIRECT3DDEVICE9 pD3D9)
	{
		auto original = app.GetOriginalFunction<tDX9_EndScene>("EndScene");
		//Call of Duty 4 specific address. idea behind this is to make sure only the games endscene ran and not a overlay which can make our-
		//code in here run 2+ times a frame depending how many overlays the user is using.
		if (reinterpret_cast<uintptr_t>(_ReturnAddress()) != 0x615743 && pD3D9)
			return original(pD3D9);

		MainRender();

		HRESULT result = original(pD3D9);
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