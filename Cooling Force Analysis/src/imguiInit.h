#pragma once

#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                  FenceValue;
};


// Data
int const                    NUM_FRAMES_IN_FLIGHT = 3;
int const                    NUM_BACK_BUFFERS = 3;

extern FrameContext                g_frameContext[NUM_FRAMES_IN_FLIGHT];
extern UINT                         g_frameIndex;

extern ID3D12Device* g_pd3dDevice;
extern ID3D12DescriptorHeap* g_pd3dRtvDescHeap;
extern ID3D12DescriptorHeap* g_pd3dSrvDescHeap;
extern ID3D12CommandQueue* g_pd3dCommandQueue;
extern ID3D12GraphicsCommandList* g_pd3dCommandList;
extern ID3D12Fence* g_fence;
extern HANDLE                       g_fenceEvent;
extern UINT64                       g_fenceLastSignaledValue;
extern IDXGISwapChain3* g_pSwapChain;
extern HANDLE                       g_hSwapChainWaitableObject;
extern ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS];
extern D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS];


// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
FrameContext* WaitForNextFrameResources();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

