#include "App.h"

//only utilizing stb lib if its not DX9
#ifndef DirectX9
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_tables.cpp"
#ifdef ImGui_IncludeDemo
#include "imgui_demo.cpp"
#endif
#include "imgui_widgets.cpp"
#include "misc/cpp/imgui_stdlib.cpp"
#include "backends/imgui_impl_win32.cpp"

#ifdef DirectX9
#include "backends/imgui_impl_dx9.cpp"
#include "Hooks/DX9_Hooks.cpp"
#elifdef DirectX10
#include "backends/imgui_impl_dx10.cpp"
#include "Hooks/DX10_Hooks.cpp"
#elifdef DirectX11
#include "backends/imgui_impl_dx11.cpp"
#include "Hooks/DX11_Hooks.cpp"
#elifdef OpenGL2
#include "backends/imgui_impl_opengl2.cpp"
#include "Hooks/OpenGL_Hooks.cpp"
#elifdef OpenGL3
#include "backends/imgui_impl_opengl3.cpp"
#include "Hooks/OpenGL_Hooks.cpp"
#endif

App app;

HookingLayer::HookingLayer() {}
HookingLayer::~HookingLayer() {}
App::App(){}
App::~App(){}

void App::InitRenderer()
{
	ImGui::CreateContext();

#ifdef ImGui_EnableDpiAwareness
	ImGui_ImplWin32_EnableDpiAwareness();
#endif

#ifndef AnyOpenGLActive
	ImGui_ImplWin32_Init(hwnd);
#endif

#ifdef DirectX9
	assert(dxDevice && "device was nullptr!");
	ImGui_ImplDX9_Init(dxDevice);
#elifdef DirectX10
	assert(dxDevice && "device was nullptr!");
	ImGui_ImplDX10_Init(dxDevice);
#elifdef DirectX11
	assert(dxDevice && "device was nullptr!");
	assert(dxSwapChain && "swapChain was nullptr!");
	assert(dxContext && "context was nullptr!");
	ImGui_ImplDX11_Init(dxDevice, dxContext);
#elifdef OpenGL2
	ImGui_ImplWin32_InitForOpenGL(hwnd);
	ImGui_ImplOpenGL2_Init();
#elifdef OpenGL3
	ImGui_ImplWin32_InitForOpenGL(hwnd);
	ImGui_ImplOpenGL3_Init(glsl_version);
#endif

	Override_WndProc();

	Set_RendererActive(true);
	imguiFirstInitDone = true;
}

void App::ReloadRenderer()
{
	if (!imguiFirstInitDone) return;
	Shutdown();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	InitRenderer();
}

void App::Reset_WndProc() const
{
#ifdef BUILD_x86
	SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)original_WndProc);
#elifdef BUILD_x64
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)original_WndProc);
#endif
}

void App::Override_WndProc()
{
#ifdef BUILD_x86
	original_WndProc = (WNDPROC)SetWindowLongPtr(this->hwnd, GWL_WNDPROC, (LONG_PTR)ProvidedDetours::WndProc_Detour);
#elifdef BUILD_x64
	original_WndProc = (WNDPROC)SetWindowLongPtr(this->hwnd, GWLP_WNDPROC, (LONG_PTR)ProvidedDetours::WndProc_Detour);
#endif
}

void App::Shutdown()
{
	Reset_WndProc();

	//for this to run, call Start_Eject() as soon as you flag your dll for eject
	if (IsEjecting())
	{
		UninstallPatches();
		UninstallHooks();
	}

	FreeResources();

#ifdef DirectX9
	ImGui_ImplDX9_Shutdown();
#elifdef DirectX10
	ImGui_ImplDX10_Shutdown();
#elifdef DirectX11
	ImGui_ImplDX11_Shutdown();
#elifdef OpenGL2
	ImGui_ImplOpenGL2_Shutdown();
#elifdef OpenGL3
	ImGui_ImplOpenGL3_Shutdown();
#endif

	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void App::BeginFrame(bool want_mouse_this_frame)
{
	this->hasMouseCursor = want_mouse_this_frame;
	static bool lastMouseState = want_mouse_this_frame;
	if (!want_mouse_this_frame && lastMouseState)
		MouseStateSnapshot_OnMenuClosed();
	else if (want_mouse_this_frame && !lastMouseState)
		MouseStateSnapshot_OnMenuOpened();

	lastMouseState = want_mouse_this_frame;

	ImGuiIO& io = ImGui::GetIO();
	io.MouseDrawCursor = want_mouse_this_frame;
	io.ConfigFlags = ImGuiConfigFlags_None;
	//if a cursor is not explicitly given to us, completely drop mouse support just in case the user is rendering any HUD then they can't
	//accidentally click and drag to move stuff around even though they never intended to.
	if (!want_mouse_this_frame) io.ConfigFlags |= ImGuiConfigFlags_NoMouse;

	Update_IsTargetWindowFocused();

#ifdef DirectX9
	ImGui_ImplDX9_NewFrame();
#elifdef DirectX10
	ImGui_ImplDX10_NewFrame();
#elifdef DirectX11
	ImGui_ImplDX11_NewFrame();
#elifdef OpenGL2
	ImGui_ImplOpenGL2_NewFrame();
#elifdef OpenGL3
	ImGui_ImplOpenGL3_NewFrame();
#endif

	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void App::EndFrame() const
{
	ImGui::EndFrame();
	ImGui::Render();

#ifdef DirectX9
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
#elifdef DirectX10
	ImGui_ImplDX10_RenderDrawData(ImGui::GetDrawData());
#elifdef DirectX11
	dxContext->OMSetRenderTargets(1, &dxMainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#elifdef OpenGL2
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
#elifdef OpenGL3
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

void App::ImGui_InvalidateDeviceObjects() const
{
#ifdef DirectX9
	ImGui_ImplDX9_InvalidateDeviceObjects();
#elifdef DirectX10
	ImGui_ImplDX10_InvalidateDeviceObjects();
#elifdef DirectX11
	ImGui_ImplDX11_InvalidateDeviceObjects();
#elifdef OpenGL2
	ImGui_ImplOpenGL2_DestroyDeviceObjects();
#elifdef OpenGL3
	ImGui_ImplOpenGL3_DestroyDeviceObjects();
#endif
}

void App::ImGui_CreateDeviceObjects() const
{
#ifdef DirectX9
	ImGui_ImplDX9_CreateDeviceObjects();
#elifdef DirectX10
	ImGui_ImplDX10_CreateDeviceObjects();
#elifdef DirectX11
	ImGui_ImplDX11_CreateDeviceObjects();
#elifdef OpenGL2
	ImGui_ImplOpenGL2_CreateDeviceObjects();
#elifdef OpenGL3
	ImGui_ImplOpenGL3_CreateDeviceObjects();
#endif
}

void App::FreeResources()
{
	FreeTextures();
	FreeFonts();
}

void App::RegisterUniversalHooks()
{
	RegisterHook("CreateWindowExA",     (uint64_t)CreateWindowExA_Addr,     (uint64_t)ProvidedDetours::CreateWindowExA_Detour);
	RegisterHook("NtUserDestroyWindow", (uint64_t)NtUserDestroyWindow_Addr, (uint64_t)ProvidedDetours::NtUserDestroyWindow_Detour);
	RegisterHook("GetCursorPos",        (uint64_t)GetCursorPos_Addr,        (uint64_t)ProvidedDetours::GetCursorPos_Detour);
	RegisterHook("SetCursorPos",        (uint64_t)SetCursorPos_Addr,        (uint64_t)ProvidedDetours::SetCursorPos_Detour);
	RegisterHook("ClipCursor",          (uint64_t)ClipCursor_Addr,          (uint64_t)ProvidedDetours::ClipCursor_Detour);
	RegisterHook("NtUserSetCursorPos",  (uint64_t)NtUserSetCursorPos_Addr,  (uint64_t)ProvidedDetours::NtUserSetCursorPos_Detour);
	RegisterHook("DeviceIoControl",     (uint64_t)DeviceIoControl_Addr,     (uint64_t)ProvidedDetours::DeviceIoControl_Detour);
	RegisterHook("ScreenToClient",      (uint64_t)ScreenToClient_Addr,      (uint64_t)ProvidedDetours::ScreenToClient_Detour);
}

void App::RegisterBackEndHooks()
{
#ifdef DirectX9
	app.RegisterHook("BeginScene",           (uint64_t)DX9_BeginScene_Addr,           (uint64_t)ProvidedDetours::BeginScene_Detour);
	app.RegisterHook("EndScene",             (uint64_t)DX9_EndScene_Addr,             (uint64_t)ProvidedDetours::EndScene_Detour);
	app.RegisterHook("DrawIndexedPrimitive", (uint64_t)DX9_DrawIndexedPrimitive_Addr, (uint64_t)ProvidedDetours::DrawIndexedPrimitive_Detour);
	app.RegisterHook("Reset",                (uint64_t)DX9_Reset_Addr,                (uint64_t)ProvidedDetours::Reset_Detour);
#elifdef DirectX10
	//todo
#elifdef DirectX11
	app.RegisterHook("Present",              (uint64_t)DX11_Present_Addr,             (uint64_t)ProvidedDetours::Present_Detour);
#elifdef AnyOpenGLActive
	app.RegisterHook("wglMakeCurrent",       (uint64_t)OpenGL_wglMakeCurrent_Addr,    (uint64_t)ProvidedDetours::wglMakeCurrent_Detour);
	app.RegisterHook("SwapBuffers",          (uint64_t)OpenGL_SwapBuffers_Addr,       (uint64_t)ProvidedDetours::SwapBuffers_Detour);
#endif
}

void App::FreeTextures()
{
	textures.clear();
}

void App::FreeFonts()
{
	assert(ImGui::GetCurrentContext() && "ImGui::GetCurrentContext() is invalid!");
	assert(ImGui::GetIO().Fonts && "ImGui::GetIO().Fonts is invalid!");

	ImGui::GetIO().Fonts->Clear();
	fonts.clear();
}

//checks if 'key1' is held while 'key2' is pressed only.
//if a 3rd key is passed in, then it checks if both 'key1' and 'key2' are held while 'key3' is pressed only.
bool App::IsKeyChordPressed(AppKeys key1, AppKeys key2, AppKeys key3) const
{
	if (!IsKeyDown(key1)) return false;
	const bool hasKey3 = (key3 != AppKeys::INVALID);
	if (hasKey3 && !IsKeyDown(key2)) return false;
	return IsKeyPressed(hasKey3 ? key3 : key2);
}

bool App::IsKeyPressed(AppKeys key_code) const
{
	if (!IsTargetWindowFocused()) return false;
	const int k = static_cast<int>(key_code);
	if (k < 0 || k > 255) return false;
	static std::atomic<bool> prev_state[256] = {};
	const bool curr_state = (GetAsyncKeyState(k) & 0x8000) != 0;
	const bool prev = prev_state[k].exchange(curr_state, std::memory_order_relaxed);
	return curr_state && !prev;
}

bool App::IsKeyDown(AppKeys key_code) const
{
	if (!IsTargetWindowFocused()) return false;
	const int k = static_cast<int>(key_code);
	if (k < 0 || k > 255) return false;
	return (GetAsyncKeyState(k) & 0x8000) != 0;
}

void App::Update_IsTargetWindowFocused()
{
	const HWND fgWin = GetForegroundWindow();
	const HWND target = this->hwnd;
	this->isTargetWindowFocused = fgWin && target && fgWin == target;
}

void App::AddFontFromFile(const std::string_view& fontName, const std::string_view& path, float initialFontSize)
{
	assert(imguiFirstInitDone && "Must be called after InitRenderer!");
	if (fonts.contains(fontName))
	{
		MessageBox(nullptr, std::format("font \"{}\" already exists... Skipping", fontName).c_str(), __FUNCSIG__, MB_OK);
		return;
	}
	if (!std::filesystem::exists(path))
	{
		MessageBox(nullptr, std::format("font file not found: \"{}\"", path).c_str(), __FUNCSIG__, MB_OK);
		return;
	}
	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig fontConfig {};
	fontConfig.PixelSnapH = true;
	fontConfig.OversampleH = 2;
	ImFont* font = io.Fonts->AddFontFromFileTTF(path.data(), initialFontSize, &fontConfig);
	if (!font)
	{
		MessageBox(nullptr, std::format("AddFontFromFileTTF failed for: \"{}\"", path).c_str(), __FUNCSIG__, MB_OK);
		return;
	}
	fonts[fontName] = font;
}

void App::AddFontFromMemory(const std::string_view& fontName, const void* fontData, int data_size, float initialFontSize)
{
	assert(imguiFirstInitDone && "Must be called after InitRenderer!");
	if (fonts.contains(fontName))
	{
		MessageBox(nullptr, std::format("font \"{}\" already exists... Skipping", fontName).c_str(), __FUNCSIG__, MB_OK);
		return;
	}
	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig fontConfig = ImFontConfig();
	fontConfig.FontDataOwnedByAtlas = false;
	fontConfig.PixelSnapH = true;
	fontConfig.OversampleH = 2;
	fonts[fontName] = io.Fonts->AddFontFromMemoryCompressedTTF(fontData, data_size, initialFontSize, &fontConfig);
}

ImFont* App::GetFontByName(const std::string_view& fontName)
{
	auto it = fonts.find(fontName);
	if (it != fonts.end())
		return it->second;
	return nullptr;
}

CustomTexture* App::GetTextureByName(const std::string_view& textureName)
{
	auto it = textures.find(textureName);
	return it == textures.end() ? nullptr : it->second.get();
}

void App::AddTextureFromFile(const std::string_view& name, const std::string_view& path)
{
	assert(imguiFirstInitDone && "Must be called after InitRenderer!");
	if (!std::filesystem::exists(path)) { MessageBox(nullptr, std::format("\"{}\" doesn't exist!", path).c_str(), "F", MB_OK); return; }
	auto tex = std::make_unique<CustomTexture>();
#ifdef DirectX9
	tex->ptr = DX9_LoadTextureFromFile(path.data());
	if (!tex->ptr) { MessageBox(nullptr, std::format("DX9: failed to load texture \"{}\"", path).c_str(), "F", MB_OK); return; }
#elifdef DirectX10
	tex->ptr = DX10_LoadTextureFromFile(path.data());
	if (!tex->ptr) { MessageBox(nullptr, std::format("DX10: failed to load texture \"{}\"", path).data(), "F", MB_OK); return; }
#elifdef DirectX11
	tex->ptr = DX11_LoadTextureFromFile(path.data());
	if (!tex->ptr) { MessageBox(nullptr, std::format("DX11: failed to load texture \"{}\"", path).data(), "F", MB_OK); return; }
//#elif defined(DirectX12)
//		tex->ptr = DX12_LoadTextureFromFile(path.data());
//		if (!tex->ptr) { MessageBox(nullptr, std::format("DX12: failed to load texture \"{}\"", path).data(), "F", MB_OK); return; }
#elifdef AnyOpenGLActive
	tex->texture = GL_LoadTextureFromFile(path.data());
	if (!tex->texture.id) { MessageBox(nullptr, std::format("GL: failed to load texture \"{}\"", path).data(), "F", MB_OK); return; }
#endif

	textures[name] = std::move(tex);
}

void App::AddTextureFromMemory(const std::string_view& name, void* data, const size_t data_size)
{
	assert(imguiFirstInitDone && "Must be called after InitRenderer!");
	if (!data || data_size == 0)
	{
		MessageBox(nullptr, std::format("Data was invalid for \"{}\"", name).c_str(), "F", MB_OK);
		return;
	}

	auto tex = std::make_unique<CustomTexture>();

#ifdef DirectX9
	tex->ptr = DX9_LoadTextureFromMemory(data, data_size);
	if (!tex->ptr) { MessageBox(nullptr, std::format("DX9: failed to load memory texture \"{}\"", name).c_str(), "F", MB_OK); return; }
#elifdef DirectX10
	tex->ptr = DX10_LoadTextureFromMemory(data, data_size);
	if (!tex->ptr) { MessageBox(nullptr, std::format("DX10: failed to load memory texture \"{}\"", name).c_str(), "F", MB_OK); return; }
#elifdef DirectX11
	tex->ptr = DX11_LoadTextureFromMemory(data, data_size);
	if (!tex->ptr) { MessageBox(nullptr, std::format("DX11: failed to load memory texture \"{}\"", name).c_str(), "F", MB_OK); return; }
//#elifdef DirectX12
//		tex->ptr = DX12_LoadTextureFromMemory(data, data_size);
//		if (!tex->ptr) { MessageBox(nullptr, std::format("DX12: failed to load memory texture \"{}\"", name).c_str(), "F", MB_OK); return; }
#elifdef AnyOpenGLActive
	tex->texture = GL_LoadTextureFromMemory(data, data_size);
	if (!tex->texture.id) { MessageBox(nullptr, std::format("GL: failed to load memory texture \"{}\"", name).c_str(), "F", MB_OK); return; }
#endif

	textures[name] = std::move(tex);
}

#ifdef AnyOpenGLActive
bool App::GL_CreateTextureRGBA8(const unsigned char* rgba, int w, int h, GLTexture& out)
{
	if (!rgba || w <= 0 || h <= 0) return false;
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#ifdef OpenGL3
	//GL3+ prefers explicit sized internal formats.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba); 
#else
	//GL2 typically uses GL_RGBA as internal format.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
#endif

	out.id = tex;
	out.width = w;
	out.height = h;
	return true;
}
void App::GL_DestroyTexture(GLTexture& t)
{
	if (t.id)
	{
		glDeleteTextures(1, &t.id);
		t.id = 0;
		t.width = t.height = 0;
	}
}
#endif

#ifdef AnyDirectXActive
void App::UpdateDirectXDeviceVTable()
{
	if (!dxDevice)
	{
		MessageBox(nullptr, "Device invalid!", "Called from UpdateDirectXDeviceVTable", MB_OK);
		return;
	}
	dxDeviceVTable = *reinterpret_cast<void***>(dxDevice);
	UpdateDirectXSwapChainVTable();
}
void App::UpdateDirectXSwapChainVTable()
{
#ifdef DirectX9
	if (!dxDevice)
	{
		MessageBox(nullptr, "DX9 device is null!\n", "Called from UpdateDirectXSwapChainVTable", MB_OK | MB_ICONERROR);
		ExitProcess(EXIT_FAILURE);
	}

	IDirect3DSwapChain9* temp = nullptr;
	HRESULT hr = dxDevice->GetSwapChain(0, &temp);
	if (FAILED(hr) || !temp)
	{
		MessageBox(nullptr, "Couldn't obtain the DirectX9 SwapChain!\n", "Called from UpdateDirectXSwapChainVTable", MB_OK | MB_ICONERROR);
		ExitProcess(EXIT_FAILURE);
	}
	dxSwapChainVTable = *reinterpret_cast<void***>(temp);
	temp->Release();
#elif defined DirectX10 || defined DirectX11
	if (dxSwapChain) dxSwapChainVTable = *reinterpret_cast<void***>(dxSwapChain);
#endif
}
#endif

#ifdef DirectX9
void App::UpdateDirectXDevice(IDirect3DDevice9* device)
{
	IDirect3DDevice9* newDev = device;
	if (newDev == dxDevice) return;
	if (newDev) newDev->AddRef();
	if (dxDevice) dxDevice->Release();
	dxDevice = newDev;
	UpdateDirectXDeviceVTable();
	UpdateDirectXSwapChainVTable();
}
#elifdef DirectX10
void App::UpdateDirectXSwapChain(IDXGISwapChain* swapChain)
{
	IDXGISwapChain* newSwap = swapChain;
	if (newSwap == dxSwapChain) return;
	if (newSwap) newSwap->AddRef();
	if (dxSwapChain) dxSwapChain->Release();
	dxSwapChain = newSwap;
	UpdateDirectXSwapChainVTable();
}
#elifdef DirectX11
void App::UpdateDirectXContextVTable()
{
	reinterpret_cast<ID3D11Device*>(dxDevice)->GetImmediateContext(&dxContext);
	if (dxContext)
		dxContextVTable = *reinterpret_cast<void***>(dxContext);
	else
		MessageBox(nullptr, "Couldn't obtain the DirectX11 immediate context", "DX Error", MB_OK | MB_ICONERROR);
}
void App::UpdateDirectXSwapChain(IDXGISwapChain* swapChain)
{
	IDXGISwapChain* newSwap = swapChain;
	if (newSwap == dxSwapChain) return;
	if (newSwap) newSwap->AddRef();
	if (dxSwapChain) dxSwapChain->Release();
	dxSwapChain = newSwap;
	UpdateDirectXSwapChainVTable();
}
#endif

	
#if defined DirectX9 || defined DirectX10 || defined DirectX11
void* App::GetDirectXDeviceMethodByIndex(int index) const
{
	return dxDeviceVTable[index];
}
void* App::GetDirectXSwapChainMethodByIndex(int index) const
{
	return dxSwapChainVTable[index];
}
#if defined(DirectX11)
void* App::GetDirectXContextMethodByIndex(int index) const
{
	return dxContextVTable[index];
}
#endif
#endif

#ifdef DirectX9
PDIRECT3DTEXTURE9 App::DX9_LoadTextureFromFile(const char* filename)
{
	PDIRECT3DTEXTURE9 baseTexture = nullptr;
	HRESULT hr = D3DXCreateTextureFromFileExA(reinterpret_cast<LPDIRECT3DDEVICE9>(dxDevice), filename, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, nullptr, nullptr, &baseTexture);
	if (FAILED(hr) || !baseTexture)
	{
		MessageBox(nullptr, std::format("Loading base texture failed for \"{}\".", filename).c_str(), "DX9_LoadTextureFromFile fail", MB_OK);
		return nullptr;
	}
	return baseTexture;
}
PDIRECT3DTEXTURE9 App::DX9_LoadTextureFromMemory(void* data, size_t size)
{
	PDIRECT3DTEXTURE9 texture = nullptr;
	HRESULT result = D3DXCreateTextureFromFileInMemoryEx(reinterpret_cast<LPDIRECT3DDEVICE9>(dxDevice), data, size, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, nullptr, nullptr, &texture);
	if (result != S_OK)
		return nullptr;
	return texture;
}
#elifdef DirectX10
ID3D10ShaderResourceView* App::DX10_LoadTextureFromFile(const char* filename)
{
	if (!filename || !filename[0]) return nullptr;
	int width = 0, height = 0, channels = 0;
	unsigned char* pixels = stbi_load(filename, &width, &height, &channels, 4);
	if (!pixels) return nullptr;
	ID3D10Device* dev = reinterpret_cast<ID3D10Device*>(dxDevice);
	D3D10_TEXTURE2D_DESC desc = {};
	desc.Width = (UINT)width;
	desc.Height = (UINT)height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D10_USAGE_DEFAULT;
	desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	D3D10_SUBRESOURCE_DATA init = {};
	init.pSysMem = pixels;
	init.SysMemPitch = (UINT)(width * 4);
	init.SysMemSlicePitch = 0;
	ID3D10Texture2D* tex = nullptr;
	HRESULT hr = dev->CreateTexture2D(&desc, &init, &tex);
	stbi_image_free(pixels);
	if (FAILED(hr) || !tex) return nullptr;
	ID3D10ShaderResourceView* srv = nullptr;
	hr = dev->CreateShaderResourceView(tex, nullptr, &srv);
	tex->Release();
	if (FAILED(hr) || !srv) return nullptr;
	return srv;
}
ID3D10ShaderResourceView* App::DX10_LoadTextureFromMemory(void* data, size_t size)
{
	if (!data || size == 0) return nullptr;
	int width = 0, height = 0, channels = 0;
	unsigned char* pixels = stbi_load_from_memory((const unsigned char*)data, (int)size, &width, &height, &channels, 4);
	if (!pixels) return nullptr;
	ID3D10Device* dev = reinterpret_cast<ID3D10Device*>(dxDevice);
	D3D10_TEXTURE2D_DESC desc = {};
	desc.Width = (UINT)width;
	desc.Height = (UINT)height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D10_USAGE_DEFAULT;
	desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	D3D10_SUBRESOURCE_DATA init = {};
	init.pSysMem = pixels;
	init.SysMemPitch = (UINT)(width * 4);
	init.SysMemSlicePitch = 0;
	ID3D10Texture2D* tex = nullptr;
	HRESULT hr = dev->CreateTexture2D(&desc, &init, &tex);
	stbi_image_free(pixels);
	if (FAILED(hr) || !tex) return nullptr;
	ID3D10ShaderResourceView* srv = nullptr;
	hr = dev->CreateShaderResourceView(tex, nullptr, &srv);
	tex->Release();
	if (FAILED(hr) || !srv) return nullptr;
	return srv;
}
#elifdef DirectX11
ID3D11ShaderResourceView* App::DX11_LoadTextureFromFile(const char* filename)
{
	if (!filename || !filename[0]) return nullptr;
	int width = 0;
	int height = 0;
	int channels = 0;
	unsigned char* pixels = stbi_load(filename, &width, &height, &channels, 4);
	if (!pixels) return nullptr;
	D3D11_TEXTURE2D_DESC texDesc {};
	texDesc.Width = (UINT)width;
	texDesc.Height = (UINT)height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA sub {};
	sub.pSysMem = pixels;
	sub.SysMemPitch = (UINT)(width * 4);
	ID3D11Texture2D* tex = nullptr;
	ID3D11Device* dev = reinterpret_cast<ID3D11Device*>(dxDevice);
	HRESULT hr = dev->CreateTexture2D(&texDesc, &sub, &tex);
	stbi_image_free(pixels);
	if (FAILED(hr) || !tex) return nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	hr = dev->CreateShaderResourceView(tex, nullptr, &srv);
	tex->Release();
	if (FAILED(hr) || !srv) return nullptr;
	return srv;
}
ID3D11ShaderResourceView* App::DX11_LoadTextureFromMemory(void* data, size_t size)
{
	if (!data || size == 0) return nullptr;
	int width = 0;
	int height = 0;
	int channels = 0;
	unsigned char* pixels = stbi_load_from_memory((const unsigned char*)data, (int)size, &width, &height, &channels, 4);
	if (!pixels) return nullptr;
	D3D11_TEXTURE2D_DESC texDesc {};
	texDesc.Width = (UINT)width;
	texDesc.Height = (UINT)height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA sub {};
	sub.pSysMem = pixels;
	sub.SysMemPitch = (UINT)(width * 4);
	ID3D11Texture2D* tex = nullptr;
	HRESULT hr = reinterpret_cast<ID3D11Device*>(dxDevice)->CreateTexture2D(&texDesc, &sub, &tex);
	stbi_image_free(pixels);
	if (FAILED(hr) || !tex) return nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	hr = reinterpret_cast<ID3D11Device*>(dxDevice)->CreateShaderResourceView(tex, nullptr, &srv);
	tex->Release();
	if (FAILED(hr) || !srv) return nullptr;
	return srv;
}
//#elifdef DirectX12
//ID3D12Resource* App::DX12_LoadTextureFromFile(const char* filename)
//{
//	if (!filename || !filename[0]) return nullptr;
//	int width = 0, height = 0, channels = 0;
//	unsigned char* pixels = stbi_load(filename, &width, &height, &channels, 4);
//	if (!pixels) return nullptr;
//	ID3D12Resource* tex = DX12_LoadTextureFromMemory(pixels, (size_t)width * (size_t)height * 4, width, height);
//	stbi_image_free(pixels);
//	return tex;
//}
//ID3D12Resource* App::DX12_LoadTextureFromMemory(void* data, size_t size, int knownWidth, int knownHeight) const
//{
//	if (!data || size == 0) return nullptr;
//	int width = knownWidth;
//	int height = knownHeight;
//	int channels = 0;
//	unsigned char* pixels = nullptr;
//	bool pixels_owned = false;
//	if (width == 0 || height == 0)
//	{
//		pixels = stbi_load_from_memory((const unsigned char*)data, (int)size, &width, &height, &channels, 4);
//		if (!pixels) return nullptr;
//		pixels_owned = true;
//	}
//	else pixels = reinterpret_cast<unsigned char*>(data);
//
//	ID3D12Device* dev = reinterpret_cast<ID3D12Device*>(device);
//	if (!dev)
//	{
//		if (pixels_owned) stbi_image_free(pixels);
//		return nullptr;
//	}
//
//	D3D12_RESOURCE_DESC desc = {};
//	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//	desc.Width = (UINT)width;
//	desc.Height = (UINT)height;
//	desc.DepthOrArraySize = 1;
//	desc.MipLevels = 1;
//	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	desc.SampleDesc.Count = 1;
//	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
//	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//	D3D12_HEAP_PROPERTIES defaultHeapProps = {};
//	defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
//	defaultHeapProps.CreationNodeMask = 1;
//	defaultHeapProps.VisibleNodeMask = 1;
//
//	ID3D12Resource* defaultResource = nullptr;
//	HRESULT hr = dev->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&defaultResource));
//	if (FAILED(hr) || !defaultResource)
//	{
//		if (pixels_owned) stbi_image_free(pixels);
//		return nullptr;
//	}
//
//	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
//	UINT numRows = 0;
//	UINT64 rowSizeInBytes = 0;
//	UINT64 requiredSize = 0;
//	dev->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &numRows, &rowSizeInBytes, &requiredSize);
//	D3D12_HEAP_PROPERTIES uploadHeapProps = {};
//	uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
//	uploadHeapProps.CreationNodeMask = 1;
//	uploadHeapProps.VisibleNodeMask = 1;
//	D3D12_RESOURCE_DESC uploadDesc = {};
//	uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//	uploadDesc.Width = requiredSize;
//	uploadDesc.Height = 1;
//	uploadDesc.DepthOrArraySize = 1;
//	uploadDesc.MipLevels = 1;
//	uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
//	uploadDesc.SampleDesc.Count = 1;
//	uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//	uploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
//	ID3D12Resource* uploadResource = nullptr;
//	hr = dev->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadResource));
//	if (FAILED(hr) || !uploadResource)
//	{
//		defaultResource->Release();
//		if (pixels_owned) stbi_image_free(pixels);
//		return nullptr;
//	}
//	void* mappedBase = nullptr;
//	D3D12_RANGE readRange = { 0, 0 };
//	hr = uploadResource->Map(0, &readRange, &mappedBase);
//	if (FAILED(hr) || !mappedBase)
//	{
//		uploadResource->Release();
//		defaultResource->Release();
//		if (pixels_owned) stbi_image_free(pixels);
//		return nullptr;
//	}
//
//	{
//		const UINT srcRowPitch = static_cast<UINT>(width * 4);
//		BYTE* destBase = reinterpret_cast<BYTE*>(mappedBase) + footprint.Offset;
//		for (UINT r = 0; r < numRows; ++r)
//			memcpy(destBase + static_cast<size_t>(r) * footprint.Footprint.RowPitch, pixels + static_cast<size_t>(r) * srcRowPitch, srcRowPitch);
//	}
//	uploadResource->Unmap(0, nullptr);
//
//	ID3D12CommandAllocator* cmdAlloc = nullptr;
//	ID3D12GraphicsCommandList* cmdList = nullptr;
//	ID3D12CommandQueue* cmdQueue = nullptr;
//	ID3D12Fence* fence = nullptr;
//	HANDLE fenceEvent = nullptr;
//	UINT64 fenceValue = 1;
//
//	auto cleanup_and_return = [&]() -> ID3D12Resource*
//	{
//		if (fenceEvent) { CloseHandle(fenceEvent); fenceEvent = nullptr; }
//		if (fence) { fence->Release(); fence = nullptr; }
//		if (cmdQueue) { cmdQueue->Release(); cmdQueue = nullptr; }
//		if (cmdList) { cmdList->Release(); cmdList = nullptr; }
//		if (cmdAlloc) { cmdAlloc->Release(); cmdAlloc = nullptr; }
//		if (uploadResource) { uploadResource->Release(); uploadResource = nullptr; }
//		if (defaultResource) { defaultResource->Release(); defaultResource = nullptr; }
//		if (pixels_owned) { stbi_image_free(pixels); pixels = nullptr; }
//		return nullptr;
//	};
//
//	if (FAILED(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc))))
//		return cleanup_and_return();
//	if (FAILED(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, nullptr, IID_PPV_ARGS(&cmdList))))
//		return cleanup_and_return();
//
//	D3D12_COMMAND_QUEUE_DESC qdesc = {};
//	qdesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//	qdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
//	if (FAILED(dev->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&cmdQueue)))) return cleanup_and_return();
//	D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
//	dstLoc.pResource = defaultResource;
//	dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
//	dstLoc.SubresourceIndex = 0;
//	D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
//	srcLoc.pResource = uploadResource;
//	srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
//	srcLoc.PlacedFootprint = footprint;
//	cmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
//	D3D12_RESOURCE_BARRIER barrier = {};
//	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//	barrier.Transition.pResource = defaultResource;
//	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
//	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//	cmdList->ResourceBarrier(1, &barrier);
//	hr = cmdList->Close();
//	if (FAILED(hr)) return cleanup_and_return();
//	ID3D12CommandList* listsToExecute[] = { cmdList };
//	cmdQueue->ExecuteCommandLists(1, listsToExecute);
//	if (FAILED(dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) return cleanup_and_return();
//	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
//	if (!fenceEvent) return cleanup_and_return();
//	if (FAILED(cmdQueue->Signal(fence, fenceValue))) return cleanup_and_return();
//	if (fence->GetCompletedValue() < fenceValue)
//	{
//		if (FAILED(fence->SetEventOnCompletion(fenceValue, fenceEvent))) return cleanup_and_return();
//		WaitForSingleObject(fenceEvent, INFINITE);
//	}
//	++fenceValue;
//	if (fenceEvent) { CloseHandle(fenceEvent); fenceEvent = nullptr; }
//	if (fence) { fence->Release(); fence = nullptr; }
//	if (cmdQueue) { cmdQueue->Release(); cmdQueue = nullptr; }
//	if (cmdList) { cmdList->Release(); cmdList = nullptr; }
//	if (cmdAlloc) { cmdAlloc->Release(); cmdAlloc = nullptr; }
//	if (uploadResource) { uploadResource->Release(); uploadResource = nullptr; }
//	if (pixels_owned) { stbi_image_free(pixels); pixels = nullptr; }
//	return defaultResource;
//}
#elifdef AnyOpenGLActive
GLTexture App::GL_LoadTextureFromFile(const char* filename)
{
	GLTexture t;
	if (!filename || !filename[0]) return t;
	int w = 0, h = 0, comp = 0;
	unsigned char* rgba = stbi_load(filename, &w, &h, &comp, 4);
	if (!rgba) return t;
	if (!GL_CreateTextureRGBA8(rgba, w, h, t)) GL_DestroyTexture(t);
	stbi_image_free(rgba);
	return t;
}
GLTexture App::GL_LoadTextureFromMemory(void* data, size_t size)
{
	GLTexture t;
	if (!data || size == 0) return t;
	int w = 0, h = 0, comp = 0;
	unsigned char* rgba = stbi_load_from_memory((const unsigned char*)data, (int)size, &w, &h, &comp, 4);
	if (!rgba) return t;
	if (!GL_CreateTextureRGBA8(rgba, w, h, t)) GL_DestroyTexture(t);
	stbi_image_free(rgba);
	return t;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HookingLayer::InstallHooks()
{
	if (hooks_applied)
	{
		MessageBox(nullptr, "InstallHooks was called but it has already been called. To call again, you must first call UninstallHooks. Terminating...", "F", MB_OK);
		ExitProcess(1);
	}
	for (auto& [name, hookInfo] : hooks)
	{
		if (!hookInfo.detour->hook())
		{
			MessageBox(nullptr, std::format("Failed to install hook for \"{}\"", name).c_str(), "F", MB_OK);
			ExitProcess(1);
		}
	}
	hooks_applied = true;
}

void HookingLayer::UninstallHooks()
{
	if (!hooks_applied)
	{
		MessageBox(nullptr, "UninstallHooks was called but InstallHooks was never called before. Terminating...", "F", MB_OK);
		ExitProcess(1);
	}
	for (auto& [name, hookInfo] : hooks)
	{
		if (hookInfo.detour->isHooked()) hookInfo.detour->unHook();
		delete hookInfo.detour;
	}
	hooks.clear();
	hooks_applied = false;
}

bool HookingLayer::IsHookIntalled(const std::string_view& name)
{
	return hooks.contains(name);
}

void HookingLayer::RegisterPatch(uint64_t address, const std::vector<uint8_t>& patchBytes, size_t singleByteCount)
{
	for (auto& patch : patches)
	{
		if (patch.address == address)
		{
			MessageBox(nullptr, std::format("RegisterPatch was called again on address: '{:#X}'. Terminating...", address).c_str(), "F", MB_OK);
			ExitProcess(1);
		}
	}
	size_t writeCount = patchBytes.size();
	if (singleByteCount > 0 && patchBytes.size() == 1) writeCount = singleByteCount;
	Patch patch {};
	patch.address = address;
	if (writeCount != patchBytes.size())
		patch.patchBytes.assign(writeCount, patchBytes[0]);
	else
		patch.patchBytes = patchBytes;

	patch.originalBytes.resize(writeCount);
	memcpy(patch.originalBytes.data(), reinterpret_cast<void*>(address), writeCount);
	patches.push_back(std::move(patch));
}

void HookingLayer::ReplaceCall(uint64_t callAddress, uint64_t newFunction)
{
	for (auto& patch : patches)
	{
		if (patch.address == callAddress)
		{
			MessageBox(nullptr, std::format("ReplaceCall was already called for address: '{:#X}'. Terminating...", callAddress).c_str(), "F", MB_OK);
			ExitProcess(1);
		}
	}
	Patch patch;
	patch.originalBytes.resize(5);
	memcpy(patch.originalBytes.data(), reinterpret_cast<void*>(callAddress), 5);
	patch.address = callAddress;
	uint64_t relativeAddr = newFunction - callAddress - 5;
	patch.patchBytes.resize(5);
	patch.patchBytes[0] = 0xE8;
	*reinterpret_cast<uint64_t*>(patch.patchBytes.data() + 1) = relativeAddr;
	patches.push_back(std::move(patch));
}

void HookingLayer::InstallPatches()
{
	if (patches_applied)
	{
		MessageBox(nullptr, "InstallPatches was called but it has already been called. To call again, you must first call UninstallPatches. Terminating...", "F", MB_OK);
		ExitProcess(1);
	}
	for (auto& patch : patches)
	{
		BYTE* dst = (BYTE*)patch.address;
		BYTE* src = patch.patchBytes.data();
		const size_t size = patch.patchBytes.size();
		DWORD oldProtect;
		VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(dst, src, size);
		VirtualProtect(dst, size, oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), dst, size);
	}
	patches_applied = true;
}

void HookingLayer::RegisterHook(const std::string_view& name, uint64_t funcToHook, uint64_t detour)
{
	if (hooks.contains(name))
	{
		MessageBox(nullptr, std::format("Tried registering a hook for \"{}\" but it already exists. Terminating...", name).c_str(), "F", MB_OK);
		ExitProcess(1);
	}
	hooks[name] = Hook();
#ifdef BUILD_x86
	this->hooks[name].detour = new PLH::x86Detour(funcToHook, detour, &this->hooks[name].original);
#elifdef BUILD_x64
	this->hooks[name].detour = new PLH::x64Detour(funcToHook, detour, &this->hooks[name].original);
#endif
}

void HookingLayer::UninstallPatches()
{
	if (!patches_applied)
	{
		MessageBox(nullptr, "UninstallPatches was called but InstallPatches was never called before. Terminating...", "F", MB_OK);
		ExitProcess(1);
	}
	for (auto& patch : patches)
	{
		BYTE* dst = (BYTE*)patch.address;
		BYTE* src = patch.originalBytes.data();
		const size_t size = patch.originalBytes.size();
		DWORD oldProtect;
		VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(dst, src, size);
		VirtualProtect(dst, size, oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), dst, size);
	}
	patches.clear();
	patches_applied = false;
}

static std::vector<int> ParsePattern(const std::string_view& pattern)
{
	std::vector<int> bytes;
	std::string token;
	for (size_t i = 0u; i < pattern.size(); ++i)
	{
		char c = pattern[i];
		if (c == ' ')
		{
			if (!token.empty())
			{
				if (token == "?")
					bytes.push_back(-1);
				else
					bytes.push_back(static_cast<int>(strtoul(token.c_str(), nullptr, 16)));
				token.clear();
			}
			continue;
		}
		token.push_back(c);
	}
	if (!token.empty())
	{
		if (token == "?")
			bytes.push_back(-1);
		else
			bytes.push_back(static_cast<int>(strtoul(token.c_str(), nullptr, 16)));
	}
	return bytes;
}
BYTE* HookingLayer::FindPattern(const std::string_view& pattern, const std::string_view& module)
{
	HMODULE modHandle = module.empty() ? GetModuleHandleA(nullptr) : GetModuleHandleA(module.data());
	if (!modHandle) return nullptr;
	MODULEINFO moduleInfo {};
	if (!GetModuleInformation(GetCurrentProcess(), modHandle, &moduleInfo, sizeof(moduleInfo))) return nullptr;
	BYTE* begin = static_cast<BYTE*>(moduleInfo.lpBaseOfDll);
	const uintptr_t size = moduleInfo.SizeOfImage;
	if (!begin || size == 0u) return nullptr;
	std::vector<int> bytePattern = ParsePattern(pattern);
	if (bytePattern.empty()) return nullptr;
	MEMORY_BASIC_INFORMATION memInfo {};
	BYTE* maxAddr = begin + size;
	for (BYTE* addr = begin; addr < maxAddr; addr += memInfo.RegionSize)
	{
		if (VirtualQuery(addr, &memInfo, sizeof(memInfo)) == 0) break;
		if (memInfo.State != MEM_COMMIT) continue;
		if (memInfo.Protect == PAGE_NOACCESS) continue;
		if (memInfo.Protect & PAGE_GUARD) continue;
		BYTE* regionStart = static_cast<BYTE*>(memInfo.BaseAddress);
		SIZE_T regionSize = memInfo.RegionSize;
		if (regionStart + regionSize > maxAddr) regionSize = static_cast<SIZE_T>(maxAddr - regionStart);
		if (regionSize < bytePattern.size()) continue;
		for (SIZE_T offset = 0; offset + bytePattern.size() <= regionSize; ++offset)
		{
			bool matched = true;
			for (SIZE_T pi = 0; pi < bytePattern.size(); ++pi)
			{
				const int pat = bytePattern[pi];
				BYTE b = *(regionStart + offset + pi);
				if (pat != -1 && static_cast<BYTE>(pat) != b) { matched = false; break; }
			}
			if (matched) return regionStart + offset;
		}
	}
	return nullptr;
}
