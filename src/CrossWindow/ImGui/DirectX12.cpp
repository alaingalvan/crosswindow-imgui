#include "DirectX12.h"
#include "DirectX12-Shaders.h"
#include "imgui.h"

// DirectX
#include <d3d12.h>
#include <dxgi1_4.h>

namespace xgfx
{

// DirectX data
struct ImGuiD3D12RenderBuffers;
struct ImGuiD3D12Data
{
    ID3D12Device* pd3dDevice;
    ID3D12RootSignature* pRootSignature;
    ID3D12PipelineState* pPipelineState;
    DXGI_FORMAT RTVFormat;
    ID3D12Resource* pFontTextureResource;
    D3D12_CPU_DESCRIPTOR_HANDLE hFontSrvCpuDescHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE hFontSrvGpuDescHandle;
    ID3D12DescriptorHeap* pd3dSrvDescHeap;
    UINT numFramesInFlight;

    ImGuiD3D12RenderBuffers* pFrameResources;
    UINT frameIndex;

    ImGuiD3D12Data()
    {
        memset((void*)this, 0, sizeof(*this));
        frameIndex = UINT_MAX;
    }
};

// Backend data stored in io.BackendRendererUserData to allow support for
// multiple Dear ImGui contexts It is STRONGLY preferred that you use docking
// branch with multi-viewports (== single Dear ImGui context + multiple windows)
// instead of multiple Dear ImGui contexts.
static ImGuiD3D12Data* GetBackendData()
{
    return ImGui::GetCurrentContext()
               ? (ImGuiD3D12Data*)ImGui::GetIO().BackendRendererUserData
               : nullptr;
}

// Buffers used during the rendering of a frame
struct ImGuiD3D12RenderBuffers
{
    ID3D12Resource* IndexBuffer;
    ID3D12Resource* VertexBuffer;
    int IndexBufferSize;
    int VertexBufferSize;
};

struct VERTEX_CONSTANT_BUFFER_DX12
{
    float mvp[4][4];
};

// Functions
void D3D12ImGuiManager::setupRenderState(ImDrawData* drawData,
                                         ID3D12GraphicsCommandList* ctx,
                                         ImGuiD3D12RenderBuffers* fr)
{
    ImGuiD3D12Data* bd = GetBackendData();

    // Setup orthographic projection matrix into our constant buffer
    // Our visible imgui space lies from drawData->DisplayPos (top left) to
    // drawData->DisplayPos+data_data->DisplaySize (bottom right).
    VERTEX_CONSTANT_BUFFER_DX12 vertex_constant_buffer;
    {
        float L = drawData->DisplayPos.x;
        float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        float T = drawData->DisplayPos.y;
        float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
        float mvp[4][4] = {
            {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
            {0.0f, 0.0f, 0.5f, 0.0f},
            {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
        };
        memcpy(&vertex_constant_buffer.mvp, mvp, sizeof(mvp));
    }

    // Setup viewport
    D3D12_VIEWPORT vp;
    memset(&vp, 0, sizeof(D3D12_VIEWPORT));
    vp.Width = drawData->DisplaySize.x * drawData->FramebufferScale.x;
    vp.Height = drawData->DisplaySize.y * drawData->FramebufferScale.y;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = vp.TopLeftY = 0.0f;
    ctx->RSSetViewports(1, &vp);

    // Bind shader and vertex buffers
    unsigned int stride = sizeof(ImDrawVert);
    unsigned int offset = 0;
    D3D12_VERTEX_BUFFER_VIEW vbv;
    memset(&vbv, 0, sizeof(D3D12_VERTEX_BUFFER_VIEW));
    vbv.BufferLocation = fr->VertexBuffer->GetGPUVirtualAddress() + offset;
    vbv.SizeInBytes = fr->VertexBufferSize * stride;
    vbv.StrideInBytes = stride;
    ctx->IASetVertexBuffers(0, 1, &vbv);
    D3D12_INDEX_BUFFER_VIEW ibv;
    memset(&ibv, 0, sizeof(D3D12_INDEX_BUFFER_VIEW));
    ibv.BufferLocation = fr->IndexBuffer->GetGPUVirtualAddress();
    ibv.SizeInBytes = fr->IndexBufferSize * sizeof(ImDrawIdx);
    ibv.Format =
        sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    ctx->IASetIndexBuffer(&ibv);
    ctx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->SetPipelineState(bd->pPipelineState);
    ctx->SetGraphicsRootSignature(bd->pRootSignature);
    ctx->SetGraphicsRoot32BitConstants(0, 16, &vertex_constant_buffer, 0);

    // Setup blend factor
    const float blend_factor[4] = {0.f, 0.f, 0.f, 0.f};
    ctx->OMSetBlendFactor(blend_factor);
}

template <typename T> static inline void SafeRelease(T*& res)
{
    if (res) res->Release();
    res = nullptr;
}

D3D12ImGuiManager::D3D12ImGuiManager() {}

D3D12ImGuiManager::~D3D12ImGuiManager() { invalidateDeviceObjects(); }

bool D3D12ImGuiManager::init(ID3D12Device* device, int numFramesInFlight,
                             DXGI_FORMAT rtvFormat,
                             ID3D12DescriptorHeap* cbvSrvHeap,
                             D3D12_CPU_DESCRIPTOR_HANDLE fontCpuDescHandle,
                             D3D12_GPU_DESCRIPTOR_HANDLE fontGpuDeschandle)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup CrossWindow event mapping:
    ImGuiManager::create();

    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr &&
              "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    ImGuiD3D12Data* bd = IM_NEW(ImGuiD3D12Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "imgui_crosswindow_dx12";
    io.BackendFlags |=
        ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the
                                                // ImDrawCmd::VtxOffset field,
                                                // allowing for large meshes.

    bd->pd3dDevice = device;
    bd->RTVFormat = rtvFormat;
    bd->hFontSrvCpuDescHandle = fontCpuDescHandle;
    bd->hFontSrvGpuDescHandle = fontGpuDeschandle;
    bd->pFrameResources = new ImGuiD3D12RenderBuffers[numFramesInFlight];
    bd->numFramesInFlight = numFramesInFlight;
    bd->pd3dSrvDescHeap = cbvSrvHeap;
    bd->frameIndex = UINT_MAX;

    // Create buffers with a default size (they will later be grown as needed)
    for (int i = 0; i < numFramesInFlight; i++)
    {
        ImGuiD3D12RenderBuffers* fr = &bd->pFrameResources[i];
        fr->IndexBuffer = nullptr;
        fr->VertexBuffer = nullptr;
        fr->IndexBufferSize = 10000;
        fr->VertexBufferSize = 5000;
    }

    return true;
}

void D3D12ImGuiManager::shutdown()
{
    ImGuiD3D12Data* bd = GetBackendData();
    IM_ASSERT(bd != nullptr &&
              "No renderer backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    // Clean up windows and device objects
    invalidateDeviceObjects();
    delete[] bd->pFrameResources;
    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    IM_DELETE(bd);
}
void D3D12ImGuiManager::newFrame()
{
    ImGuiD3D12Data* bd = GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call init()?");

    if (!bd->pPipelineState) createDeviceObjects();
}
void D3D12ImGuiManager::renderDrawData(
    ImDrawData* drawData, ID3D12GraphicsCommandList* graphicsCommandList)
{
    // Avoid rendering when minimized
    if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
        return;

    // FIXME: I'm assuming that this only gets called once per frame!
    // If not, we can't just re-allocate the IB or VB, we'll have to do a proper
    // allocator.
    ImGuiD3D12Data* bd = GetBackendData();
    bd->frameIndex = bd->frameIndex + 1;
    ImGuiD3D12RenderBuffers* fr =
        &bd->pFrameResources[bd->frameIndex % bd->numFramesInFlight];

    // Create and grow vertex/index buffers if needed
    if (fr->VertexBuffer == nullptr ||
        fr->VertexBufferSize < drawData->TotalVtxCount)
    {
        SafeRelease(fr->VertexBuffer);
        fr->VertexBufferSize = drawData->TotalVtxCount + 5000;
        D3D12_HEAP_PROPERTIES props;
        memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
        props.Type = D3D12_HEAP_TYPE_UPLOAD;
        props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        D3D12_RESOURCE_DESC desc;
        memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width = fr->VertexBufferSize * sizeof(ImDrawVert);
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        if (bd->pd3dDevice->CreateCommittedResource(
                &props, D3D12_HEAP_FLAG_NONE, &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&fr->VertexBuffer)) < 0)
            return;
    }
    if (fr->IndexBuffer == nullptr ||
        fr->IndexBufferSize < drawData->TotalIdxCount)
    {
        SafeRelease(fr->IndexBuffer);
        fr->IndexBufferSize = drawData->TotalIdxCount + 10000;
        D3D12_HEAP_PROPERTIES props;
        memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
        props.Type = D3D12_HEAP_TYPE_UPLOAD;
        props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        D3D12_RESOURCE_DESC desc;
        memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width = fr->IndexBufferSize * sizeof(ImDrawIdx);
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        if (bd->pd3dDevice->CreateCommittedResource(
                &props, D3D12_HEAP_FLAG_NONE, &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&fr->IndexBuffer)) < 0)
            return;
    }

    // Upload vertex/index data into a single contiguous GPU buffer
    void *vtx_resource, *idx_resource;
    D3D12_RANGE range;
    memset(&range, 0, sizeof(D3D12_RANGE));
    if (fr->VertexBuffer->Map(0, &range, &vtx_resource) != S_OK) return;
    if (fr->IndexBuffer->Map(0, &range, &idx_resource) != S_OK) return;
    ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource;
    ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource;
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = drawData->CmdLists[n];
        memcpy(vtx_dst, cmd_list->VtxBuffer.Data,
               cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idx_dst, cmd_list->IdxBuffer.Data,
               cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtx_dst += cmd_list->VtxBuffer.Size;
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    fr->VertexBuffer->Unmap(0, &range);
    fr->IndexBuffer->Unmap(0, &range);

    // Setup desired DX state
    setupRenderState(drawData, graphicsCommandList, fr);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own
    // offset into them)
    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    ImVec2 clip_off = drawData->DisplayPos;
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = drawData->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value
                // used by the user to request the renderer to reset render
                // state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    setupRenderState(drawData, graphicsCommandList, fr);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) *
                                    drawData->FramebufferScale.x,
                                (pcmd->ClipRect.y - clip_off.y) *
                                    drawData->FramebufferScale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) *
                                    drawData->FramebufferScale.x,
                                (pcmd->ClipRect.w - clip_off.y) *
                                    drawData->FramebufferScale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply Scissor/clipping rectangle, Bind texture, Draw
                const D3D12_RECT r = {(LONG)clip_min.x, (LONG)clip_min.y,
                                      (LONG)clip_max.x, (LONG)clip_max.y};
                D3D12_GPU_DESCRIPTOR_HANDLE texture_handle = {};
                texture_handle.ptr = (UINT64)pcmd->GetTexID();
                graphicsCommandList->SetGraphicsRootDescriptorTable(
                    1, texture_handle);
                graphicsCommandList->RSSetScissorRects(1, &r);
                graphicsCommandList->DrawIndexedInstanced(
                    pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset,
                    pcmd->VtxOffset + global_vtx_offset, 0);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }
}

void D3D12ImGuiManager::invalidateDeviceObjects()
{
    ImGuiD3D12Data* bd = GetBackendData();
    if (!bd || !bd->pd3dDevice) return;

    ImGuiIO& io = ImGui::GetIO();
    SafeRelease(bd->pRootSignature);
    SafeRelease(bd->pPipelineState);
    SafeRelease(bd->pFontTextureResource);
    io.Fonts->SetTexID(0); // We copied bd->pFontTextureView to io.Fonts->TexID
                           // so let's clear that as well.

    for (UINT i = 0; i < bd->numFramesInFlight; i++)
    {
        ImGuiD3D12RenderBuffers* fr = &bd->pFrameResources[i];
        SafeRelease(fr->IndexBuffer);
        SafeRelease(fr->VertexBuffer);
    }
}
bool D3D12ImGuiManager::createDeviceObjects()
{
    ImGuiD3D12Data* bd = GetBackendData();
    if (!bd || !bd->pd3dDevice) return false;
    if (bd->pPipelineState) invalidateDeviceObjects();

    // Create the root signature
    {
        D3D12_DESCRIPTOR_RANGE1 descRange = {};
        descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRange.NumDescriptors = 1;
        descRange.BaseShaderRegister = 0;
        descRange.RegisterSpace = 0;
        descRange.OffsetInDescriptorsFromTableStart = 0;

        D3D12_ROOT_PARAMETER1 param[2] = {};

        param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.ShaderRegister = 0;
        param[0].Constants.RegisterSpace = 0;
        param[0].Constants.Num32BitValues = 16;
        param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges = 1;
        param[1].DescriptorTable.pDescriptorRanges = &descRange;
        param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // Bilinear sampling is required by default. Set 'io.Fonts->Flags |=
        // ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex =
        // false' to allow point/nearest sampling.
        D3D12_STATIC_SAMPLER_DESC staticSampler = {};
        staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        staticSampler.MipLODBias = 0.f;
        staticSampler.MaxAnisotropy = 0;
        staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        staticSampler.MinLOD = 0.f;
        staticSampler.MaxLOD = 0.f;
        staticSampler.ShaderRegister = 0;
        staticSampler.RegisterSpace = 0;
        staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // TODO load serialized root signature...
        D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        rootSignatureDesc.Desc_1_1.Flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
        rootSignatureDesc.Desc_1_1.NumParameters = _countof(param);
        rootSignatureDesc.Desc_1_1.pParameters = param;
        rootSignatureDesc.Desc_1_1.NumStaticSamplers = 1;
        rootSignatureDesc.Desc_1_1.pStaticSamplers = &staticSampler;

        ID3DBlob* signature;
        ID3DBlob* error;

        if (D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature,
                                                 &error) != S_OK)
        {
            error->Release();
            return false;
        }

        bd->pd3dDevice->CreateRootSignature(0, signature->GetBufferPointer(),
                                            signature->GetBufferSize(),
                                            IID_PPV_ARGS(&bd->pRootSignature));
        signature->Release();
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    memset(&psoDesc, 0, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.NodeMask = 1;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.pRootSignature = bd->pRootSignature;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = bd->RTVFormat;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // Create the vertex shader
    {
        psoDesc.VS = {xgfx::imguiD3D12VertexShader,
                      IM_ARRAYSIZE(xgfx::imguiD3D12VertexShader)};

        // Create the input layout
        static D3D12_INPUT_ELEMENT_DESC local_layout[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
             (UINT)IM_OFFSETOF(ImDrawVert, pos),
             D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
             (UINT)IM_OFFSETOF(ImDrawVert, uv),
             D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0,
             (UINT)IM_OFFSETOF(ImDrawVert, col),
             D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };
        psoDesc.InputLayout = {local_layout, 3};
    }

    // Create the pixel shader
    {
        psoDesc.PS = {xgfx::imguiD3D12PixelShader,
                      IM_ARRAYSIZE(xgfx::imguiD3D12PixelShader)};
    }

    // Create the blending setup
    {
        D3D12_BLEND_DESC& desc = psoDesc.BlendState;
        desc.AlphaToCoverageEnable = false;
        desc.RenderTarget[0].BlendEnable = true;
        desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask =
            D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    // Create the rasterizer state
    {
        D3D12_RASTERIZER_DESC& desc = psoDesc.RasterizerState;
        desc.FillMode = D3D12_FILL_MODE_SOLID;
        desc.CullMode = D3D12_CULL_MODE_NONE;
        desc.FrontCounterClockwise = FALSE;
        desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        desc.DepthClipEnable = true;
        desc.MultisampleEnable = FALSE;
        desc.AntialiasedLineEnable = FALSE;
        desc.ForcedSampleCount = 0;
        desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    }

    // Create depth-stencil State
    {
        D3D12_DEPTH_STENCIL_DESC& desc = psoDesc.DepthStencilState;
        desc.DepthEnable = false;
        desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        desc.StencilEnable = false;
        desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp =
            desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        desc.BackFace = desc.FrontFace;
    }

    HRESULT result_pipeline_state = bd->pd3dDevice->CreateGraphicsPipelineState(
        &psoDesc, IID_PPV_ARGS(&bd->pPipelineState));

    if (result_pipeline_state != S_OK) return false;

    createFontTexture();

    return true;
}

void D3D12ImGuiManager::createFontTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    ImGuiD3D12Data* bd = GetBackendData();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Upload texture to graphics system
    {
        D3D12_HEAP_PROPERTIES props;
        memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
        props.Type = D3D12_HEAP_TYPE_DEFAULT;
        props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        D3D12_RESOURCE_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Alignment = 0;
        desc.Width = width;
        desc.Height = height;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        ID3D12Resource* pTexture = nullptr;
        bd->pd3dDevice->CreateCommittedResource(
            &props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr, IID_PPV_ARGS(&pTexture));

        UINT uploadPitch =
            (width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) &
            ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
        UINT uploadSize = height * uploadPitch;
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = uploadSize;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        props.Type = D3D12_HEAP_TYPE_UPLOAD;
        props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        ID3D12Resource* uploadBuffer = nullptr;
        HRESULT hr = bd->pd3dDevice->CreateCommittedResource(
            &props, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&uploadBuffer));
        IM_ASSERT(SUCCEEDED(hr));

        void* mapped = nullptr;
        D3D12_RANGE range = {0, uploadSize};
        hr = uploadBuffer->Map(0, &range, &mapped);
        IM_ASSERT(SUCCEEDED(hr));
        for (int y = 0; y < height; y++)
            memcpy((void*)((uintptr_t)mapped + y * uploadPitch),
                   pixels + y * width * 4, width * 4);
        uploadBuffer->Unmap(0, &range);

        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.pResource = uploadBuffer;
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcLocation.PlacedFootprint.Footprint.Format =
            DXGI_FORMAT_R8G8B8A8_UNORM;
        srcLocation.PlacedFootprint.Footprint.Width = width;
        srcLocation.PlacedFootprint.Footprint.Height = height;
        srcLocation.PlacedFootprint.Footprint.Depth = 1;
        srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

        D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
        dstLocation.pResource = pTexture;
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = 0;

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = pTexture;
        barrier.Transition.Subresource =
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter =
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        ID3D12Fence* fence = nullptr;
        hr = bd->pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                         IID_PPV_ARGS(&fence));
        IM_ASSERT(SUCCEEDED(hr));

        HANDLE event = CreateEvent(0, 0, 0, 0);
        IM_ASSERT(event != nullptr);

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 1;

        ID3D12CommandQueue* cmdQueue = nullptr;
        hr = bd->pd3dDevice->CreateCommandQueue(&queueDesc,
                                                IID_PPV_ARGS(&cmdQueue));
        IM_ASSERT(SUCCEEDED(hr));

        ID3D12CommandAllocator* cmdAlloc = nullptr;
        hr = bd->pd3dDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
        IM_ASSERT(SUCCEEDED(hr));

        ID3D12GraphicsCommandList* cmdList = nullptr;
        hr = bd->pd3dDevice->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, nullptr,
            IID_PPV_ARGS(&cmdList));
        IM_ASSERT(SUCCEEDED(hr));

        cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation,
                                   nullptr);
        cmdList->ResourceBarrier(1, &barrier);

        hr = cmdList->Close();
        IM_ASSERT(SUCCEEDED(hr));

        cmdQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&cmdList);
        hr = cmdQueue->Signal(fence, 1);
        IM_ASSERT(SUCCEEDED(hr));

        fence->SetEventOnCompletion(1, event);
        WaitForSingleObject(event, INFINITE);

        cmdList->Release();
        cmdAlloc->Release();
        cmdQueue->Release();
        CloseHandle(event);
        fence->Release();
        uploadBuffer->Release();

        // Create texture view
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        bd->pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc,
                                                 bd->hFontSrvCpuDescHandle);
        SafeRelease(bd->pFontTextureResource);
        bd->pFontTextureResource = pTexture;
    }

    // Store our identifier
    // READ THIS IF THE STATIC_ASSERT() TRIGGERS:
    // - Important: to compile on 32-bit systems, this backend requires code to
    // be compiled with '#define ImTextureID ImU64'.
    // - This is because we need ImTextureID to carry a 64-bit value and by
    // default ImTextureID is defined as void*. [Solution 1] IDE/msbuild: in
    // "Properties/C++/Preprocessor Definitions" add 'ImTextureID=ImU64' (this
    // is what we do in the
    // 'example_win32_direct12/example_win32_direct12.vcxproj' project file)
    // [Solution 2] IDE/msbuild: in "Properties/C++/Preprocessor Definitions"
    // add 'IMGUI_USER_CONFIG="my_imgui_config.h"' and inside
    // 'my_imgui_config.h' add '#define ImTextureID ImU64' and as many other
    // options as you like. [Solution 3] IDE/msbuild: edit imconfig.h and add
    // '#define ImTextureID ImU64' (prefer solution 2 to create your own config
    // file!) [Solution 4] command-line: add '/D ImTextureID=ImU64' to your
    // cl.exe command-line (this is what we do in the
    // example_win32_direct12/build_win32.bat file)
    static_assert(
        sizeof(ImTextureID) >= sizeof(bd->hFontSrvGpuDescHandle.ptr),
        "Can't pack descriptor handle into TexID, 32-bit not supported yet.");
    io.Fonts->SetTexID((ImTextureID)bd->hFontSrvGpuDescHandle.ptr);
}

}