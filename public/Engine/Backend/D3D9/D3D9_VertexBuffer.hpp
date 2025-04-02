#pragma once

#include <Engine/Core/Runtime/Graphics/IVertexBuffer.hpp>

// forward definition of D3D9 types
struct IDirect3DDevice9;
struct IDirect3DVertexBuffer9;

namespace engine::backend::dx9 {
    struct D3D9VertexBuffer : public core::runtime::graphics::IVertexBuffer {
        D3D9VertexBuffer(IDirect3DDevice9 *device) :
                m_Device{device},
                m_VertexBuffer{nullptr},
                m_VertexCount{0},
                m_UsageHint{core::runtime::graphics::BufferUsageHint::BUFFER_USAGE_HINT_STATIC},
                m_PrimType{core::runtime::graphics::PrimitiveType::PRIMITIVE_TYPE_TRIANGLES} {}

        bool Create() override;

        void Destroy() override;

        void Bind() override;

        void Unbind() override;

        void Draw() override;

        void Upload(
                const std::vector<core::runtime::graphics::Vertex> &data,
                core::runtime::graphics::PrimitiveType type,
                core::runtime::graphics::BufferUsageHint usage
        ) override;

        size_t Size() override;

        core::runtime::graphics::PrimitiveType GetPrimitiveType() override;

        std::vector<core::runtime::graphics::Vertex> Download() override;

    protected:
        size_t GetPrimitiveCount() const;

        IDirect3DDevice9 *m_Device;
        IDirect3DVertexBuffer9 *m_VertexBuffer;
        size_t m_VertexCount;
        size_t m_BufferCapacity;
        core::runtime::graphics::BufferUsageHint m_UsageHint;
        core::runtime::graphics::PrimitiveType m_PrimType;
    };
}