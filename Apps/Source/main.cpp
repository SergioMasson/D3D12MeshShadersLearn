#include <iostream>
#include <windows.h>
#include "Engine/CoreApp.h"

class Viewer : public Engine::IGameApp 
{
public:
    void Startup(void) override 
    {

    }

    void Cleanup(void) override 
    {

    }

    void Update(float deltaT) override 
    {

    }
};

CREATE_APPLICATION(Viewer)