#include "DX11_Hooks.h"

namespace ProvidedDetours
{
    static bool firstEverInit = false;
	HRESULT __stdcall Present_Detour(_In_ IDXGISwapChain* pSwapChain, _In_ UINT SyncInterval, _In_ UINT Flags)
	{
        if (!firstEverInit)
        {
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(app.dxDevice), (void**)&app.dxDevice)))
            {
                firstEverInit = true;
                app.UpdateDirectXDeviceVTable();
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
	void __stdcall DrawIndexedPrimitive_Detour(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
	{
		app.GetOriginalFunction<tDX11_DrawIndexedPrimitive>("DrawIndexedPrimitive")(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
	}
}