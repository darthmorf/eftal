#include <cmath>
#include <cstdio>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#if DEBUG
#include <dxgidebug.h>

static ComPtr<IDXGIInfoQueue> g_dxgiInfoQueue;
static ComPtr<ID3D12InfoQueue> g_d3d12InfoQueue;
#endif

// TODO_ASSERT: replace with proper error handling. Only usable in main() because of the return.
#define CHECK_HR(expr)                                                                          \
    do                                                                                          \
    {                                                                                           \
        HRESULT hr_ = (expr);                                                                   \
        if (FAILED(hr_))                                                                        \
        {                                                                                       \
            fprintf(stderr, "%s(%d): HRESULT 0x%08lX from %s\n", __FILE__, __LINE__, hr_, #expr); \
            if (IsDebuggerPresent())                                                            \
                __debugbreak();                                                                 \
            return -1;                                                                          \
        }                                                                                       \
    } while (0)


void glfwErr(int error, const char* description)
{
    // TODO_LOGGING
    fprintf(stderr, "[GLFW] Error %d: %s\n", error, description);
}


int main()
{
    // Init GLFW
    glfwSetErrorCallback(glfwErr);

    if (!glfwInit())
        return -1; // TODO_ASSERT, Asserts when we have them

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO_WINDOW_RESIZE
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We're using DX12 so we want 'no api'.

    const int width = 640;
    const int height = 480;
    const UINT framesInFlight = 3;
    ComPtr<ID3D12Resource> backBuffers[framesInFlight];

    GLFWwindow* window = glfwCreateWindow(width, height, "Eftal", NULL, NULL);
    if (!window)
       return -1; // TODO_ASSERT

    // Init DX12
#if DEBUG
    ComPtr<ID3D12Debug> debugInterface;
    CHECK_HR(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();

    // Breaking with no debugger attached would crash the app instead.
    CHECK_HR(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&g_dxgiInfoQueue)));
    if (IsDebuggerPresent())
    {
        g_dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        g_dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
    }
#endif

    ComPtr<IDXGIFactory6> dxgiFactory;
    UINT createFactoryFlags = 0;
#if DEBUG
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    CHECK_HR(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    ComPtr<IDXGIAdapter4> dxgiAdaptor;
    for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&dxgiAdaptor)) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC3 desc;
        dxgiAdaptor->GetDesc3(&desc);

        if ((desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(dxgiAdaptor.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
            break;
    }

    if (!dxgiAdaptor)
        return -1; // TODO_ASSERT

    ComPtr<ID3D12Device2> d3d12device;
    CHECK_HR(D3D12CreateDevice(dxgiAdaptor.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&d3d12device)));

#if DEBUG
    CHECK_HR(d3d12device.As(&g_d3d12InfoQueue));
    if (IsDebuggerPresent())
    {
        g_d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        g_d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        g_d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
    }
#endif

    ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cmdQueueDesc.NodeMask = 0;

    CHECK_HR(d3d12device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&d3d12CommandQueue)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = framesInFlight;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = false ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0; // TODO_TEARING - We can enable this to make use of Freesync/Vsync

    HWND windowHdl = glfwGetWin32Window(window);
    ComPtr<IDXGISwapChain1> dxgiSwapChain_1; // We must create as a swapchain1 then convert
    ComPtr<IDXGISwapChain4> dxgiSwapChain;
    CHECK_HR(dxgiFactory->CreateSwapChainForHwnd(d3d12CommandQueue.Get(), windowHdl, &swapChainDesc, nullptr, nullptr, &dxgiSwapChain_1));
    CHECK_HR(dxgiSwapChain_1.As(&dxgiSwapChain));
    CHECK_HR(dxgiFactory->MakeWindowAssociation(windowHdl, DXGI_MWA_VALID)); // TODO_FULLSCREEN

    ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    descriptorHeapDesc.NumDescriptors = framesInFlight;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    CHECK_HR(d3d12device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap)));

    UINT rtvDescriptorSize = d3d12device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < framesInFlight; i++)
    {
        ComPtr<ID3D12Resource> backBuffer;
        CHECK_HR(dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        d3d12device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
        backBuffers[i] = backBuffer;
        rtvHandle.Offset(rtvDescriptorSize);
    }

    ComPtr<ID3D12CommandAllocator> commandAllocators[framesInFlight];
    for (UINT i = 0; i < framesInFlight; i++)
        CHECK_HR(d3d12device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i])));

    ComPtr<ID3D12Fence> fence;
    CHECK_HR(d3d12device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    UINT64 fenceValue = 0;
    UINT64 frameFenceValues[framesInFlight] = {};
    HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!fenceEvent)
        return -1; // TODO_ASSERT


    UINT frameIdx = dxgiSwapChain->GetCurrentBackBufferIndex();

    ComPtr<ID3D12GraphicsCommandList> commandList;
    CHECK_HR(d3d12device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[frameIdx].Get(), nullptr, IID_PPV_ARGS(&commandList)));
    CHECK_HR(commandList->Close());

    // Game loop
    fprintf(stdout, "[GLFW] Polling...\n");
    while (!glfwWindowShouldClose(window))
    {
        // Wait until the GPU has finished the last frame that used this slot.
        if (fence->GetCompletedValue() < frameFenceValues[frameIdx])
        {
            CHECK_HR(fence->SetEventOnCompletion(frameFenceValues[frameIdx], fenceEvent));
            WaitForSingleObject(fenceEvent, INFINITE); // TODO_SYNC: time out instead of waiting forever
        }

        ID3D12CommandAllocator* commandAllocator = commandAllocators[frameIdx].Get();
        ID3D12Resource* backBuffer = backBuffers[frameIdx].Get();

        CHECK_HR(commandAllocator->Reset());
        CHECK_HR(commandList->Reset(commandAllocator, nullptr));

        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandList->ResourceBarrier(1, &barrier);

        // Three sine waves a third of a cycle apart give a smooth hue cycle.
        const float t = static_cast<float>(glfwGetTime());
        FLOAT clearColour[] = { 0.5f + 0.5f * sinf(t), 0.5f + 0.5f * sinf(t + 2.094f), 0.5f + 0.5f * sinf(t + 4.189f), 1.0f };
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIdx, rtvDescriptorSize);

        commandList->ClearRenderTargetView(rtv, clearColour, 0, nullptr);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        commandList->ResourceBarrier(1, &barrier);

        CHECK_HR(commandList->Close());

        ID3D12CommandList* const commandLists[] =
        {
            commandList.Get()
        };

        d3d12CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

        CHECK_HR(dxgiSwapChain->Present(0, 0));

        // Mark the point in the queue where this frame's work ends.
        frameFenceValues[frameIdx] = ++fenceValue;
        CHECK_HR(d3d12CommandQueue->Signal(fence.Get(), fenceValue));

        frameIdx = dxgiSwapChain->GetCurrentBackBufferIndex();

        glfwPollEvents();
    }

    // Cleanup DX12
    CHECK_HR(d3d12CommandQueue->Signal(fence.Get(), ++fenceValue));
    CHECK_HR(fence->SetEventOnCompletion(fenceValue, fenceEvent));
    WaitForSingleObject(fenceEvent, INFINITE);
    CloseHandle(fenceEvent);

#if DEBUG
    // The D3D12 info queue holds a device reference; release both before main returns.
    g_d3d12InfoQueue.Reset();
    g_dxgiInfoQueue.Reset();
#endif

    // Cleanup GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
