#pragma once
#include "Main.h"

typedef HRESULT(__stdcall* tDX9_Reset)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
typedef HRESULT(__stdcall* tDX9_BeginScene) (LPDIRECT3DDEVICE9);
typedef HRESULT(__stdcall* tDX9_EndScene) (LPDIRECT3DDEVICE9);
typedef HRESULT(__stdcall* tDX9_Present)(IDirect3DSwapChain9* swapChain, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags);
typedef HRESULT(__stdcall* tDX9_DrawIndexedPrimitive)(LPDIRECT3DDEVICE9, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);

namespace ProvidedDetours
{
	HRESULT __stdcall Reset_Detour(LPDIRECT3DDEVICE9 pD3D9, D3DPRESENT_PARAMETERS* pPresentationParameters);
	HRESULT __stdcall BeginScene_Detour(IDirect3DDevice9* pDevice);
	HRESULT __stdcall EndScene_Detour(LPDIRECT3DDEVICE9 pD3D9);
	HRESULT __stdcall Present_Detour(MAYBEUNUSED IDirect3DSwapChain9* swapChain, MAYBEUNUSED const RECT* pSourceRect, MAYBEUNUSED const RECT* pDestRect, MAYBEUNUSED HWND hDestWindowOverride, MAYBEUNUSED const RGNDATA* pDirtyRegion, MAYBEUNUSED DWORD dwFlags) noexcept;
	HRESULT __stdcall DrawIndexedPrimitive_Detour(LPDIRECT3DDEVICE9 pD3D9, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
}