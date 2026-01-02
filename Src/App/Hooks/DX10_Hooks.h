#pragma once
#include "Main.h"

typedef HRESULT(__stdcall* tDX10_Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef void(__stdcall* tDX10_DrawIndexed)(ID3D10Device* pDevice, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);

namespace ProvidedDetours
{
	HRESULT __stdcall Present_Detour(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	void __stdcall DrawIndexed_Detour(ID3D10Device* pDevice, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
}