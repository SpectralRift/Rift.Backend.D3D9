#include <Engine/Backend/DX9/DX9_VertexBuffer.hpp>

#define D3D_DEBUG_INFO

#include <d3d9.h>
#include <d3dx9.h>

namespace engine::backend::dx9 {
    static D3DVERTEXELEMENT9 DX9_VertexDeclList[] = {
            {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0}, // position
            {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // uv
            {0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},   // normal
            {0, 32, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},  // color
            D3DDECL_END()
    };

    static IDirect3DVertexDeclaration9 *DX9_VertexDecl = nullptr;

    D3DPRIMITIVETYPE DX9_ConvertPrimitiveType(core::runtime::graphics::PrimitiveType type) {
        switch (type) {
            case core::runtime::graphics::PrimitiveType::PRIMITIVE_TYPE_TRIANGLES:
                return D3DPT_TRIANGLELIST;
            case core::runtime::graphics::PrimitiveType::PRIMITIVE_TYPE_LINES:
                return D3DPT_LINELIST;
            case core::runtime::graphics::PrimitiveType::PRIMITIVE_TYPE_POINTS:
                return D3DPT_POINTLIST;
            default:
                printf("DX9_ConvertPrimitiveType: Unknown primitive type! Defaulting to D3DPT_TRIANGLELIST\n");
                return D3DPT_TRIANGLELIST;
        }
    }

    size_t DX9VertexBuffer::GetPrimitiveCount() const {
        switch (m_PrimType) {
            case core::runtime::graphics::PrimitiveType::PRIMITIVE_TYPE_TRIANGLES:
                return m_VertexCount / 3;
            case core::runtime::graphics::PrimitiveType::PRIMITIVE_TYPE_LINES:
                return m_VertexCount / 2;
            case core::runtime::graphics::PrimitiveType::PRIMITIVE_TYPE_POINTS:
                return m_VertexCount;
            default:
                return 0;
        }
    }

    bool DX9VertexBuffer::Create() {
        if (!m_Device) return false;
        return true;
    }

    void DX9VertexBuffer::Destroy() {
        if (m_VertexBuffer) {
            m_VertexBuffer->Release();
            m_VertexBuffer = nullptr;
        }
    }

    void DX9VertexBuffer::Bind() {

    }

    void DX9VertexBuffer::Unbind() {
        m_Device->SetStreamSource(0, nullptr, 0, 0);
    }

    void DX9VertexBuffer::Draw() {
        if (m_VertexBuffer && m_VertexCount > 0) {
            // set vertex format for DX
            if (DX9_VertexDecl == nullptr) {
                m_Device->CreateVertexDeclaration(DX9_VertexDeclList, &DX9_VertexDecl);
            }

            m_Device->SetVertexDeclaration(DX9_VertexDecl);
            m_Device->SetStreamSource(0, m_VertexBuffer, 0, sizeof(core::runtime::graphics::Vertex));
            m_Device->DrawPrimitive(DX9_ConvertPrimitiveType(m_PrimType), 0, GetPrimitiveCount());
        }
    }

    void DX9VertexBuffer::Upload(
            const std::vector<core::runtime::graphics::Vertex> &data,
            core::runtime::graphics::PrimitiveType type,
            core::runtime::graphics::BufferUsageHint usage
    ) {
        if (m_VertexBuffer && m_VertexCount > data.size()) {
            Destroy();
        }

        m_VertexCount = data.size();
        m_PrimType = type;
        m_UsageHint = usage;

        if (m_VertexCount == 0) return;

        const size_t bufferSize = m_VertexCount * sizeof(core::runtime::graphics::Vertex);
        HRESULT hr;

        if (m_VertexBuffer == nullptr) {
            DWORD dxUsage = D3DUSAGE_WRITEONLY;

            if(usage == core::runtime::graphics::BufferUsageHint::BUFFER_USAGE_HINT_DYNAMIC) {
                dxUsage |= D3DUSAGE_DYNAMIC;
            }

            hr = m_Device->CreateVertexBuffer(
                    bufferSize,
                    dxUsage,
                    0,
                    D3DPOOL_MANAGED,
                    &m_VertexBuffer,
                    nullptr
            );

            if (FAILED(hr)) {
                m_VertexBuffer = nullptr;
                return;
            }
        }

        core::runtime::graphics::Vertex *vertexData;
        hr = m_VertexBuffer->Lock(0, bufferSize, reinterpret_cast<void **>(&vertexData), usage == core::runtime::graphics::BufferUsageHint::BUFFER_USAGE_HINT_DYNAMIC ? D3DLOCK_DISCARD : D3DLOCK_NOSYSLOCK);

        if (SUCCEEDED(hr)) {
            memcpy(vertexData, data.data(), bufferSize);
            m_VertexBuffer->Unlock();
        }
    }

    size_t DX9VertexBuffer::Size() {
        return m_VertexCount;
    }

    core::runtime::graphics::PrimitiveType DX9VertexBuffer::GetPrimitiveType() {
        return m_PrimType;
    }

    std::vector<core::runtime::graphics::Vertex> DX9VertexBuffer::Download() {
        std::vector<core::runtime::graphics::Vertex> result;
        if (!m_VertexBuffer || m_VertexCount == 0) return result;

        const size_t bufferSize = m_VertexCount * sizeof(core::runtime::graphics::Vertex);
        result.resize(m_VertexCount);

        void *vertexData;
        HRESULT hr = m_VertexBuffer->Lock(0, bufferSize, &vertexData, D3DLOCK_READONLY);

        if (SUCCEEDED(hr)) {
            memcpy(result.data(), vertexData, bufferSize);
            m_VertexBuffer->Unlock();
        }

        return result;
    }
}