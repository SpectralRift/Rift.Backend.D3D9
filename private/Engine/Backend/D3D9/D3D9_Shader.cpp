#include <Engine/Backend/D3D9/D3D9_Shader.hpp>
#include <Engine/Runtime/Logger.hpp>

#include <d3d9.h>
#include <d3dx9.h>

namespace engine::backend::dx9 {
    static runtime::Logger g_LoggerD3D9Shader("D3D9Shader");

    void D3D9Shader::Destroy() {
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

    void D3D9Shader::SetSource(std::string_view source, core::runtime::graphics::ShaderType type) {
        m_SourceCode = source;
        m_ShaderType = type;
    }

    const char *D3D9_GetProfile(core::runtime::graphics::ShaderType type) {
        switch (type) {
            default:
            case core::runtime::graphics::ShaderType::SHADER_TYPE_VERTEX:
                return "vs_3_0";
            case core::runtime::graphics::ShaderType::SHADER_TYPE_FRAGMENT:
                return "ps_3_0";
        }
    }

    bool D3D9Shader::UseCompiledShader(const std::span<unsigned char> &data, core::runtime::graphics::ShaderType type) {
        if (data.empty()) {
            g_LoggerD3D9Shader.Log(runtime::LOG_LEVEL_ERROR, "Shader code is empty.");
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
            g_LoggerD3D9Shader.Log(runtime::LOG_LEVEL_ERROR, "Unknown shader type!");
            return false;
        }

        if (FAILED(hr)) {
            g_LoggerD3D9Shader.Log(runtime::LOG_LEVEL_ERROR, "Failed to create shader object! Error: 0x%X", hr);
            return false;
        }

        g_LoggerD3D9Shader.Log(runtime::LOG_LEVEL_DEBUG, "Successfully created shader object from compiled shader.");
        return true;
    }

    std::span<unsigned char> D3D9Shader::GetCompiledShader() {
        return {
            (unsigned char *) m_CompiledShader->GetBufferPointer(),
            m_CompiledShader->GetBufferSize()
        };
    }

    bool D3D9Shader::Compile() {
        if (m_SourceCode.empty()) {
            g_LoggerD3D9Shader.Log(runtime::LOG_LEVEL_ERROR, "Source code is empty. Please call SetSource first.");
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
                D3D9_GetProfile(m_ShaderType),
                0,
                &m_CompiledShader,
                &m_ErrorBuffer,
                &m_ConstantTable
        );

        if (FAILED(hr)) {
            g_LoggerD3D9Shader.Log(runtime::LOG_LEVEL_ERROR, "Failed to compile shader");
            g_LoggerD3D9Shader.Log(runtime::LOG_LEVEL_ERROR, "%s", GetCompileLog().c_str());
            return false;
        }

        return UseCompiledShader(GetCompiledShader(), m_ShaderType);
    }

    std::string D3D9Shader::GetSource() {
        return m_SourceCode;
    }

    std::string D3D9Shader::GetCompileLog() {
        if (m_ErrorBuffer && m_ErrorBuffer->GetBufferPointer()) {
            return {static_cast<const char *>(m_ErrorBuffer->GetBufferPointer()),
                    m_ErrorBuffer->GetBufferSize()};
        }

        return "";
    }

    bool D3D9Shader::IsCompiled() {
        return m_ShaderHandle != nullptr;
    }
}