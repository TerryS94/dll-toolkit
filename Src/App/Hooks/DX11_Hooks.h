#pragma once
#include "App/App.h"

typedef HRESULT(__stdcall* tDX11_Present)(_In_ IDXGISwapChain* pSwapChain, _In_ UINT SyncInterval, _In_ UINT Flags);
typedef HRESULT(__stdcall* tDX11_ResizeBuffers)(IDXGISwapChain* pSC, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

namespace ProvidedDetours
{
	HRESULT __stdcall Present_Detour(_In_ IDXGISwapChain* pSwapChain, _In_ UINT SyncInterval, _In_ UINT Flags);
	HRESULT __stdcall ResizeBuffers_Detour(IDXGISwapChain* pSC, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
}