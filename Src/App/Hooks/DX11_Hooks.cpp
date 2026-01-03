#include "DX11_Hooks.h"

namespace ProvidedDetours
{
	HRESULT __stdcall Present_Detour(_In_ IDXGISwapChain* pSwapChain, _In_ UINT SyncInterval, _In_ UINT Flags)
	{
		HRESULT result = app.GetOriginalFunction<tDX11_Present>("Present")(pSwapChain, SyncInterval, Flags);
		return result;
	}
	void __stdcall DrawIndexedPrimitive_Detour(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
	{
		app.GetOriginalFunction<tDX11_DrawIndexedPrimitive>("DrawIndexedPrimitive")(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
	}
}