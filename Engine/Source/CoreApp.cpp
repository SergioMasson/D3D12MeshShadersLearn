#include "Engine/CoreApp.h"
#include "Engine/Utility.h"
#include "Engine/SystemTime.h"
#include <cstdint>

namespace Engine 
{
    uint32_t g_DisplayWidth = 1024;
    uint32_t g_DisplayHeight = 900;
    HWND g_hWnd = NULL;

	bool IGameApp::IsDone() 
	{
		return false;
	}

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void InitializeApplication(IGameApp& game)
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        //CommandLineArgs::Initialize(argc, argv);

        //Graphics::Initialize(game.RequiresRaytracingSupport());
        SystemTime::Initialize();
        //GameInput::Initialize();
        //EngineTuning::Initialize();

        game.Startup();
    }

    void TerminateApplication(IGameApp& game)
    {
        //g_CommandManager.IdleGPU();

        game.Cleanup();

        //GameInput::Shutdown();
    }

    bool UpdateApplication(IGameApp& game)
    {
        double frameTime = SystemTime::GetCurrentFrameTime();

        /*EngineProfiling::Update();

        float DeltaTime = Graphics::GetFrameTime();

        GameInput::Update(DeltaTime);
        EngineTuning::Update(DeltaTime);

        game.Update(DeltaTime);
        game.RenderScene();

        PostEffects::Render();

        GraphicsContext& UiContext = GraphicsContext::Begin(L"Render UI");
        UiContext.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        UiContext.ClearColor(g_OverlayBuffer);
        UiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
        UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
        game.RenderUI(UiContext);

        UiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
        UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
        EngineTuning::Display(UiContext, 10.0f, 40.0f, 1900.0f, 1040.0f);

        UiContext.Finish();

        Display::Present();*/

        return !game.IsDone();
    }

	int RunApplication(IGameApp& app, const char* className, HINSTANCE hInst, int nCmdShow) 
	{
        // Register class
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInst;
        wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = className;
        wcex.hIconSm = LoadIcon(hInst, IDI_APPLICATION);
        ENGINE_ASSERT(0 != RegisterClassEx(&wcex), "Unable to register a window");


        RECT rc = { 0, 0, (LONG)g_DisplayWidth, (LONG)g_DisplayHeight };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        g_hWnd = CreateWindow(className, className, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);

        ENGINE_ASSERT(g_hWnd != 0);

        InitializeApplication(app);

        ShowWindow(g_hWnd, nCmdShow/*SW_SHOWDEFAULT*/);

        do
        {
            MSG msg = {};
            bool done = false;
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT)
                    done = true;
            }

            if (done)
                break;
        } while (UpdateApplication(app));	// Returns false to quit loop

        TerminateApplication(app);
        //Graphics::Shutdown();
        return 0;
	}

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_SIZE:
            //Graphics::Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        return 0;
    }
}