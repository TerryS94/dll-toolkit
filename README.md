Setting up boilerplate on every new project gets old fast, so this toolkit exists to help you skip that and work directly with the game’s own renderer from the start.

## What You Need First
This dll toolkit does not create its own backend instances. Instead, it expects you to provide one of the following for your target game:
- DirectX9: the game’s ```IDirect3DDevice9*```. Everything else is derived from that device.
- DirectX10/11: the game’s ```IDXGISwapChain*```. Everything else is derived from that swapchain.
- OpenGL2: nothing required.
- OpenGL3: the GLSL version string (e.g. ```"#version 330 core"```) that ImGui needs at init.

This lets the toolkit run on the game’s real renderer rather than a local dummy, so visuals, device state, and input match the game exactly.

This may sound overkill if you only plan to make the simplest overlay, but if you know basic reverse engineering, providing the required field is usually quick.

## Project Setup
```
git clone https://github.com/TerryS94/dll-toolkit.git
cd dll-toolkit
git submodule update --init --remote --recursive
```
- Add the **DirectX9** folder into your dll project. (DX9 isn't officially supported on Windows so that's why we need it)
- Add the **App** folder into your dll project.
- Also Pull ImGui into your project. (Leave its folder structure in tact)

## Notes
This project uses the vcpkg package manager for a couple static dependencies, such as:
- **STB** header file (For loading textures from file/memory for backends that are not DX9)
- **PolyHook2** (Mainly For convenience but the hooking layer could be replaced easily)

So you will need to also obtain those as well if you don't have already.

If you already have vcpkg but don't have those packages then open a terminal and do:
```
vcpkg install stb:x86-windows-static stb:x64-windows-static
vcpkg install polyhook2:x86-windows-static polyhook2:x64-windows-static
```

## Example Usage
<details>
<summary>CLICK HERE to see an example dllmain.cpp</summary>
	
```cpp
//inside this App header at the top you can mess with the configuration (which backend you want etc)
#include "App/App.h"

//define a function like this for your textures/fonts and pass-
//the function pointer into app.Set_InitResourcesFunc on inject
void InitResources()
{
    app.AddTextureFromFile("UniqueName1", "C:\\some_location\\image.png");
    app.AddTextureFromMemory("UniqueName2", pngDataByteArray, sizeof(pngDataByteArray));
    //... similar functions for loading Fonts
}

void MainRender()
{
    //ensure we only ever run first time init once
    static bool calledInitFirstTime = false;
    if (!app.HasInitializedFirstTime() && !calledInitFirstTime)
    {
#ifdef DirectX9
        if (HWND hwnd = GetWindowHandle())
        {
            calledInitFirstTime = true;
            app.Update_HWND(hwnd);
            app.InitRenderer();
            app.Call_UserInitResources();
        }
//Present function will first derive the desired outputwindow (hwnd) and mainRenderTargetView then our MainRender is called
#elif defined DirectX10 || defined DirectX11
        calledInitFirstTime = true;
        app.InitRenderer();
        app.Call_UserInitResources();
#elifdef AnyOpenGLActive
        calledInitFirstTime = true;
        app.Update_HWND(GetWindowHandle());
        app.InitRenderer();
        app.Call_UserInitResources();
#endif
    }

#if defined DirectX9 || defined AnyOpenGLActive
    //triggers when changing the window resolution or some other event that requires a hard reload
    if (app.Need_ImGui_Reload())
    {
#ifdef DirectX9
        if (HWND hwnd = GetGameWindowHandle())
        {
            app.Set_RendererActive(false);
            app.Set_ImGui_Reload(false);
            app.UpdateDirectXDevice(GetGameDevicePtr());
            app.Update_HWND(hwnd);
            app.ReloadRenderer();
            InitResources();
        }
#elifdef AnyOpenGLActive
        app.Set_ImGui_Reload(false);
        app.ReloadRenderer();
        InitResources();
#endif
    }
#endif
    
    if (!app.IsRendererActive())
		return;

	//give yourself a free cursor without needing to open your whole menu (for moving HUD around)
	if (app.IsKeyPressed(AppKeys::KEY_F3))
    	app.ToggleFreeMouseCursor(!app.IsFreeMouseCursorActive());
	//opens your menu
    if (app.IsKeyPressed(AppKeys::KEY_F4))
        app.ToggleMenu(!app.IsMenuOpen());

	//input/cursor blocking to the target window and cursor restoration is handled automatically :)
    const bool isMenuOpen = app.IsMenuOpen();
	const bool hasFreeCursor = app.IsFreeMouseCursorActive();
	const bool hasMouseAtAll = isMenuOpen || hasFreeCursor;
	const bool want_mouse_this_frame = hasMouseAtAll;// || other custom logic you may want here

    app.BeginFrame(want_mouse_this_frame);

	if (isMenuOpen)
	{
#ifdef ImGui_IncludeDemo
		ImGui::ShowDemoWindow();
#endif
		ImGui::Begin("##testwindow123", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("hello");
		auto* customTexturePtr = app.GetTextureByName("UniqueName1");
		//for OpenGL
		ImGui::Image((ImTextureID)(intptr_t)customTexturePtr->texture.id, ImVec2(64, 64));
		//for DirectX9/10/11
		ImGui::Image((ImTextureID)(intptr_t)customTexturePtr->ptr, ImVec2(64, 64));
		ImGui::End();
	}

    app.EndFrame();
}

DWORD WINAPI MainThread([[maybe_unused]] LPVOID lpParameter)
{
	{
		app.Set_PriorityClass(HIGH_PRIORITY_CLASS); //optional
		//pass your InitResources to App class so it can call it where it needs to in the provided hooks
		app.Set_InitResourcesFunc(InitResources);
		//the toolkit can now automatically call your render function where it needs to.
		app.Set_UserRenderFunc(MainRender);

#ifdef DirectX9
		//make App class aware of the window we want to work with
        app.Set_TargetWindowInfo("Call of Duty 4 X");//example for the game 'Call of Duty 4'
        app.UpdateDirectXDevice(GetGameDevicePtr()); 
#elif defined DirectX10 || defined DirectX11
		//make App class aware of the window we want to work with
		app.Set_TargetWindowInfo("game window title");
		//dx device and context are derived from the swapchain
        app.UpdateDirectXSwapChain(*reinterpret_cast<IDXGISwapChain**>(0x35E5F94));//the games swapchain address
#elifdef OpenGL2
		//make App class aware of the window we want to work with
		app.Set_TargetWindowInfo("game window title");
#elifdef OpenGL3
		//make App class aware of the window we want to work with
        app.Set_TargetWindowInfo("MX Bikes");//example for the game 'MX Bikes'
        app.SetGLSLVersion("#version 330 core");
#endif
		//registers backend specific hooks (EndScene for DirectX9 and so on...)
        app.RegisterBackEndHooks();
		//registers several windows api hooks that the backend will make use of.
        app.RegisterUniversalHooks();

		//example for registering your own hook. (detour not implemented in this example)
		app.RegisterHook("OnTakeDamage", 0x123456, OnTakeDamage_Detour);
        //example for writing a single byte x times at a specific address
        app.RegisterPatch(0x123456, { 0x90 }, 5u);//nop 5 bytes starting at 0x123456
        //example for replacing x bytes at a specific address
        app.RegisterPatch(0x123456, { 0x10, 0x20, 0x30, 0x40 });

		//install any hooks you registered or indirectly registered
		//using the RegisterBackEndHooks/RegisterUniversalHooks functions
        app.InstallHooks();
		//if registered any patches or replaced any function calls then those can get installed too.
		//otherwise this is a no-op.
        app.InstallPatches();

        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
			//eject our dll if this key combo is pressed
            if (app.IsKeyChordPressed(AppKeys::LEFT_SHIFT, AppKeys::DEL))
            {
                //flag to the Shutdown call that we want to tear down EVERYTHING
				//including hooks etc.
                app.SignalEject();
                break;
            }
        }
        app.Shutdown();//will fully shutdown since SignalEject was called above.
	}
    FreeLibraryAndExitThread(app.Get_DLLHandle(), 0);
}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, [[maybe_unused]] LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
		//make App class aware of what dll to eject later
		app.Set_DLLHandle(hModule);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, NULL);
    }
    return TRUE;
}
```
</details>

## Contributing

Contributions of all kinds are welcome! Whether it's bug fixes, optimizations, new features, additional backends, or documentation improvements, your help makes the project better.  

If you submit a pull request, please try to match the existing code style and structure. Keeping things consistent makes it easier for everyone to read, review, and maintain the code.
