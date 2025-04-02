#pragma once

#include <Engine/Core/Runtime/Graphics/IShader.hpp>

// forward definition of D3D9 types
struct IDirect3DDevice9;
struct ID3DXBuffer;
struct ID3DXConstantTable;

namespace engine::backend::dx9 {
    struct D3D9Shader : public core::runtime::graphics::IShader {
        explicit D3D9Shader(IDirect3DDevice9 *device) : m_Device(device),
                                              m_ShaderHandle(nullptr),
                                              m_CompiledShader(nullptr),
                                              m_ErrorBuffer(nullptr),
                                              m_ShaderType(core::runtime::graphics::ShaderType::SHADER_TYPE_UNKNOWN) {}

        ~D3D9Shader() {
            printf("D3D9Shader destructor!\n");
        }

        bool Compile() override;

        void Destroy() override;

        void SetSource(std::string_view source, core::runtime::graphics::ShaderType type) override;

        std::string GetSource() override;

        std::string GetCompileLog() override;

        bool IsCompiled() override;

        bool UseCompiledShader(const std::span<unsigned char> &data, core::runtime::graphics::ShaderType type) override;

        std::span<unsigned char> GetCompiledShader() override;

        core::runtime::graphics::ShaderCapsFlags GetImplCapabilities() const override {
            return core::runtime::graphics::ShaderCapsFlags::SHADER_CAPS_ALLOW_PROVIDE_COMPILED;
        }

        void *GetHandle() const {
            return m_ShaderHandle;
        };

        core::runtime::graphics::ShaderType GetShaderType() const {
            return m_ShaderType;
        }

        ID3DXConstantTable* GetConstantTable() const {
            return m_ConstantTable;
        }

    protected:
        IDirect3DDevice9 *m_Device;

        ID3DXBuffer *m_CompiledShader;
        ID3DXBuffer *m_ErrorBuffer;
        ID3DXConstantTable* m_ConstantTable;

        void *m_ShaderHandle; // it's either IDirect3DVertexShader9* or IDirect3DPixelShader9*

        core::runtime::graphics::ShaderType m_ShaderType;
        std::string m_SourceCode;
    };
}