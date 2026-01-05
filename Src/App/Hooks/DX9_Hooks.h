#pragma once
#include "App/App.h"

typedef HRESULT(__stdcall* tDX9_Reset)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
typedef HRESULT(__stdcall* tDX9_BeginScene) (IDirect3DDevice9*);
typedef HRESULT(__stdcall* tDX9_EndScene) (IDirect3DDevice9*);
typedef HRESULT(__stdcall* tDX9_Present)(IDirect3DSwapChain9* swapChain, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags);
typedef HRESULT(__stdcall* tDX9_DrawIndexedPrimitive)(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);

namespace ProvidedDetours
{
	HRESULT __stdcall Reset_Detour(IDirect3DDevice9* pD3D9, D3DPRESENT_PARAMETERS* pPresentationParameters);
	HRESULT __stdcall BeginScene_Detour(IDirect3DDevice9* pDevice);
	HRESULT __stdcall EndScene_Detour(IDirect3DDevice9* pD3D9);
	HRESULT __stdcall Present_Detour(IDirect3DSwapChain9* swapChain, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags);
	HRESULT __stdcall DrawIndexedPrimitive_Detour(IDirect3DDevice9* pD3D9, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
}