#include "Engine/Graphics.h"
#include "Engine/Utility.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <winrt/base.h>
#include <d3dx12.h>

#include <assert.h>

#include <p_CoreApp.h>

const DXGI_FORMAT BACKBUFFER_FORMAT = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
const uint32_t SWAP_CHAIN_BUFFER_COUNT = 2;

winrt::com_ptr<IDXGIFactory4> g_dxgiFactory;
winrt::com_ptr<IDXGISwapChain> g_swapChain;

winrt::com_ptr<ID3D12Device> g_device;
winrt::com_ptr<ID3D12Fence> g_fence;
winrt::com_ptr<ID3D12CommandQueue> g_commandQueue;
winrt::com_ptr<ID3D12CommandAllocator> g_commandAllocator;
winrt::com_ptr<ID3D12GraphicsCommandList> g_commandList;
winrt::com_ptr<ID3D12Resource> g_SwapChainBuffers[ SWAP_CHAIN_BUFFER_COUNT ];
winrt::com_ptr<ID3D12Resource> g_DepthStencilBuffer;

winrt::com_ptr<ID3D12DescriptorHeap> g_rtvHeap;
winrt::com_ptr<ID3D12DescriptorHeap> g_dsvHeap;

uint32_t RTV_DESCRIPTOR_SIZE;
uint32_t DSV_DESCRIPTOR_SIZE;
uint32_t CBV_SRV_UVA_DESCRIPTOR_SIZE;


uint32_t m4xMsaaQuality = 0;
bool m4xMsaaState = true;
uint32_t currentBackBuffer = 0;

void CreateD3D12Resources()
{
#if defined(DEBUG) || defined(_DEBUG)
    // Enble the D3D12 debug layer
    {
        winrt::com_ptr<ID3D12Debug> debugController;
        ENGINE_ASSERT_SUCCEEDED(D3D12GetDebugInterface(__uuidof(debugController), debugController.put_void()), "Fail to get D3D12 Debug Layer");
        debugController->EnableDebugLayer();
    }
#endif

    ENGINE_ASSERT_SUCCEEDED(CreateDXGIFactory1(__uuidof(g_dxgiFactory), g_dxgiFactory.put_void()), "Fail to get DXGI Factory.");
    auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, __uuidof(g_device), g_device.put_void());

    // If we fail to create a hardware device we try to create one using a WARP adapter.
    if (FAILED(hr))
    {
        winrt::com_ptr<IDXGIAdapter> pWarpAdapter;
        ENGINE_ASSERT_SUCCEEDED(g_dxgiFactory->EnumWarpAdapter(__uuidof(pWarpAdapter), pWarpAdapter.put_void()));
        ENGINE_ASSERT_SUCCEEDED(D3D12CreateDevice(pWarpAdapter.get(), D3D_FEATURE_LEVEL_11_0, __uuidof(g_device), g_device.put_void()), "Fail to create D3D12Device using WARP adapter.");
    }

    // Create main Fence.
    ENGINE_ASSERT_SUCCEEDED(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(g_fence), g_fence.put_void()));

    // Get the size of all descripor types.
    RTV_DESCRIPTOR_SIZE = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    DSV_DESCRIPTOR_SIZE = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    CBV_SRV_UVA_DESCRIPTOR_SIZE = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Get MSAA quality support.
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels{};
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.NumQualityLevels = 0;
    msQualityLevels.Format = BACKBUFFER_FORMAT;

    ENGINE_ASSERT_SUCCEEDED(g_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));
    m4xMsaaQuality = msQualityLevels.NumQualityLevels;
    assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

    // Create command objects.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    ENGINE_ASSERT_SUCCEEDED(g_device->CreateCommandQueue(&queueDesc, __uuidof(g_commandQueue), g_commandQueue.put_void()), "Fail to create ID3D12CommandQueue.");
    ENGINE_ASSERT_SUCCEEDED(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(g_commandAllocator), g_commandAllocator.put_void()), "Fail to create ID3D12CommandAllocator");
    ENGINE_ASSERT_SUCCEEDED(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator.get(), nullptr, __uuidof(g_commandList), g_commandList.put_void()), "Fail to create ID3D12GraphicsCommandList");

    g_commandList->Close();

    // Create descript heaps.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    ENGINE_ASSERT_SUCCEEDED(g_device->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(g_rtvHeap), g_rtvHeap.put_void()));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;

    ENGINE_ASSERT_SUCCEEDED(g_device->CreateDescriptorHeap(&dsvHeapDesc, __uuidof(g_dsvHeap), g_dsvHeap.put_void()));
}

void CreateScreenDependentResources(uint32_t width, uint32_t height)
{
    g_swapChain = nullptr;

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = BACKBUFFER_FORMAT;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    swapChainDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
    swapChainDesc.OutputWindow = Engine::g_hWnd;
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ENGINE_ASSERT_SUCCEEDED(g_dxgiFactory->CreateSwapChain(g_commandQueue.get(), 
                                                           &swapChainDesc, 
                                                           (IDXGISwapChain **)g_swapChain.put_void()), 
                                                           "Fail to create IDXGISwapChain");

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (size_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
    {
        g_swapChain->GetBuffer(i, __uuidof(g_SwapChainBuffers[i]), g_SwapChainBuffers[i].put_void());
        g_device->CreateRenderTargetView(g_SwapChainBuffers[ i ].get(), nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, RTV_DESCRIPTOR_SIZE);
    }

    D3D12_CLEAR_VALUE depthClear{};
    depthClear.DepthStencil.Depth = 1.0f;
    depthClear.DepthStencil.Stencil = 0;

    D3D12_RESOURCE_DESC depthStencilDesc{};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R16_FLOAT;
    depthStencilDesc.MipLevels = 0;
    depthStencilDesc.Height = width;
    depthStencilDesc.Height = height;
    depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    g_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
                                      D3D12_HEAP_FLAG_NONE, 
                                      &depthStencilDesc, 
                                      D3D12_RESOURCE_STATE_COMMON, 
                                      &depthClear, 
                                      __uuidof(g_DepthStencilBuffer),
                                      g_DepthStencilBuffer.put_void());

    g_device->CreateDepthStencilView(g_DepthStencilBuffer.get(), nullptr, DepthStencilView());
}

D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() 
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(g_rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentBackBuffer, RTV_DESCRIPTOR_SIZE);
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()
{
    return g_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

namespace Engine
{
    namespace Graphics
    {
        void Initialize()
        {
            CreateD3D12Resources();
        }

        void Resize(uint32_t width, uint32_t height) 
        {
            CreateScreenDependentResources(width, height);
        }
    }
}