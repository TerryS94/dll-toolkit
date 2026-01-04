#pragma once
#include "App/App.h"

typedef HRESULT(__stdcall* tDX11_Present)(_In_ IDXGISwapChain* pSwapChain, _In_ UINT SyncInterval, _In_ UINT Flags);
typedef void(__stdcall* tDX11_DrawIndexedPrimitive)(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);

namespace ProvidedDetours
{
	HRESULT __stdcall Present_Detour(_In_ IDXGISwapChain* pSwapChain, _In_ UINT SyncInterval, _In_ UINT Flags);
	void WINAPI DrawIndexedPrimitive_Detour(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
}