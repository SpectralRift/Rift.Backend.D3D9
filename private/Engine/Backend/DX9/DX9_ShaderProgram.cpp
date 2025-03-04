#include <Engine/Backend/DX9/DX9_ShaderProgram.hpp>
#include <Engine/Backend/DX9/DX9_Shader.hpp>

#include <Engine/Core/Runtime/Graphics/IShader.hpp>

#include <d3d9.h>
#include <d3dx9.h>

namespace engine::backend::dx9 {
    bool DX9ShaderProgram::Link() {
        bool ret = true;

        if (m_FragmentShader && !m_FragmentShader->IsCompiled()) {
            ret &= m_FragmentShader->Compile();
        }

        if (m_VertexShader && !m_VertexShader->IsCompiled()) {
            ret &= m_VertexShader->Compile();
        }

        return ret;
    }

    void DX9ShaderProgram::Destroy() {
        if (m_FragmentShader) {
            m_FragmentShader->Destroy();
            m_FragmentShader = nullptr;
        }

        if (m_VertexShader) {
            m_VertexShader->Destroy();
            m_VertexShader = nullptr;
        }
    }

    void DX9ShaderProgram::Bind() {
        if (m_VertexShader && m_VertexShader->IsCompiled()) {
            auto vShader = dynamic_cast<DX9Shader*>(m_VertexShader.get());
            m_Device->SetVertexShader(reinterpret_cast<IDirect3DVertexShader9 *>(vShader->GetHandle()));
        }

        if (m_FragmentShader && m_FragmentShader->IsCompiled()) {
            auto fShader = dynamic_cast<DX9Shader*>(m_FragmentShader.get());
            m_Device->SetPixelShader(reinterpret_cast<IDirect3DPixelShader9 *>(fShader->GetHandle()));
        }
    }

    void DX9ShaderProgram::Unbind() {
        m_Device->SetVertexShader(nullptr);
        m_Device->SetPixelShader(nullptr);
    }

    void DX9ShaderProgram::AddShader(std::unique_ptr<core::runtime::graphics::IShader> shader) {
        if(shader == nullptr) {
            printf("DX9ShaderProgram: failed to AddShader: shader = NULL!\n");
            return;
        }

        // try to check type by dynamically casting to DX9Shader before
        auto dxShader = dynamic_cast<DX9Shader*>(shader.get());

        if (dxShader) {
            if (dxShader->GetShaderType() == core::runtime::graphics::ShaderType::SHADER_TYPE_FRAGMENT) {
                m_FragmentShader = std::move(shader);
            } else if (dxShader->GetShaderType() == core::runtime::graphics::ShaderType::SHADER_TYPE_VERTEX) {
                m_VertexShader = std::move(shader);
            } else {
                printf("DX9ShaderProgram: Unknown shader type! It must be of SHADER_TYPE_VERTEX or SHADER_TYPE_FRAGMENT!\n");
            };
        } else {
            printf("DX9ShaderProgram: Unknown shader object! It must be of DX9Shader type!\n");
        }
    }

    void DX9ShaderProgram::SetUniformMat4(std::string_view name, const glm::mat4 &mat) {
        // check type and if it's compiled
        auto vShader = dynamic_cast<DX9Shader*>(m_VertexShader.get());
        if (!vShader && !vShader->IsCompiled()) return;

        // get uniform (also known as constant in DX world)
        D3DXHANDLE handle = vShader->GetConstantTable()->GetConstantByName(nullptr, name.data());
        if (!handle) return;

        // set matrix value
        vShader->GetConstantTable()->SetMatrix(m_Device, handle, (D3DXMATRIX*)&mat);
    }

    void DX9ShaderProgram::SetUniformI(std::string_view name, int val) {
        auto vShader = dynamic_cast<DX9Shader*>(m_VertexShader.get());
        if (!vShader && !vShader->IsCompiled()) return;

        D3DXHANDLE handle = vShader->GetConstantTable()->GetConstantByName(nullptr, name.data());
        if (!handle) return;

        vShader->GetConstantTable()->SetInt(m_Device, handle, val);
    }

    std::string DX9ShaderProgram::GetLinkLog() {
        return "";
    }

    bool DX9ShaderProgram::IsLinked() {
        return m_VertexShader != nullptr || m_FragmentShader != nullptr;
    }
}