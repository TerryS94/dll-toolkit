Setting up boilerplate on every new project gets old fast, so this toolkit exists to let you skip it and work directly with the game’s own renderer from the start.

## What You Need First
This dll toolkit does not create its own backend instances. Instead, it expects you to provide one of the following for your target game:
- DirectX 9: the game’s ```IDirect3DDevice9*```. Everything else is derived from that device.
- DirectX10/11: the game’s ```IDXGISwapChain*```. Everything else is derived from that swapchain.
- OpenGL2/3: the GLSL version string (e.g. ```"#version 330 core"```) that ImGui needs at init.

This lets the toolkit run on the game’s real renderer rather than a local dummy, so visuals, device state, and input match the game exactly.

This may sound overkill if you only plan to make the simplest overlay, but if you know basic reverse engineering, providing the required field is usually quick.

## Project Setup
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

# Example Usage
### Here is a simple dllmain.cpp sample you can achieve with this toolkit in it's current state
- even less code if this example only focused on a single game :)

```cpp
//the flow of things may evolve with time... but even already
//you can see how simple it is to get somethin goin :)

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
static void DrawStuff()
{
    if (app.IsMenuOpen())
    {
#ifdef ImGui_IncludeDemo
        ImGui::ShowDemoWindow();
#endif
        ImGui::Begin("##testwindow123", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        {
            ImGui::Text("Hello");
            auto* customTexturePtr = app.GetTextureByName("UniqueName1");
#ifdef AnyOpenGLActive
            ImGui::Image((ImTextureID)(intptr_t)customTexturePtr->texture.id, ImVec2(64, 64));
#elifdef AnyDirectXActive
            ImGui::Image((ImTextureID)(intptr_t)customTexturePtr->ptr, ImVec2(64, 64));
#endif
            ImGui::End();
        }
    }
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
#elif defined DirectX10 || defined DirectX11 //Present function will first derive the device and rendertargetview and hwnd etc then we can finally init
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
    //triggers when changing the window resolution or some other event that requires a hard reload
	//if using DX10/DX11 then pretty sure you don't even need this scope at all from what i've gathered.
    if (app.Need_ImGui_Reload())
    {
#ifdef AnyDirectXActive
        if (HWND hwnd = GetGameWindowHandle())
        {
            app.Set_RendererActive(false);
            app.Set_ImGui_Reload(false);
#ifdef DirectX9
            app.UpdateDirectXDevice(GetGameDevicePtr());
#endif
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
    
    if (!app.IsRendererActive())
		return;

	if (app.IsKeyPressed(AppKeys::KEY_F3))
    	app.ToggleFreeMouseCursor(!app.IsFreeMouseCursorActive());
    if (app.IsKeyPressed(AppKeys::KEY_F4))
        app.ToggleMenu(!app.IsMenuOpen());

    const bool want_mouse_this_frame = app.IsMenuOpen() || app.IsFreeMouseCursorActive();// || other custom logic you may want here
    app.BeginFrame(want_mouse_this_frame);

    DrawStuff();

    app.EndFrame();
}

DWORD WINAPI MainThread(MAYBEUNUSED LPVOID lpParameter)
{
	{
		app.Set_PriorityClass(HIGH_PRIORITY_CLASS); //optional
		//pass your InitResources to App so it can call it where it needs to in the provided hooks
		app.Set_InitResourcesFunc(InitResources);//toolkit can now automatically call your init resources in certain hooks
		app.Set_UserRenderFunc(MainRender);//toolkit can now automatically call your render function where it needs to.

#ifdef DirectX9
        auto* device = GetGameDevicePtr();
        app.Set_TargetWindowInfo("Call of Duty 4 X", "CoD4");//example for the game 'Call of Duty 4'
        app.UpdateDirectXDevice(device); 
#elifdef DirectX11
        app.Update_HWND(GetWindowHandle());
		app.Set_TargetWindowInfo("game window title", "window class name");
		//dx device and context are derived from the swapchain
        app.UpdateDirectXSwapChain(*reinterpret_cast<IDXGISwapChain**>(0x35E5F94));//example for BO2 Plutonium version
#elifdef AnyOpenGLActive
        app.Set_TargetWindowInfo("MX Bikes", "Core Window Class");//example for the game 'MX Bikes'
        app.SetGLSLVersion("#version 330 core");
#endif

        app.RegisterBackEndHooks();
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
            if (app.IsKeyChordPressed(AppKeys::LEFT_SHIFT, AppKeys::DEL))
            {
                //flag to the Shutdown call that we want to tear down EVERYTHING
				//including hooks etc.
                app.SignalEject();
                break;
            }
        }
        app.Shutdown();
	}
    FreeLibraryAndExitThread(app.Get_DLLHandle(), 0);
}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, MAYBEUNUSED LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
		app.Set_DLLHandle(hModule);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, NULL);
    }
    return TRUE;
}
```
