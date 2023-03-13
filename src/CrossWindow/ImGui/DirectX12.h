#pragma once

#include "ImGuiManager.h"
#include "imgui.h"

#include <dxgiformat.h> // DXGI_FORMAT

struct ImDrawData;

struct ID3D12Device;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;

namespace xgfx
{

struct ImGuiD3D12RenderBuffers;
/**
 * A fork of ImGUI's DirectX 12 implementation.
 * imgui/backends/imgui_impl_dx12.cpp
 *
 * Modified to be more stand alone, removing the need to compile shaders,
 * And the interface simplified.
 */
class D3D12ImGuiManager : public ImGuiManager
{

  public:
    D3D12ImGuiManager();

    ~D3D12ImGuiManager();

    bool init(ID3D12Device* device, int numFramesInFlight,
              DXGI_FORMAT rtvFormat, ID3D12DescriptorHeap* cbvSrvHeap,
              D3D12_CPU_DESCRIPTOR_HANDLE fontCpuDescHandle,
              D3D12_GPU_DESCRIPTOR_HANDLE fontGpuDeschandle);
    bool createDeviceObjects();
    
    void renderDrawData(ImDrawData* drawData,
                        ID3D12GraphicsCommandList* graphicsCommandList);


    void shutdown();
        
    void newFrame();

    void setupRenderState(ImDrawData* drawData, ID3D12GraphicsCommandList* ctx,
                          ImGuiD3D12RenderBuffers* fr);

    void invalidateDeviceObjects();

    void createFontTexture();
};
}
