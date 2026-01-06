#include "DX11_Hooks.h"

namespace ProvidedDetours
{
    static bool firstEverInit = false;
	HRESULT __stdcall Present_Detour(_In_ IDXGISwapChain* pSwapChain, _In_ UINT SyncInterval, _In_ UINT Flags)
	{
        if (!firstEverInit)
        {
            //if user supplied the device pointer up front (for hooking device specific vtable functions) then use that value.
            //otherwise, derive it from swapchain :)
            if (app.dxDevice || SUCCEEDED(pSwapChain->GetDevice(__uuidof(app.dxDevice), (void**)&app.dxDevice)))
            {
                firstEverInit = true;
                app.UpdateDirectXDeviceVTable();
                //if the user didn't supply context on inject (for hooking context vtable function for example), then finally set it here.
                if (!app.dxContext) app.dxDevice->GetImmediateContext(&app.dxContext);
                app.UpdateDirectXContextVTable();
                DXGI_SWAP_CHAIN_DESC desc;
                pSwapChain->GetDesc(&desc);
                app.Update_HWND(desc.OutputWindow);
                ID3D11Texture2D* pBackBuffer = nullptr;
                pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
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

		HRESULT result = app.GetOriginalFunction<tDX11_Present>("Present")(pSwapChain, SyncInterval, Flags);
		return result;
	}
    HRESULT __stdcall ResizeBuffers_Detour(IDXGISwapChain* pSC, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
    {
        if (!pSC) return E_INVALIDARG;
        if (app.dxContext) { ID3D11RenderTargetView* nullRTV[1] = { nullptr }; app.dxContext->OMSetRenderTargets(1, nullRTV, nullptr); app.dxContext->Flush(); }
        if (app.dxMainRenderTargetView) { app.dxMainRenderTargetView->Release(); app.dxMainRenderTargetView = nullptr; }
        HRESULT hr = app.GetOriginalFunction<tDX11_ResizeBuffers>("ResizeBuffers")(pSC, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        if (FAILED(hr)) return hr;
        DXGI_SWAP_CHAIN_DESC desc{};
        if (SUCCEEDED(pSC->GetDesc(&desc))) app.Update_HWND(desc.OutputWindow);
        if (!app.dxDevice) pSC->GetDevice(__uuidof(app.dxDevice), (void**)&app.dxDevice);
        ID3D11Texture2D* pBack = nullptr;
        hr = pSC->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBack);
        if (FAILED(hr) || !pBack) return hr;
        hr = app.dxDevice->CreateRenderTargetView(pBack, nullptr, &app.dxMainRenderTargetView);
        pBack->Release();
        if (FAILED(hr)) return hr;
        if (app.dxContext) app.dxContext->OMSetRenderTargets(1, &app.dxMainRenderTargetView, nullptr);
        app.UpdateDirectXSwapChain(pSC);
        return S_OK;
    }
}