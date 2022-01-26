#include "ShaderCompiler.h"
#include <shaderc/shaderc.hpp>
#include <vector>
#include "Logging.h"

namespace ShaderCompiler
{
    namespace ShaderCompilerInternal
    {
        const char* PREPROCESSOR_DEFINES[static_cast<uint32_t>(Types::COUNT)] =
        {
            "VERTEX_SHADER",
            "FRAGMENT_SHADER"
        };

        shaderc_shader_kind GetShaderKindFromType(ShaderCompiler::Types type)
        {
            switch (type)
            {
            case Types::Vertex:
                return shaderc_shader_kind::shaderc_vertex_shader;
            case Types::Fragment:
                return shaderc_shader_kind::shaderc_fragment_shader;
            default:
                return shaderc_shader_kind::shaderc_glsl_infer_from_source;
            }
        }

        const char* GetPreprocessorDefines(Types type)
        {
            if (static_cast<uint32_t>(type) < static_cast<uint32_t>(Types::COUNT))
            {
                return PREPROCESSOR_DEFINES[static_cast<uint32_t>(type)];
            }
            return nullptr;
        }
    }

    std::vector<uint32_t> Compile(const char* name, const std::vector<char>& shaderBlob, Types type, bool optimize)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        const char* preprocessor = ShaderCompilerInternal::GetPreprocessorDefines(type);
        if (preprocessor != nullptr)
        {
            options.AddMacroDefinition(preprocessor, "1");
        }

        if (optimize)
        {
            options.SetOptimizationLevel(shaderc_optimization_level_size);
        }

        shaderc_shader_kind kind = ShaderCompilerInternal::GetShaderKindFromType(type);

        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderBlob.data(),shaderBlob.size(), kind, name, options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            LOG_ERROR(module.GetErrorMessage().c_str());
            return std::vector<uint32_t>();
        }

        return { module.cbegin(), module.cend() };
    }

}
