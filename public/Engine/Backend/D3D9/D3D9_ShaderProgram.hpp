#pragma once

#include <Engine/Core/Runtime/Graphics/IShaderProgram.hpp>
#include <Engine/Core/Runtime/Graphics/IShader.hpp>

// forward definition of D3D9 types
struct IDirect3DDevice9;

namespace engine::backend::dx9 {
    struct D3D9ShaderProgram : public core::runtime::graphics::IShaderProgram {
        D3D9ShaderProgram(IDirect3DDevice9 *device) : m_Device(device) {}

        bool Link() override;

        void Destroy() override;

        void Bind() override;

        void Unbind() override;

        void AddShader(std::unique_ptr<core::runtime::graphics::IShader> shader) override;

        void SetUniformMat4(std::string_view name, const glm::mat4 &mat) override;

        void SetUniformI(std::string_view name, int val) override;

        std::string GetLinkLog() override;

        bool IsLinked() override;

    protected:
        IDirect3DDevice9 *m_Device;
        std::unique_ptr<core::runtime::graphics::IShader> m_FragmentShader;
        std::unique_ptr<core::runtime::graphics::IShader> m_VertexShader;
    };
}