#pragma once

#include <cstdint>

namespace Engine
{
    namespace Graphics
    {
        void Initialize();

        void Present();

        void Shutdown();

        void Resize(uint32_t width, uint32_t height);
    }
}