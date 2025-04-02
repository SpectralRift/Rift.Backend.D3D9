#include <Engine/Backend/D3D9/D3D9_ShaderProgram.hpp>
#include <Engine/Backend/D3D9/D3D9_Shader.hpp>
#include <Engine/Runtime/Logger.hpp>
#include <Engine/Core/Runtime/Graphics/IShader.hpp>

#include <d3d9.h>
#include <d3dx9.h>

namespace engine::backend::dx9 {
    static runtime::Logger g_LoggerD3D9ShaderProgram("D3D9ShaderProgram");

    bool D3D9ShaderProgram::Link() {
        bool ret = true;

        g_LoggerD3D9ShaderProgram.Log(runtime::LOG_LEVEL_DEBUG, "Linking shaders...");

        if (m_FragmentShader && !m_FragmentShader->IsCompiled()) {
            ret &= m_FragmentShader->Compile();
        }

        if (m_VertexShader && !m_VertexShader->IsCompiled()) {
            ret &= m_VertexShader->Compile();
        }

        if(ret) {
            g_LoggerD3D9ShaderProgram.Log(runtime::LOG_LEVEL_INFO, "Shader program linked successfully!");
        } else {
            g_LoggerD3D9ShaderProgram.Log(runtime::LOG_LEVEL_ERROR, "Failed to link shader program!");
        }

        return ret;
    }

    void D3D9ShaderProgram::Destroy() {
        g_LoggerD3D9ShaderProgram.Log(runtime::LOG_LEVEL_DEBUG, "Shader program is being destroyed.");

        if (m_FragmentShader) {
            m_FragmentShader->Destroy();
            m_FragmentShader = nullptr;
        }

        if (m_VertexShader) {
            m_VertexShader->Destroy();
            m_VertexShader = nullptr;
        }
    }

    void D3D9ShaderProgram::Bind() {
        if (m_VertexShader && m_VertexShader->IsCompiled()) {
            auto vShader = dynamic_cast<D3D9Shader*>(m_VertexShader.get());
            m_Device->SetVertexShader(reinterpret_cast<IDirect3DVertexShader9 *>(vShader->GetHandle()));
        }

        if (m_FragmentShader && m_FragmentShader->IsCompiled()) {
            auto fShader = dynamic_cast<D3D9Shader*>(m_FragmentShader.get());
            m_Device->SetPixelShader(reinterpret_cast<IDirect3DPixelShader9 *>(fShader->GetHandle()));
        }
    }

    void D3D9ShaderProgram::Unbind() {
        m_Device->SetVertexShader(nullptr);
        m_Device->SetPixelShader(nullptr);
    }

    void D3D9ShaderProgram::AddShader(std::unique_ptr<core::runtime::graphics::IShader> shader) {
        if(shader == nullptr) {
            g_LoggerD3D9ShaderProgram.Log(runtime::LOG_LEVEL_ERROR, "failed to AddShader: shader = NULL!");
            return;
        }

        // try to check type by dynamically casting to D3D9Shader before
        auto dxShader = dynamic_cast<D3D9Shader*>(shader.get());

        if (dxShader) {
            if (dxShader->GetShaderType() == core::runtime::graphics::ShaderType::SHADER_TYPE_FRAGMENT) {
                m_FragmentShader = std::move(shader);
            } else if (dxShader->GetShaderType() == core::runtime::graphics::ShaderType::SHADER_TYPE_VERTEX) {
                m_VertexShader = std::move(shader);
            } else {
                g_LoggerD3D9ShaderProgram.Log(runtime::LOG_LEVEL_ERROR, "Unknown shader type!");
            };
        } else {
            g_LoggerD3D9ShaderProgram.Log(runtime::LOG_LEVEL_ERROR, "The shader object must be an instance of D3D9Shader!");
        }
    }

    void D3D9ShaderProgram::SetUniformMat4(std::string_view name, const glm::mat4 &mat) {
        // check type and if it's compiled
        auto vShader = dynamic_cast<D3D9Shader*>(m_VertexShader.get());
        if (!vShader && !vShader->IsCompiled()) return;

        // get uniform (also known as constant in DX world)
        D3DXHANDLE handle = vShader->GetConstantTable()->GetConstantByName(nullptr, name.data());
        if (!handle) return;

        // set matrix value
        vShader->GetConstantTable()->SetMatrix(m_Device, handle, (D3DXMATRIX*)&mat);
    }

    void D3D9ShaderProgram::SetUniformI(std::string_view name, int val) {
        auto vShader = dynamic_cast<D3D9Shader*>(m_VertexShader.get());
        if (!vShader && !vShader->IsCompiled()) return;

        D3DXHANDLE handle = vShader->GetConstantTable()->GetConstantByName(nullptr, name.data());
        if (!handle) return;

        vShader->GetConstantTable()->SetInt(m_Device, handle, val);
    }

    std::string D3D9ShaderProgram::GetLinkLog() {
        return "";
    }

    bool D3D9ShaderProgram::IsLinked() {
        return m_VertexShader != nullptr || m_FragmentShader != nullptr;
    }
}