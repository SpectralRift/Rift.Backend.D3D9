#include <Engine/Backend/D3D9/D3D9_VertexBuffer.hpp>
#include <Engine/Runtime/Logger.hpp>

#include <d3d9.h>
#include <d3dx9.h>

namespace engine::backend::dx9 {
    static D3DVERTEXELEMENT9 D3D9_VertexDeclList[] = {
            {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0}, // position
            {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // uv
            {0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},   // normal
            {0, 32, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},  // color
            D3DDECL_END()
    };

    static IDirect3DVertexDeclaration9 *D3D9_VertexDecl = nullptr;

    static runtime::Logger g_LoggerD3D9VertexBuffer("D3D9VertexBuffer");

    D3DPRIMITIVETYPE D3D9_ConvertPrimitiveType(core::runtime::graphics::PrimitiveType type) {
        switch (type) {
            default:
            case core::runtime::graphics::PrimitiveType::PRIMITIVE_TYPE_TRIANGLES:
                return D3DPT_TRIANGLELIST;
            case core::runtime::graphics::PrimitiveType::PRIMITIVE_TYPE_LINES:
                return D3DPT_LINELIST;
            case core::runtime::graphics::PrimitiveType::PRIMITIVE_TYPE_POINTS:
                return D3DPT_POINTLIST;
        }
    }

    size_t D3D9VertexBuffer::GetPrimitiveCount() const {
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

    bool D3D9VertexBuffer::Create() {
        if (!m_Device) {
            g_LoggerD3D9VertexBuffer.Log(runtime::LOG_LEVEL_ERROR, "Device is NULL.");
            return false;
        }

        g_LoggerD3D9VertexBuffer.Log(runtime::LOG_LEVEL_INFO, "Vertex Buffer created.");
        return true;
    }

    void D3D9VertexBuffer::Destroy() {
        g_LoggerD3D9VertexBuffer.Log(runtime::LOG_LEVEL_DEBUG, "This vertex buffer is being destroyed.");

        if (m_VertexBuffer) {
            m_VertexBuffer->Release();
            m_VertexBuffer = nullptr;
        }

        m_BufferCapacity = 0;
    }

    void D3D9VertexBuffer::Bind() {

    }

    void D3D9VertexBuffer::Unbind() {
        m_Device->SetStreamSource(0, nullptr, 0, 0);
    }

    void D3D9VertexBuffer::Draw() {
        if (m_VertexBuffer && m_VertexCount > 0) {
            // set vertex format for DX
            if (D3D9_VertexDecl == nullptr) {
                m_Device->CreateVertexDeclaration(D3D9_VertexDeclList, &D3D9_VertexDecl);
            }

            m_Device->SetVertexDeclaration(D3D9_VertexDecl);
            m_Device->SetStreamSource(0, m_VertexBuffer, 0, sizeof(core::runtime::graphics::Vertex));

            HRESULT hr = m_Device->DrawPrimitive(D3D9_ConvertPrimitiveType(m_PrimType), 0, GetPrimitiveCount());

            if(FAILED(hr)) {
                g_LoggerD3D9VertexBuffer.Log(runtime::LOG_LEVEL_ERROR, "Failed to draw vertex buffer. Error: 0x%08x", hr);
            }
        }
    }

    void D3D9VertexBuffer::Upload(
            const std::vector<core::runtime::graphics::Vertex> &data,
            core::runtime::graphics::PrimitiveType type,
            core::runtime::graphics::BufferUsageHint usage
    ) {
        if (m_VertexBuffer && data.size() > m_BufferCapacity) {
            g_LoggerD3D9VertexBuffer.Log(runtime::LOG_LEVEL_WARNING, "New vertex data exceeds buffer capacity. The buffer will be recreated!");
            Destroy();
        }

        m_VertexCount = data.size();
        m_PrimType = type;
        m_UsageHint = usage;

        auto isDynamicUsage = usage == core::runtime::graphics::BufferUsageHint::BUFFER_USAGE_HINT_DYNAMIC || usage == core::runtime::graphics::BufferUsageHint::BUFFER_USAGE_HINT_STREAM;

        if (m_VertexCount == 0) return;

        const size_t bufferSize = data.size() * sizeof(core::runtime::graphics::Vertex);
        HRESULT hr;

        if (m_VertexBuffer == nullptr) {
            DWORD dxUsage = D3DUSAGE_WRITEONLY;

            if(isDynamicUsage) {
                dxUsage |= D3DUSAGE_DYNAMIC;
            }

            hr = m_Device->CreateVertexBuffer(
                    bufferSize,
                    dxUsage,
                    0,
                    D3DPOOL_DEFAULT,
                    &m_VertexBuffer,
                    nullptr
            );

            if (FAILED(hr)) {
                g_LoggerD3D9VertexBuffer.Log(runtime::LOG_LEVEL_ERROR, "Failed to create vertex buffer! Error: 0x%08x", hr);
                m_VertexBuffer = nullptr;
                return;
            } else {
                m_BufferCapacity = data.size();
                g_LoggerD3D9VertexBuffer.Log(runtime::LOG_LEVEL_INFO, "Vertex buffer created successfully");
            }
        }

        core::runtime::graphics::Vertex *vertexData;
        hr = m_VertexBuffer->Lock(0, bufferSize, reinterpret_cast<void **>(&vertexData), D3DLOCK_DISCARD);

        if (SUCCEEDED(hr)) {
            memcpy(vertexData, data.data(), bufferSize);
            m_VertexBuffer->Unlock();
        }
    }

    size_t D3D9VertexBuffer::Size() {
        return m_VertexCount;
    }

    core::runtime::graphics::PrimitiveType D3D9VertexBuffer::GetPrimitiveType() {
        return m_PrimType;
    }

    std::vector<core::runtime::graphics::Vertex> D3D9VertexBuffer::Download() {
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