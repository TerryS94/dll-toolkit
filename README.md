**dll-toolkit is still ___very WIP___ but mostly working :)**

This started out initially as a private project for convenience on future dll tools because it's annoying setting up some of the boiler plate before being able to get to the fun stuff, so recently I decided that it's finally time to solve that problem for myself and maybe even for others too :)

## Adding to your dll project
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
### Here is a simple dllmain.cpp example you can achieve with this framework in it's current state
- It would be even less code if this example only focused on a single backend :)

```cpp
//the flow of things may evolve with time... but even already
//you can see how simple it is to get somethin goin :)

//inside this App header at the top you can mess with the configuration (which backend you want etc)
#include "App/App.h"

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
//call this in the appropriate provided hook such as EndScene for DX9 which is located in App/Hooks/DX9_Hooks.cpp
void MainRender()
{
    //ensure we only ever go into the second if statement exactly one time.
    static bool calledInitFirstTime = false;
    if (!app.HasInitializedFirstTime() && !calledInitFirstTime)
    {
#if defined AnyDirectXActive
        if (HWND hwnd = GetGameWindowHandle())
        {
            calledInitFirstTime = true;
            app.InitRenderer();
            InitResources();
        }
#elifdef AnyOpenGLActive
        calledInitFirstTime = true;
        app.Update_HWND(GetWindowHandle());
        app.InitRenderer();
        InitResources();//user provided function
#endif
    }
    //triggers when changing the window resolution or some other event that requires a hard reload
    if (app.Need_ImGui_Reload())
    {
#ifdef AnyDirectXActive
        if (HWND hwnd = GetGameWindowHandle())
        {
            app.Set_RendererActive(false);
            app.Set_ImGui_Reload(false);
#ifdef DirectX9 //an example for DX9
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

    if (app.IsKeyPressed(AppKeys::KEY_F4))
        app.ToggleMenu(!app.IsMenuOpen());

    const bool want_mouse_this_frame = app.IsMenuOpen(); // || other custom logic you may want here
    app.BeginFrame(want_mouse_this_frame);

    DrawStuff();

    app.EndFrame();
}

DWORD WINAPI MainThread(MAYBEUNUSED LPVOID lpParameter)
{
	{
		app.Set_PriorityClass(HIGH_PRIORITY_CLASS); //optional
#ifdef DirectX9
        auto* device = GetGameDevicePtr();
        app.Set_TargetWindowInfo("Call of Duty 4 X", "CoD4");//example for the game 'Call of Duty 4' which uses DirectX9 backend
        app.Update_HWND(GetGameWindowHandle());
        app.UpdateDirectXDevice(reinterpret_cast<void*>(device)); 
#elifdef DirectX11
        app.Update_HWND(GetWindowHandle());
        app.UpdateDirectXDevice(*reinterpret_cast<void**>(0x35AE484)); //example
        app.UpdateDirectXSwapChain(*reinterpret_cast<void**>(0x35E5F94)); //example
        app.UpdateDirectXContext(*reinterpret_cast<void**>(0x35AE488)); //example
#elifdef AnyOpenGLActive
        app.Set_TargetWindowInfo("MX Bikes", "Core Window Class");//example for the game 'MX Bikes'
        app.SetGLSLVersion("#version 330 core");
#endif

        app.RegisterBackEndHooks();
        app.RegisterUniversalHooks();

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
                //flag to the Shutdown call that we want to tear down EVERYTHING and not trying to just reload imgui.
                app.Start_Eject();
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
