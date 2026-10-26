// Scratch tool (not in the repo): writes the shader sources the engine
// compiles, through the accessors of EngineGL33_Lighting.hpp.
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <cstdio>
#include <string>

namespace gl = Poseidon::gl33lighting;

static void Write(const std::string& dir, const std::string& name, const char* text)
{
    const std::string path = dir + "/" + name + ".glsl";
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f)
        return;
    std::fwrite(text, 1, std::char_traits<char>::length(text), f);
    std::fclose(f);
}

int main(int argc, char** argv)
{
    if (argc < 2)
        return 2;
    const std::string dir = argv[1];
    const char* vsNames[] = {"vsScreenRemaster", "vsTransformRemaster", "vsShadowRemaster"};
    for (int vs = 0; vs < 3; ++vs)
        Write(dir, vsNames[vs], gl::RemasterVertexSource(vs));
    Write(dir, "psRemasterCommon", gl::RemasterFragmentCommon());
    const char* psNames[] = {"psNormalRemasterMain", "psDetailRemasterMain", "psGrassRemasterMain", "psWaterRemasterMain"};
    for (int ps = 0; ps < 4; ++ps)
        Write(dir, psNames[ps], gl::RemasterFragmentMain(ps));
    for (int i = 0; i < static_cast<int>(gl::PostShader::Count); ++i)
    {
        const auto s = static_cast<gl::PostShader>(i);
        Write(dir, gl::PostShaderName(s), gl::PostShaderSource(s));
    }
    std::printf("ok\n");
    return 0;
}
