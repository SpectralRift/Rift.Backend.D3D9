#include <Engine/Backend/D3D9/D3D9_Texture.hpp>
#include <Engine/Runtime/Logger.hpp>

#include <d3d9.h>
#include <d3dx9.h>

namespace engine::backend::dx9 {
    static runtime::Logger g_LoggerD3D9Texture("D3D9Texture");

    bool D3D9Texture::Create(const core::runtime::graphics::Bitmap &bitmap) {
        g_LoggerD3D9Texture.Log(runtime::LOG_LEVEL_DEBUG, "Creating texture %ix%i...", (unsigned int) bitmap.Size().x, (unsigned int) bitmap.Size().y);

        if (!m_Device || bitmap.GetPixels().empty()) {
            return false;
        }

        HRESULT hr = m_Device->CreateTexture(bitmap.Size().x, bitmap.Size().y, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_Texture, nullptr);
        if (FAILED(hr)) {
            g_LoggerD3D9Texture.Log(runtime::LOG_LEVEL_ERROR, "Failed to create texture. Error: 0x%08x", hr);
            return false;
        }

        D3DLOCKED_RECT lockedRect;
        if (SUCCEEDED(m_Texture->LockRect(0, &lockedRect, nullptr, 0))) {
            uint8_t *dst = static_cast<uint8_t *>(lockedRect.pBits);

            auto pixels = bitmap.GetPixels();

            for (size_t y = 0; y < bitmap.Size().y; ++y) {
                for (size_t x = 0; x < bitmap.Size().x; ++x) {
                    const auto &color = pixels[y * static_cast<size_t>(bitmap.Size().x) + x];
                    D3DCOLOR *pixel = reinterpret_cast<D3DCOLOR *>(lockedRect.pBits) + y * lockedRect.Pitch / 4 + x;
                    *pixel = D3DCOLOR_ARGB(color.a, color.r, color.g, color.b);
                }
            }

            m_Texture->UnlockRect(0);
        } else {
            g_LoggerD3D9Texture.Log(runtime::LOG_LEVEL_ERROR, "Failed to upload texture data!");
            return false;
        }

        g_LoggerD3D9Texture.Log(runtime::LOG_LEVEL_INFO, "Texture created and uploaded successfully!");

        return true;
    }

    void D3D9Texture::Destroy() {
        g_LoggerD3D9Texture.Log(runtime::LOG_LEVEL_DEBUG, "Texture is being destroyed.");

        if (m_Texture) {
            m_Texture->Release();
            m_Texture = nullptr;
        }
    }

    core::runtime::graphics::Bitmap D3D9Texture::Download() {
        // not able to download
        return {};
    }

    core::math::Vector2 D3D9Texture::GetSize() {
        if (!m_Texture) {
            return {0, 0};
        }

        D3DSURFACE_DESC desc;
        m_Texture->GetLevelDesc(0, &desc);
        return {static_cast<float>(desc.Width), static_cast<float>(desc.Height)};
    }

    void D3D9Texture::Bind(int samplerSlot) {
        if (m_Device && m_Texture) {
            m_Device->SetTexture(samplerSlot, m_Texture);

            m_Device->SetSamplerState(samplerSlot, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            m_Device->SetSamplerState(samplerSlot, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

            m_Device->SetSamplerState(samplerSlot, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            m_Device->SetSamplerState(samplerSlot, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            m_Device->SetSamplerState(samplerSlot, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
        }
    }

    void D3D9Texture::Unbind() {
        if (m_Device) {
            m_Device->SetTexture(0, nullptr);
        }
    }

}
