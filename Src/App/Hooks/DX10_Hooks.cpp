//This file is part of the dll-tookit by TerryS94 -> https://github.com/TerryS94/dll-toolkit

#include "DX10_Hooks.h"

namespace ProvidedDetours
{
    static bool firstEverInit = false;
	HRESULT __stdcall Present_Detour(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
        if (!firstEverInit)
        {
            if (app.dxDevice)
            {
                firstEverInit = true;
                DXGI_SWAP_CHAIN_DESC desc;
                pSwapChain->GetDesc(&desc);
                app.Update_HWND(desc.OutputWindow);
                ID3D10Texture2D* pBackBuffer = nullptr;
                pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer);
                if (pBackBuffer)
                {
                    app.dxDevice->CreateRenderTargetView(pBackBuffer, NULL, &app.dxMainRenderTargetView);
                    pBackBuffer->Release();
                }
            }
        }
        else
        {
            app.Call_UserRenderFunction();
        }

		HRESULT result = app.GetOriginalFunction<tDX10_Present>("Present")(pSwapChain, SyncInterval, Flags);
		return result;
	}
    HRESULT __stdcall ResizeBuffers_Detour(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
    {
        if (app.dxMainRenderTargetView) { app.dxMainRenderTargetView->Release(); app.dxMainRenderTargetView = nullptr; }
        HRESULT hr = app.GetOriginalFunction<tDX10_ResizeBuffers>("ResizeBuffers")(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        if (SUCCEEDED(hr))
        {
            ID3D10Texture2D* pBackBuffer = nullptr;
            if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&pBackBuffer)) && pBackBuffer)
            {
                app.dxDevice->CreateRenderTargetView(pBackBuffer, NULL, &app.dxMainRenderTargetView);
                pBackBuffer->Release();
            }
            DXGI_SWAP_CHAIN_DESC desc {};
            if (SUCCEEDED(pSwapChain->GetDesc(&desc))) app.Update_HWND(desc.OutputWindow);
        }
        return hr;
    }
}