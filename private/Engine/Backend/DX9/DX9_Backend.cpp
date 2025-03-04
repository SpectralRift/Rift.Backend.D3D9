#include <d3d9.h>
#include <stdexcept>
#include <sstream>

#include <Engine/Backend/DX9/DX9_Backend.hpp>
#include <Engine/Backend/DX9/DX9_Shader.hpp>
#include <Engine/Backend/DX9/DX9_ShaderProgram.hpp>
#include <Engine/Backend/DX9/DX9_VertexBuffer.hpp>
#include <Engine/Backend/DX9/DX9_Texture.hpp>
#include <Engine/Runtime/Logger.hpp>

namespace engine::backend::dx9 {
    static runtime::Logger g_LoggerDX9Backend("D3D9Backend");

    bool DX9Backend::Initialize() {
        if (!h_DX9Device) {
            g_LoggerDX9Backend.Log(runtime::LOG_LEVEL_ERROR, "device is NULL during initialization.");
            return false;
        }

        g_LoggerDX9Backend.Log(runtime::LOG_LEVEL_INFO, "DX9 backend initialized!");

        return true;
    }

    void DX9Backend::Shutdown() {
        h_DX9Device = nullptr;
    }

    std::string DX9Backend::GetName() const {
        return "DirectX 9";
    }

    std::string DX9Backend::GetIdentifier() const {
        return "dx9";
    }

    void DX9Backend::SetViewport(core::math::Vector2 pos, core::math::Vector2 size) {
        if (!h_DX9Device) {
            g_LoggerDX9Backend.Log(runtime::LOG_LEVEL_ERROR, "Cannot set viewport, device is not initialized.");
        }

        D3DVIEWPORT9 viewport = {};
        viewport.X = static_cast<DWORD>(pos.x);
        viewport.Y = static_cast<DWORD>(pos.y);
        viewport.Width = static_cast<DWORD>(size.x);
        viewport.Height = static_cast<DWORD>(size.y);
        viewport.MinZ = 0.0f;
        viewport.MaxZ = 1.0f;

        HRESULT hr = h_DX9Device->SetViewport(&viewport);
        if (FAILED(hr)) {
            g_LoggerDX9Backend.Log(runtime::LOG_LEVEL_ERROR, "Failed to set viewport! Error: 0x%08x", hr);
            return;
        }
    }

    void DX9Backend::SetScissor(core::math::Vector2 start, core::math::Vector2 size) {
        if (!h_DX9Device) {
            g_LoggerDX9Backend.Log(runtime::LOG_LEVEL_ERROR, "Cannot set scissor, device is not initialized.");
            return;
        }

        RECT scissorRect;
        scissorRect.left = static_cast<LONG>(start.x);
        scissorRect.top = static_cast<LONG>(start.y);
        scissorRect.right = static_cast<LONG>(start.x + size.x);
        scissorRect.bottom = static_cast<LONG>(start.y + size.y);

        HRESULT hr = h_DX9Device->SetScissorRect(&scissorRect);
        if (FAILED(hr)) {
            g_LoggerDX9Backend.Log(runtime::LOG_LEVEL_ERROR, "Failed to set scissor rect.");
            return;
        }
    }

    void DX9Backend::Clear(core::runtime::graphics::Color color) {
        if (!h_DX9Device) {
            g_LoggerDX9Backend.Log(runtime::LOG_LEVEL_ERROR, "Cannot clear, device is not initialized.");
            return;
        }

        D3DCOLOR dxColor = D3DCOLOR_ARGB(
                color.a,
                color.r,
                color.g,
                color.b
        );

        DWORD renderState;

        // configure clockwise culling in order to allow stuff to render properly;
        h_DX9Device->GetRenderState(D3DRS_CULLMODE, &renderState);
        if (renderState != D3DCULL_NONE) {
            h_DX9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        }

        // disable clipping
        h_DX9Device->GetRenderState(D3DRS_CLIPPING, &renderState);
        if (renderState != FALSE) {
            h_DX9Device->SetRenderState(D3DRS_CLIPPING, FALSE);
        }

        // disable lighting
        h_DX9Device->GetRenderState(D3DRS_LIGHTING, &renderState);
        if (renderState != FALSE) {
            h_DX9Device->SetRenderState(D3DRS_LIGHTING, FALSE);
        }

        // disable zbuffer
        h_DX9Device->GetRenderState(D3DRS_ZENABLE, &renderState);
        if (renderState != FALSE) {
            h_DX9Device->SetRenderState(D3DRS_ZENABLE, FALSE);
        }

        HRESULT hr = h_DX9Device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, dxColor, 1.0f, 0);
        if (FAILED(hr)) {
            g_LoggerDX9Backend.Log(runtime::LOG_LEVEL_ERROR, "Failed to clear the render target.");
            return;
        }
    }

    void DX9Backend::EnableFeatures(core::runtime::graphics::BackendFeature featuresMask) {
        DWORD renderState;

        if (featuresMask & core::runtime::graphics::BACKEND_FEATURE_SCISSOR_TEST) {
            h_DX9Device->GetRenderState(D3DRS_SCISSORTESTENABLE, &renderState);
            if (renderState != TRUE) {
                h_DX9Device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
            }
        }

        if (featuresMask & core::runtime::graphics::BACKEND_FEATURE_ALPHA_BLENDING) {
            h_DX9Device->GetRenderState(D3DRS_ALPHABLENDENABLE, &renderState);
            if (renderState != TRUE) {
                h_DX9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            }

            h_DX9Device->GetRenderState(D3DRS_BLENDOP, &renderState);
            if (renderState != D3DBLENDOP_ADD) {
                h_DX9Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
            }

            h_DX9Device->GetRenderState(D3DRS_SRCBLEND, &renderState);
            if (renderState != D3DBLEND_SRCALPHA) {
                h_DX9Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            }

            h_DX9Device->GetRenderState(D3DRS_DESTBLEND, &renderState);
            if (renderState != D3DBLEND_INVSRCALPHA) {
                h_DX9Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            }
        }

        m_ActiveFeatures |= featuresMask;
    }

    void DX9Backend::DisableFeatures(core::runtime::graphics::BackendFeature featuresMask) {
        DWORD renderState;

        if ((featuresMask & core::runtime::graphics::BACKEND_FEATURE_SCISSOR_TEST) &&
            (m_ActiveFeatures & core::runtime::graphics::BACKEND_FEATURE_SCISSOR_TEST)) {
            h_DX9Device->GetRenderState(D3DRS_SCISSORTESTENABLE, &renderState);
            if (renderState != FALSE) {
                h_DX9Device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
            }
        }

        if ((featuresMask & core::runtime::graphics::BACKEND_FEATURE_ALPHA_BLENDING) &&
            (m_ActiveFeatures & core::runtime::graphics::BACKEND_FEATURE_ALPHA_BLENDING)) {
            h_DX9Device->GetRenderState(D3DRS_ALPHABLENDENABLE, &renderState);
            if (renderState != FALSE) {
                h_DX9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
            }
        }

        m_ActiveFeatures &= ~featuresMask;
    }

    core::runtime::graphics::BackendFeature DX9Backend::GetActiveFeatures() {
        return static_cast<core::runtime::graphics::BackendFeature>(m_ActiveFeatures);
    }

    std::unique_ptr<core::runtime::graphics::IVertexBuffer> DX9Backend::CreateVertexBuffer() {
        return std::make_unique<DX9VertexBuffer>(h_DX9Device);
    }

    std::unique_ptr<core::runtime::graphics::IShader> DX9Backend::CreateShader() {
        return std::make_unique<DX9Shader>(h_DX9Device);
    }

    std::unique_ptr<core::runtime::graphics::IShaderProgram> DX9Backend::CreateShaderProgram() {
        return std::make_unique<DX9ShaderProgram>(h_DX9Device);
    }

    std::unique_ptr<core::runtime::graphics::ITexture> DX9Backend::CreateTexture() {
        return std::make_unique<DX9Texture>(h_DX9Device);
    }
}