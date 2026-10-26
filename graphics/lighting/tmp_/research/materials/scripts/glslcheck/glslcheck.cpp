// glslcheck <frag|vert> <file> [<file> ...]: concatenates the files and
// validates them as GLSL 3.30 core with glslang (the same call as the engine's
// test_remaster_lighting.cpp). Research tool, materials track.
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::fprintf(stderr, "usage: glslcheck frag|vert file...\n");
        return 2;
    }
    std::string src;
    for (int i = 2; i < argc; ++i)
    {
        std::ifstream f(argv[i], std::ios::binary);
        if (!f)
        {
            std::fprintf(stderr, "cannot read %s\n", argv[i]);
            return 2;
        }
        std::stringstream ss;
        ss << f.rdbuf();
        src += ss.str();
    }
    glslang::InitializeProcess();
    const EShLanguage stage = std::string(argv[1]) == "vert" ? EShLangVertex : EShLangFragment;
    bool ok = false;
    {
        glslang::TShader shader(stage);
        const char* strings[1] = {src.c_str()};
        shader.setStrings(strings, 1);
        ok = shader.parse(GetDefaultResources(), 330, false, EShMsgDefault);
        std::printf("%s\n%s", ok ? "OK" : "FAILED", shader.getInfoLog());
    }
    glslang::FinalizeProcess();
    return ok ? 0 : 1;
}
