#include "DX10_Hooks.h"

namespace ProvidedDetours
{
    static bool firstEverInit = false;
	HRESULT __stdcall Present_Detour(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
        if (!firstEverInit)
        {
            if (SUCCEEDED(app.dxSwapChain->GetDevice(__uuidof(app.dxDevice), (void**)&app.dxDevice)))
            {
                firstEverInit = true;
                app.UpdateDirectXDeviceVTable();
                DXGI_SWAP_CHAIN_DESC desc;
                app.dxSwapChain->GetDesc(&desc);
                app.Update_HWND(desc.OutputWindow);
                ID3D10Texture2D* pBackBuffer = nullptr;
                app.dxSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer);
                if (pBackBuffer)
                {
                    app.dxDevice->CreateRenderTargetView(pBackBuffer, NULL, &app.dxMainRenderTargetView);
                    pBackBuffer->Release();
                }
            }
        }
        else
        {
            MainRender();//replace with your
        }

		HRESULT result = app.GetOriginalFunction<tDX10_Present>("Present")(pSwapChain, SyncInterval, Flags);
		return result;
	}
	void __stdcall DrawIndexed_Detour(ID3D10Device* pDevice, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
	{
		app.GetOriginalFunction<tDX10_DrawIndexed>("DrawIndexed")(pDevice, IndexCount, StartIndexLocation, BaseVertexLocation);
	}
}