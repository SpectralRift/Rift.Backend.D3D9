#pragma once

#include <Engine/Core/Runtime/Graphics/ITexture.hpp>

// forward definition of DX9 types
struct IDirect3DDevice9;
struct IDirect3DTexture9;

namespace engine::backend::dx9 {
    struct DX9Texture : public core::runtime::graphics::ITexture {
        DX9Texture(IDirect3DDevice9* device) : m_Device(device), m_Texture(nullptr) {}

        bool Create(const core::runtime::graphics::Bitmap& bitmap) override;

        void Destroy() override;

        core::runtime::graphics::Bitmap Download() override;

        core::math::Vector2 GetSize() override;

        void Bind(int samplerSlot) override;

        void Unbind() override;

        IDirect3DTexture9* GetHandle() const {
            return m_Texture;
        }

    protected:
        IDirect3DDevice9* m_Device;
        IDirect3DTexture9* m_Texture;
    };
}
