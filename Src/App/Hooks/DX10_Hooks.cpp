#include "DX10_Hooks.h"

namespace ProvidedDetours
{
	HRESULT __stdcall Present_Detour(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		HRESULT result = app.GetOriginalFunction<tDX10_Present>("Present")(pSwapChain, SyncInterval, Flags);
		return result;
	}
	void __stdcall DrawIndexed_Detour(ID3D10Device* pDevice, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
	{
		app.GetOriginalFunction<tDX10_DrawIndexed>("DrawIndexed")(pDevice, IndexCount, StartIndexLocation, BaseVertexLocation);
	}
}