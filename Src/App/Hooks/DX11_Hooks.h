//This file is part of the dll-tookit by TerryS94 -> https://github.com/TerryS94/dll-toolkit

#pragma once
#include "App/App.h"

typedef HRESULT(__stdcall* tDX11_Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT(__stdcall* tDX11_ResizeBuffers)(IDXGISwapChain* pSC, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

namespace ProvidedDetours
{
	HRESULT __stdcall Present_Detour(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	HRESULT __stdcall ResizeBuffers_Detour(IDXGISwapChain* pSC, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
}