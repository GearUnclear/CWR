#include <catch2/catch_test_macros.hpp>

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_ProgramCache.hpp>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// The world program set links one program per (set, vertex shader, pixel
// shader), 36 in all, and stores it in the four [specular][mode] slots
// (EngineGL33_ProgramCache.hpp, EngineGL33_Shaders.cpp). These tests pin the
// cache key and version, the slot sharing, that a shared name is deleted once,
// and the source contracts that keep the sharing safe.

namespace gp = Poseidon::gl33programs;

namespace
{

using ProgramTable = unsigned int[NVertexShaders][NPixelShaderSpecular][NPixelShaderModes][NPixelShaders];
constexpr int kSets = 2;
constexpr size_t kSlots = sizeof(ProgramTable) / sizeof(unsigned int);

std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    if (!f.is_open())
        return {};
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path Gl33Dir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine" / "PoseidonGL33";
}

std::string FunctionBody(const std::string& source, const std::string& signature)
{
    const size_t start = source.find(signature);
    if (start == std::string::npos)
        return {};
    const size_t end = source.find("\n}\n", start);
    return source.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

int CountOf(const std::string& text, const std::string& needle)
{
    int n = 0;
    for (size_t at = text.find(needle); at != std::string::npos; at = text.find(needle, at + needle.size()))
        ++n;
    return n;
}

// The two tables as InitPixelShaders fills them: one fresh name per
// (set, vs, ps), names 1..36 in link order.
void FillAsInit(ProgramTable& classic, ProgramTable& remaster)
{
    std::memset(classic, 0, sizeof(ProgramTable));
    std::memset(remaster, 0, sizeof(ProgramTable));
    unsigned int next = 1;
    for (int set = 0; set < kSets; ++set)
        for (int v = 0; v < NVertexShaders; ++v)
            for (int ps = 0; ps < NPixelShaders; ++ps)
            {
                if (set == 0)
                    gp::ShareAcrossSpecularAndMode(classic, v, ps, next++);
                else
                    gp::ShareAcrossSpecularAndMode(remaster, v, ps, next++);
            }
}

} // namespace

TEST_CASE("World programs: the cache is version 2, keyed by set, vertex and pixel shader",
          "[Graphics][Shaders][ProgramCache]")
{
    STATIC_REQUIRE(gp::kShaderCacheVersion == 2);
    STATIC_REQUIRE(gp::WorldProgramKey(0, 0, 0) == 0u);
    STATIC_REQUIRE(gp::WorldProgramKey(1, 2, 5) == 0x12000005u);
    STATIC_REQUIRE(gp::WorldProgramKey(0, VSTransform, PSGrass) == ((1u << 24) | 2u));

    // 36 distinct keys, none carrying the specular or mode bits (8..23) the
    // version 1 keys held.
    std::set<uint32_t> keys;
    for (int set = 0; set < kSets; ++set)
        for (int v = 0; v < NVertexShaders; ++v)
            for (int ps = 0; ps < NPixelShaders; ++ps)
            {
                const uint32_t key = gp::WorldProgramKey(set, v, ps);
                CHECK((key & 0x00ffff00u) == 0u);
                keys.insert(key);
            }
    CHECK(keys.size() == 36);
}

TEST_CASE("World programs: one name fills the four specular and mode slots", "[Graphics][Shaders][ProgramCache]")
{
    ProgramTable table = {};
    gp::ShareAcrossSpecularAndMode(table, VSShadow, PSWater, 77u);
    int filled = 0;
    for (int v = 0; v < NVertexShaders; ++v)
        for (int s = 0; s < NPixelShaderSpecular; ++s)
            for (int m = 0; m < NPixelShaderModes; ++m)
                for (int ps = 0; ps < NPixelShaders; ++ps)
                {
                    const bool mine = v == VSShadow && ps == PSWater;
                    CHECK(table[v][s][m][ps] == (mine ? 77u : 0u));
                    filled += table[v][s][m][ps] ? 1 : 0;
                }
    CHECK(filled == static_cast<int>(NPixelShaderSpecular) * static_cast<int>(NPixelShaderModes));

    // A failed link (0) clears the slots it owned.
    gp::ShareAcrossSpecularAndMode(table, VSShadow, PSWater, 0u);
    for (int s = 0; s < NPixelShaderSpecular; ++s)
        for (int m = 0; m < NPixelShaderModes; ++m)
            CHECK(table[VSShadow][s][m][PSWater] == 0u);
}

TEST_CASE("World programs: 144 slots hold 36 names, each deleted once", "[Graphics][Shaders][ProgramCache]")
{
    STATIC_REQUIRE(kSlots * 2u == 144u);
    ProgramTable classic, remaster;
    FillAsInit(classic, remaster);

    // The night eye and the specular switch land on the same program.
    CHECK(classic[VSTransform][PSSSpecular][PSMNight][PSNormal] == classic[VSTransform][PSSNormal][PSMDay][PSNormal]);
    CHECK(remaster[VSTransform][PSSNormal][PSMNight][PSGrass] == remaster[VSTransform][PSSNormal][PSMDay][PSGrass]);
    // Different (vs, ps) and the two sets never share.
    CHECK(classic[VSTransform][PSSNormal][PSMDay][PSNormal] != classic[VSTransform][PSSNormal][PSMDay][PSDetail]);
    CHECK(classic[VSTransform][PSSNormal][PSMDay][PSNormal] != classic[VSScreen][PSSNormal][PSMDay][PSNormal]);
    CHECK(classic[VSTransform][PSSNormal][PSMDay][PSNormal] != remaster[VSTransform][PSSNormal][PSMDay][PSNormal]);

    // DeinitPixelShaders' delete list: every name once, zeros skipped.
    std::vector<unsigned int> programs;
    gp::CollectDistinctPrograms(&classic[0][0][0][0], kSlots, programs);
    gp::CollectDistinctPrograms(&remaster[0][0][0][0], kSlots, programs);
    REQUIRE(programs.size() == 36);
    CHECK(std::set<unsigned int>(programs.begin(), programs.end()).size() == 36);
    for (unsigned int i = 0; i < 36; ++i)
        CHECK(programs[i] == i + 1); // first-seen order

    // A set that failed to link leaves zeros, which are never deleted.
    std::memset(remaster, 0, sizeof(ProgramTable));
    remaster[VSScreen][PSSNormal][PSMDay][PSFlat] = 500u;
    programs.clear();
    gp::CollectDistinctPrograms(&remaster[0][0][0][0], kSlots, programs);
    REQUIRE(programs.size() == 1);
    CHECK(programs[0] == 500u);
}

TEST_CASE("World programs: the backend links 36, deletes each once and skips same-program rebinds",
          "[Graphics][Shaders][ProgramCache]")
{
    const std::string shaders = ReadTextFile(Gl33Dir() / "EngineGL33_Shaders.cpp");
    REQUIRE_FALSE(shaders.empty());
    // One version, from the shared header.
    CHECK(shaders.find("constexpr uint32_t kCacheVersion = Poseidon::gl33programs::kShaderCacheVersion;") !=
          std::string::npos);
    CHECK(CountOf(shaders, "kCacheVersion =") == 1);

    const std::string init = FunctionBody(shaders, "void EngineGL33::InitPixelShaders()");
    REQUIRE_FALSE(init.empty());
    // The key has no specular or mode bits; the slots are filled by sharing.
    CHECK(init.find("gp::WorldProgramKey(") != std::string::npos);
    CHECK(init.find("<< 16") == std::string::npos);
    CHECK(init.find("gp::ShareAcrossSpecularAndMode(_shaderProgram, v, i, prog);") != std::string::npos);
    CHECK(init.find("gp::ShareAcrossSpecularAndMode(_shaderProgramRemaster, v, i, prog);") != std::string::npos);
    // The loops run over (set, vs, ps) only.
    CHECK(init.find("NPixelShaderSpecular") == std::string::npos);
    CHECK(init.find("NPixelShaderModes") == std::string::npos);
    // Every restore and every link is issued before its first status query.
    const size_t lastRestore = init.rfind("BeginRestoreProgramFromCache(");
    const size_t firstRestoreStatus = init.find("FinishRestoreProgramFromCache(");
    REQUIRE(lastRestore != std::string::npos);
    REQUIRE(firstRestoreStatus != std::string::npos);
    CHECK(lastRestore < firstRestoreStatus);
    const size_t lastLink = init.rfind("BeginLinkProgramRetrievable(");
    const size_t firstLinkStatus = init.find("FinishLinkProgramRetrievable(");
    REQUIRE(lastLink != std::string::npos);
    REQUIRE(firstLinkStatus != std::string::npos);
    CHECK(lastLink < firstLinkStatus);
    CHECK(init.find("GL_LINK_STATUS") == std::string::npos);
    // Samplers and blocks are bound once per program.
    CHECK(CountOf(init, "bindProgramSamplersAndBlocks(prog);") == 1);
    // The vertex shaders compile only on the link path, not in InitVertexShaders.
    CHECK(init.find("EnsureWorldVsCompiled();") != std::string::npos);
    const std::string initVs = FunctionBody(shaders, "void EngineGL33::InitVertexShaders()");
    REQUIRE_FALSE(initVs.empty());
    CHECK(initVs.find("CompileGLShader(") == std::string::npos);
    CHECK(FunctionBody(shaders, "void EngineGL33::DeinitVertexShaders()").find("s_vsCompiled = false;") !=
          std::string::npos);

    // Deinit deletes the distinct names, once each.
    const std::string deinit = FunctionBody(shaders, "void EngineGL33::DeinitPixelShaders()");
    REQUIRE_FALSE(deinit.empty());
    CHECK(CountOf(deinit, "CollectDistinctPrograms(") == 2);
    CHECK(CountOf(deinit, "glDeleteProgram(") == 1);
    CHECK(deinit.find("s_worldProgramBound = 0;") != std::string::npos);

    // The selection binds through one guarded call; the grass and water
    // constants are uploaded on every call, outside the guard.
    const std::string select = FunctionBody(shaders, "void EngineGL33::DoSelectPixelShader(");
    REQUIRE_FALSE(select.empty());
    CHECK(CountOf(select, "glUseProgram(") == 1);
    const size_t guard = select.find("if (prog == s_worldProgramBound && _pixelShaderSel != PSNone)");
    const size_t guardEnd = select.find("};", guard);
    REQUIRE(guard != std::string::npos);
    REQUIRE(guardEnd != std::string::npos);
    const size_t grass = select.find("DoSetGrassParamsPS();");
    const size_t water = select.find("UploadPSConstant(PSConstants::SlotLightDir");
    REQUIRE(grass != std::string::npos);
    REQUIRE(water != std::string::npos);
    CHECK(grass > guardEnd);
    CHECK(water > guardEnd);
    // The night-eye coefficients go to the constant block after the mode switch.
    const std::string nightEye = FunctionBody(shaders, "void EngineGL33::EnableNightEye(");
    const size_t modeSwitch = nightEye.find("SelectPixelShaderMode(mode);");
    const size_t eyeUpload = nightEye.find("UploadPSConstant(PSConstants::SlotRgbEyeCoef");
    REQUIRE(modeSwitch != std::string::npos);
    REQUIRE(eyeUpload != std::string::npos);
    CHECK(modeSwitch < eyeUpload);
}

TEST_CASE("World programs: per-program uniform state stays keyed by GL name", "[Graphics][Shaders][ProgramCache]")
{
    // The surface class is the one plain uniform a world program gets after
    // linking; its cache is indexed by the program's name, so slots sharing a
    // program share its one cached value, which is what the program holds.
    const std::string surface = ReadTextFile(Gl33Dir() / "EngineGL33_Surface.cpp");
    REQUIRE_FALSE(surface.empty());
    CHECK(surface.find("SurfClassProgram& sp = s_surfClassPrograms[_surfBoundProgram];") != std::string::npos);
    CHECK(CountOf(surface, "glUniform") == 1);

    // The [specular][mode] tables are read only by the selection, the init and
    // deinit, and the set chooser: nothing else can attach state to one slot.
    int users = 0;
    for (const auto& entry : std::filesystem::directory_iterator(Gl33Dir()))
    {
        if (entry.path().extension() != ".cpp")
            continue;
        const std::string name = entry.path().filename().string();
        const std::string text = ReadTextFile(entry.path());
        if (text.find("_shaderProgramRemaster[") == std::string::npos &&
            text.find("_shaderProgram[") == std::string::npos)
            continue;
        CAPTURE(name);
        CHECK((name == "EngineGL33_Shaders.cpp" || name == "EngineGL33_Lighting.cpp"));
        ++users;
    }
    CHECK(users == 2);
}
