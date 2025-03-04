#include <Engine/Backend/DX9/DX9_Shader.hpp>

#include <d3d9.h>
#include <d3dx9.h>

namespace engine::backend::dx9 {
    void DX9Shader::Destroy() {
        if (m_ErrorBuffer) {
            m_ErrorBuffer->Release();
            m_ErrorBuffer = nullptr;
        }

        if(m_CompiledShader) {
            m_CompiledShader->Release();
            m_CompiledShader = nullptr;
        }

        if(m_ShaderHandle) {
            if (m_ShaderType == core::runtime::graphics::ShaderType::SHADER_TYPE_VERTEX) {
                reinterpret_cast<IDirect3DVertexShader9 *>(m_ShaderHandle)->Release();
            } else if (m_ShaderType == core::runtime::graphics::ShaderType::SHADER_TYPE_FRAGMENT) {
                reinterpret_cast<IDirect3DPixelShader9 *>(m_ShaderHandle)->Release();
            }
        }
    }

    void DX9Shader::SetSource(std::string_view source, core::runtime::graphics::ShaderType type) {
        m_SourceCode = source;
        m_ShaderType = type;
    }

    const char *DX9_GetProfile(core::runtime::graphics::ShaderType type) {
        switch (type) {
            case core::runtime::graphics::ShaderType::SHADER_TYPE_VERTEX:
                return "vs_3_0";
            case core::runtime::graphics::ShaderType::SHADER_TYPE_FRAGMENT:
                return "ps_3_0";
            default:
                printf("DX9_GetProfile: Unknown shader type. Defaulting to 'vs_3_0'\n");
                return "vs_3_0";
        }
    }

    bool DX9Shader::UseCompiledShader(const std::span<unsigned char> &data, core::runtime::graphics::ShaderType type) {
        if (data.empty()) {
            printf("DX9Shader: Shader code is empty.\n");
            return false;
        }

        m_ShaderType = type;

        // cleanup resources if compiled
        if(IsCompiled()) {
            Destroy();
        }

        HRESULT hr;

        if (type == core::runtime::graphics::ShaderType::SHADER_TYPE_VERTEX) {
            hr = m_Device->CreateVertexShader(
                    reinterpret_cast<const DWORD*>(data.data()),
                    reinterpret_cast<IDirect3DVertexShader9 **>(&m_ShaderHandle)
            );
        } else if (type == core::runtime::graphics::ShaderType::SHADER_TYPE_FRAGMENT) {
            hr = m_Device->CreatePixelShader(
                    reinterpret_cast<const DWORD*>(data.data()),
                    reinterpret_cast<IDirect3DPixelShader9 **>(&m_ShaderHandle)
            );
        } else {
            printf("DX9Shader: Unknown shader type!\n");
            return false;
        }

        if (FAILED(hr)) {
            printf("DX9Shader: Failed to create shader object! Error: 0x%X\n", hr);
            return false;
        }

        printf("DX9Shader: Applied compiled shader!\n");
        return true;
    }

    std::span<unsigned char> DX9Shader::GetCompiledShader() {
        return {
            (unsigned char *) m_CompiledShader->GetBufferPointer(),
            m_CompiledShader->GetBufferSize()
        };
    }

    bool DX9Shader::Compile() {
        if (m_SourceCode.empty()) {
            printf("DX9Shader: Source code is empty. Please call SetSource first.\n");
            return false;
        }

        // cleanup resources if it's already compiled
        if(IsCompiled()) {
            Destroy();
        }

        HRESULT hr = D3DXCompileShader(
                m_SourceCode.c_str(),
                static_cast<UINT>(m_SourceCode.size()),
                nullptr,
                nullptr,
                "main",
                DX9_GetProfile(m_ShaderType),
                0,
                &m_CompiledShader,
                &m_ErrorBuffer,
                &m_ConstantTable
        );

        if (FAILED(hr)) {
            printf("DX9Shader: Failed to compile shader.\nReason: %s", GetCompileLog().c_str());
            return false;
        }

        return UseCompiledShader(GetCompiledShader(), m_ShaderType);
    }

    std::string DX9Shader::GetSource() {
        return m_SourceCode;
    }

    std::string DX9Shader::GetCompileLog() {
        if (m_ErrorBuffer && m_ErrorBuffer->GetBufferPointer()) {
            return {static_cast<const char *>(m_ErrorBuffer->GetBufferPointer()),
                    m_ErrorBuffer->GetBufferSize()};
        }

        return "";
    }

    bool DX9Shader::IsCompiled() {
        return m_ShaderHandle != nullptr;
    }
}