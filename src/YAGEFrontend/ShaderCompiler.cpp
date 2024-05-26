#include "ShaderCompiler.h"
#include <shaderc/shaderc.hpp>
#include <vector>
#include "Logging.h"
#include "FileParser.h"

#define SHADER_CACHE_DIR "shadercache"
#define SHADER_CACHE_SUFFIX "cache"

namespace ShaderCompiler
{
    namespace
    {
        const char* PREPROCESSOR_DEFINES[static_cast<uint32_t>(Types::COUNT)] =
        {
            "VERTEX_SHADER",
            "FRAGMENT_SHADER"
        };

        shaderc_shader_kind GetShaderKindFromType(Types type)
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

        bool ReadShaderCache(const std::string& vsCacheName, const std::string& fsCacheName, CombinedShaderBinary& shaders)
        {
            std::vector<char> vsCache;
            if (FileParser::Read(vsCacheName, vsCache))
            {
                std::vector<char> fsCache;
                if (!FileParser::Read(fsCacheName, fsCache))
                {
                    LOG_ERROR("Only found partial shader cache files, aborting");
                    abort();
                }

                shaders.m_vs.resize(vsCache.size() / sizeof(uint32_t));
                memcpy(shaders.m_vs.data(), vsCache.data(), vsCache.size());

                shaders.m_fs.resize(fsCache.size() / sizeof(uint32_t));
                memcpy(shaders.m_fs.data(), fsCache.data(), fsCache.size());

                return true;
            }
            return false;
        }

        void WriteShaderCache(const std::string& vsCacheName, const std::string& fsCacheName, CombinedShaderBinary& shaders)
        {
            FileParser::CreateDirectory(SHADER_CACHE_DIR);
            if (!FileParser::Write(vsCacheName, shaders.m_vs.data(), shaders.m_vs.size() * sizeof(uint32_t)))
            {
                LOG_ERROR("Failed to write shader cache file");
            }
            if (!FileParser::Write(fsCacheName, shaders.m_fs.data(), shaders.m_fs.size() * sizeof(uint32_t)))
            {
                LOG_ERROR("Failed to write shader cache file");
            }
        }
    }

    std::vector<uint32_t> Compile(const char* name, const std::vector<char>& shaderBlob, Types type, bool optimize)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        const char* preprocessor = GetPreprocessorDefines(type);
        if (preprocessor != nullptr)
        {
            options.AddMacroDefinition(preprocessor, "1");
        }

        if (optimize)
        {
            options.SetOptimizationLevel(shaderc_optimization_level_size);
        }

        shaderc_shader_kind kind = GetShaderKindFromType(type);

        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderBlob.data(),shaderBlob.size(), kind, name, options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            LOG_ERROR(module.GetErrorMessage().c_str());
            return std::vector<uint32_t>();
        }
        
        return { module.cbegin(), module.cend() };
    }

    

    CombinedShaderBinary GetCompiledShaders(const char* name)
    {
        CombinedShaderBinary shaders;
        std::string strippedName = FileParser::StripFileEnding(name);
        std::string fmt("%s/%s_%s.%s");
        std::string vsCacheName = string_format(fmt, SHADER_CACHE_DIR, strippedName.c_str(), PREPROCESSOR_DEFINES[static_cast<uint32_t>(ShaderCompiler::Types::Vertex)], SHADER_CACHE_SUFFIX);
        std::string fsCacheName = string_format(fmt, SHADER_CACHE_DIR, strippedName.c_str(), PREPROCESSOR_DEFINES[static_cast<uint32_t>(ShaderCompiler::Types::Fragment)], SHADER_CACHE_SUFFIX);

        if (ReadShaderCache(vsCacheName, fsCacheName, shaders))
        {
            return shaders;
        }

        std::vector<char> shaderBlob;
        if (!FileParser::Read(name, shaderBlob))
        {
            LOG_ERROR("Failed to parse shader file");
            abort();
        }

        shaders.m_vs = ShaderCompiler::Compile(name, shaderBlob, ShaderCompiler::Types::Vertex);
        shaders.m_fs = ShaderCompiler::Compile(name, shaderBlob, ShaderCompiler::Types::Fragment);

        WriteShaderCache(vsCacheName, fsCacheName, shaders);

        return shaders;
    }

}
