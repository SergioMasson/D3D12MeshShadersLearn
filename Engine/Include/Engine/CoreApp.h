#pragma once

#include "windows.h"

namespace Engine
{
    class IGameApp
    {
    public:
        // This function can be used to initialize application state and will run after essential
        // hardware resources are allocated.  Some state that does not depend on these resources
        // should still be initialized in the constructor such as pointers and flags.
        virtual void Startup(void) = 0;
        virtual void Cleanup(void) = 0;

        // Decide if you want the app to exit.  By default, app continues until the 'ESC' key is pressed.
        virtual bool IsDone(void);

        // The update method will be invoked once per frame.  Both state updating and scene
        // rendering should be handled by this method.
        virtual void Update(float deltaT) = 0;
    };
}

namespace Engine
{
    int RunApplication(IGameApp& app, const char* className, HINSTANCE hInst, int nCmdShow);
}

#define CREATE_APPLICATION( app_class ) \
    int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int nCmdShow) \
    { \
        return Engine::RunApplication( app_class(), #app_class, hInstance, nCmdShow ); \
    }