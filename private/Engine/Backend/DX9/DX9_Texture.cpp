#include <Engine/Backend/DX9/DX9_Texture.hpp>

#define D3D_DEBUG_INFO

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9tex.h>

namespace engine::backend::dx9 {
    bool DX9Texture::Create(const core::runtime::graphics::Bitmap &bitmap) {
        if (!m_Device || bitmap.GetPixels().empty()) {
            return false;
        }

        HRESULT hr = D3DXCreateTexture(m_Device, bitmap.Size().x, bitmap.Size().y, 1, 0, D3DFMT_A8R8G8B8,
                                       D3DPOOL_MANAGED, &m_Texture);
        if (FAILED(hr)) {
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
        }

        return true;
    }

    void DX9Texture::Destroy() {
        if (m_Texture) {
            m_Texture->Release();
            m_Texture = nullptr;
        }
    }

    core::runtime::graphics::Bitmap DX9Texture::Download() {
        // not able to download
        return {};
    }

    core::math::Vector2 DX9Texture::GetSize() {
        if (!m_Texture) {
            return {0, 0};
        }

        D3DSURFACE_DESC desc;
        m_Texture->GetLevelDesc(0, &desc);
        return {static_cast<float>(desc.Width), static_cast<float>(desc.Height)};
    }

    void DX9Texture::Bind(int samplerSlot) {
        if (m_Device && m_Texture) {
            m_Device->SetTexture(samplerSlot, m_Texture);
        }
    }

    void DX9Texture::Unbind() {
        if (m_Device) {
            m_Device->SetTexture(0, nullptr);
        }
    }

}
