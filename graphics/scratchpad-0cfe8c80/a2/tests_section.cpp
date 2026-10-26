// --- GLSL contracts --------------------------------------------------------
//
// The tests read the shaders through the accessors of EngineGL33_Lighting.hpp
// (the sources the engine compiles, assembled from their pieces). They read
// the C++ sources only to check that every raw GLSL string reaches a program,
// that the assembly tables the lighting lab parses are the ones the engine
// uses and that the LitConstants block has no hand copy.

namespace
{
namespace gl = Poseidon::gl33lighting;

std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path Gl33Dir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine" / "PoseidonGL33";
}

// The backend's .cpp files whose name starts with `prefix`, sorted.
std::vector<std::filesystem::path> Gl33Sources(const std::string& prefix)
{
    std::vector<std::filesystem::path> out;
    for (const auto& entry : std::filesystem::directory_iterator(Gl33Dir()))
    {
        const std::string name = entry.path().filename().string();
        if (entry.is_regular_file() && entry.path().extension() == ".cpp" && name.rfind(prefix, 0) == 0)
            out.push_back(entry.path());
    }
    std::sort(out.begin(), out.end());
    return out;
}

// Every `char name[] = R"(...)"` raw string of a C++ source, by name.
std::map<std::string, std::string> RawStrings(const std::string& src)
{
    std::map<std::string, std::string> out;
    static const std::regex header(R"(\bchar\s+(\w+)\s*\[\s*\]\s*=\s*R\"\()");
    for (auto it = std::sregex_iterator(src.begin(), src.end(), header); it != std::sregex_iterator(); ++it)
    {
        const size_t body = static_cast<size_t>(it->position() + it->length());
        const size_t end = src.find(")\"", body);
        if (end != std::string::npos)
            out[(*it)[1].str()] = src.substr(body, end - body);
    }
    return out;
}

// The raw strings of every backend file whose name starts with `prefix`.
std::map<std::string, std::string> RawStringsOf(const std::string& prefix)
{
    std::map<std::string, std::string> out;
    for (const auto& path : Gl33Sources(prefix))
        for (auto& [name, body] : RawStrings(ReadTextFile(path)))
            out[name] = body;
    return out;
}

// The identifiers of a C++ brace-list body, `//` comments dropped and any
// namespace qualification stripped.
std::vector<std::string> ListedNames(const std::string& list)
{
    std::vector<std::string> out;
    static const std::regex comment(R"(//[^\n]*)");
    static const std::regex ident(R"(([A-Za-z_][\w:]*))");
    const std::string clean = std::regex_replace(list, comment, "");
    for (auto it = std::sregex_iterator(clean.begin(), clean.end(), ident); it != std::sregex_iterator(); ++it)
    {
        const std::string name = (*it)[1].str();
        const size_t colon = name.rfind("::");
        out.push_back(colon == std::string::npos ? name : name.substr(colon + 2));
    }
    return out;
}

// The raw strings named, joined in order.
std::string Joined(const std::vector<std::string>& names, const std::map<std::string, std::string>& raw)
{
    std::string out;
    for (const std::string& name : names)
    {
        const auto it = raw.find(name);
        if (it == raw.end())
            return "<no raw string " + name + ">";
        out += it->second;
    }
    return out;
}

int CountOf(const std::string& haystack, const std::string& needle)
{
    int n = 0;
    for (size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + needle.size()))
        ++n;
    return n;
}

// Plain GLSL 3.30 core validation (glslangValidator's default). No SPIR-V
// client, unlike the I-28 harness: the post shaders use loose uniforms, which
// GL 3.3 allows and SPIR-V for OpenGL does not.
bool Compiles(const std::string& src, EShLanguage stage, std::string& log)
{
    glslang::TShader shader(stage);
    const char* strings[1] = {src.c_str()};
    shader.setStrings(strings, 1);
    const bool ok = shader.parse(GetDefaultResources(), 330, false, EShMsgDefault);
    log = shader.getInfoLog();
    return ok;
}

struct GlslangInit
{
    GlslangInit() { glslang::InitializeProcess(); }
    ~GlslangInit() { glslang::FinalizeProcess(); }
};

// `<qualifier> <type> <name>;` declarations (varyings) of a GLSL block.
std::set<std::string> Varyings(const std::string& glsl, const std::string& qualifier)
{
    std::set<std::string> out;
    const std::regex decl("(?:flat\\s+)?" + qualifier + "\\s+(\\w+)\\s+(\\w+)\\s*;");
    for (auto it = std::sregex_iterator(glsl.begin(), glsl.end(), decl); it != std::sregex_iterator(); ++it)
    {
        const std::string name = (*it)[2].str();
        if (name != "fragColor")
            out.insert((*it)[1].str() + " " + name);
    }
    return out;
}

// Covers every VertexShaderID and PixelShaderID; the accessors return null
// past the last one.
constexpr int kShaderIds = 16;

std::string Piece(gl::FragmentPiece piece)
{
    const char* text = gl::RemasterFragmentPiece(piece);
    return text ? text : "";
}

// The remastered programs: each vertex shader and the prelude with each main.
std::vector<std::string> RemasterPrograms()
{
    std::vector<std::string> out;
    for (int vs = 0; vs < kShaderIds; ++vs)
        if (const char* src = gl::RemasterVertexSource(vs))
            out.emplace_back(src);
    for (int ps = 0; ps < kShaderIds; ++ps)
        if (const char* main = gl::RemasterFragmentMain(ps))
            out.push_back(std::string(gl::RemasterFragmentCommon()) + main);
    return out;
}

// Every program the remastered set and the post pass compile.
std::vector<std::string> AssembledPrograms()
{
    std::vector<std::string> out = RemasterPrograms();
    for (int i = 0; i < static_cast<int>(gl::PostShader::Count); ++i)
        out.emplace_back(gl::PostShaderSource(static_cast<gl::PostShader>(i)));
    return out;
}
} // namespace

TEST_CASE("Remaster shaders compile under glslang and their varyings match", "[Graphics][Shaders][remaster]")
{
    GlslangInit init;
    const std::string common = gl::RemasterFragmentCommon();
    REQUIRE_FALSE(common.empty());

    int vertex = 0, fragment = 0;
    for (int vs = 0; vs < kShaderIds; ++vs)
    {
        const char* src = gl::RemasterVertexSource(vs);
        if (!src)
            continue;
        CAPTURE(vs);
        std::string log;
        const bool ok = Compiles(src, EShLangVertex, log);
        CAPTURE(log);
        CHECK(ok);
        // Every remastered VS feeds the same varyings the common FS reads.
        CHECK(Varyings(src, "out") == Varyings(common, "in"));
        ++vertex;
    }
    for (int ps = 0; ps < kShaderIds; ++ps)
    {
        const char* main = gl::RemasterFragmentMain(ps);
        if (!main)
            continue;
        CAPTURE(ps);
        std::string log;
        const bool ok = Compiles(common + main, EShLangFragment, log);
        CAPTURE(log);
        CHECK(ok);
        ++fragment;
    }
    CHECK(vertex == 3);   // VSScreen, VSTransform, VSShadow
    CHECK(fragment == 4); // PSNormal, PSDetail, PSGrass, PSWater

    int post = 0;
    for (int i = 0; i < static_cast<int>(gl::PostShader::Count); ++i)
    {
        const auto shader = static_cast<gl::PostShader>(i);
        const char* name = gl::PostShaderName(shader);
        REQUIRE(name != nullptr);
        CAPTURE(name);
        std::string log;
        const bool ok = Compiles(gl::PostShaderSource(shader),
                                 shader == gl::PostShader::Vertex ? EShLangVertex : EShLangFragment, log);
        CAPTURE(log);
        CHECK(ok);
        ++post;
    }
    CHECK(post == 7); // vsPost, AO, AO blur, bloom down and up, shafts, composite
}

TEST_CASE("Every raw GLSL string of the lighting and post sources reaches a compiled program",
          "[Graphics][Shaders][remaster]")
{
    // A piece left out of its assembly table, or a shader in a new
    // EngineGL33_Post*.cpp that no table names, would never be compiled or
    // tested: every raw string must sit inside a program the engine builds.
    const std::vector<std::string> programs = AssembledPrograms();
    const auto lighting = Gl33Sources("EngineGL33_LightingShaders");
    const auto post = Gl33Sources("EngineGL33_Post");
    CHECK(lighting.size() == 4); // the main file, Sky, Lights, Surface
    REQUIRE_FALSE(post.empty());
    int strings = 0;
    for (const auto& files : {lighting, post})
        for (const auto& path : files)
            for (const auto& [name, body] : RawStrings(ReadTextFile(path)))
            {
                CAPTURE(path.filename().string(), name);
                const std::string& text = body;
                const bool reached = std::any_of(programs.begin(), programs.end(), [&text](const std::string& p)
                                                 { return p.find(text) != std::string::npos; });
                CHECK(reached);
                ++strings;
            }
    CHECK(strings == 27); // 3 VS + 10 prelude pieces + 4 mains; 6 post shaders + AO's 3 pieces + vsPost
}

TEST_CASE("Remaster fragment prelude is its concern pieces in order", "[Graphics][Shaders][remaster]")
{
    using P = gl::FragmentPiece;
    std::string joined;
    for (int i = 0; i < static_cast<int>(P::Count); ++i)
    {
        const std::string piece = Piece(static_cast<P>(i));
        CAPTURE(i);
        CHECK_FALSE(piece.empty());
        joined += piece;
    }
    const std::string common = gl::RemasterFragmentCommon();
    CHECK(joined == common);
    CHECK(gl::RemasterFragmentPiece(P::Count) == nullptr);
    CHECK(Piece(P::LitConstants) == gl::LitConstantsBlock());
    CHECK(Piece(P::Header).rfind("#version 330 core\n", 0) == 0);

    // Each concern's declarations sit in its own piece, once, in this order.
    const std::pair<P, const char*> owners[] = {
        {P::Header, "uniform PSConstants {"},
        {P::LitConstants, "uniform LitConstants {"},
        {P::Inputs, "uniform sampler2DArrayShadow shadowMap;"},
        {P::Inputs, "vec3 tonemapNeutral("},
        {P::Shadows, "float shadowMapVisibility("},
        {P::TerrainClouds, "float terrainSunVisibility("},
        {P::TerrainClouds, "float cloudSunVisibility("},
        {P::Sky, "vec3 skyIrradiance("},
        {P::Sky, "vec3 fogColorToward("},
        {P::Lights, "vec3 localLights("},
        {P::Surface, "vec3 glossy("},
        {P::Shade, "vec3 shadeRemastered("},
        {P::Finish, "void finishFragment("},
    };
    size_t last = 0;
    for (const auto& [piece, decl] : owners)
    {
        CAPTURE(static_cast<int>(piece), decl);
        CHECK(Piece(piece).find(decl) != std::string::npos);
        CHECK(CountOf(common, decl) == 1);
        const size_t at = common.find(decl);
        CHECK(at >= last);
        last = at;
    }

    // The lighting lab assembles the prelude from the kFragmentPieces table it
    // parses out of EngineGL33_LightingShaders.cpp: that table, read the same
    // way, gives the prelude the engine compiles.
    const std::string source = ReadTextFile(Gl33Dir() / "EngineGL33_LightingShaders.cpp");
    const std::string tableStart = "kFragmentPieces[] = {";
    const size_t open = source.find(tableStart);
    REQUIRE(open != std::string::npos);
    const size_t close = source.find("};", open);
    REQUIRE(close != std::string::npos);
    const std::vector<std::string> names =
        ListedNames(source.substr(open + tableStart.size(), close - open - tableStart.size()));
    CHECK(names.size() == static_cast<size_t>(P::Count));
    CHECK(Joined(names, RawStringsOf("EngineGL33_LightingShaders")) == common);
}

TEST_CASE("Post shaders are the pieces their table names", "[Graphics][Shaders][remaster]")
{
    // The same contract for the post pass: kPostShaders in EngineGL33_PostFX.cpp,
    // read the way the lighting lab reads it, gives each program the engine
    // compiles, under its override name.
    const std::string source = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    const size_t open = source.find("kPostShaders[] = {");
    REQUIRE(open != std::string::npos);
    const size_t close = source.find("\n};", open);
    REQUIRE(close != std::string::npos);
    const std::string table = source.substr(open, close - open);
    const auto raw = RawStringsOf("EngineGL33_Post");
    static const std::regex row(R"(\{\s*"(\w+)"\s*,\s*\{([^{}]*)\}\s*\})");
    int i = 0;
    for (auto it = std::sregex_iterator(table.begin(), table.end(), row); it != std::sregex_iterator(); ++it, ++i)
    {
        REQUIRE(i < static_cast<int>(gl::PostShader::Count));
        const auto shader = static_cast<gl::PostShader>(i);
        const std::string name = (*it)[1].str();
        CAPTURE(name);
        CHECK(name == gl::PostShaderName(shader));
        CHECK(Joined(ListedNames((*it)[2].str()), raw) == gl::PostShaderSource(shader));
    }
    CHECK(i == static_cast<int>(gl::PostShader::Count));
    CHECK(gl::PostShaderName(gl::PostShader::Count) == nullptr);

    // psPostAo reads the shared depth reconstruction.
    const std::string ao = gl::PostShaderSource(gl::PostShader::Ao);
    CHECK(CountOf(ao, "float viewZ(vec2 uv) {") == 1);
    CHECK(CountOf(ao, "vec3 viewPos(vec2 uv, float z) {") == 1);
}

TEST_CASE("Remaster lighting normals survive leaves without a mesh normal", "[Graphics][Shaders][remaster]")
{
    // Many CWA tree crowns (the town birch on Malden, several pines and
    // broadleaves) have leaf cards whose mesh normals are zero. normalize() of a
    // zero vector is undefined (NaN on NVIDIA), and a NaN lighting normal makes
    // the crown fall back to ground-only sky light with no sun shadow. The
    // vertex shaders and the per-fragment normal setup must use the guarded
    // form, x * inversesqrt(max(dot(x, x), eps)), which keeps a zero vector zero.
    int vertex = 0;
    for (int vs = 0; vs < kShaderIds; ++vs)
    {
        const char* src = gl::RemasterVertexSource(vs);
        if (!src)
            continue;
        CAPTURE(vs);
        CHECK(std::string(src).find("normalize(") == std::string::npos);
        ++vertex;
    }
    CHECK(vertex == 3);
    const std::string piece = Piece(gl::FragmentPiece::Shade);
    const size_t start = piece.find("vec3 shadeRemastered(");
    REQUIRE(start != std::string::npos);
    const size_t end = piece.find("\n}\n", start);
    REQUIRE(end != std::string::npos);
    const std::string shade = piece.substr(start, end - start);
    CHECK(shade.find("normalize(") == std::string::npos);
    // A foliage fragment without a normal of its own lights with the crown's.
    CHECK(std::regex_search(shade, std::regex(R"(if\s*\(\s*foliage\s*&&\s*dot\(n,\s*n\)\s*<[^)]*\)\s*n\s*=\s*cn;)")));
}

TEST_CASE("Remaster sun shafts brighten the air, not the surfaces near the eye", "[Graphics][Shaders][remaster]")
{
    // Clear air: the fog end of any view distance from 900 m up.
    for (float fogEnd : {880.0f, 1480.0f, 3980.0f})
    {
        CAPTURE(fogEnd);
        const float length = ShaftScatterLength(fogEnd);
        CHECK(length <= kShaftScatterLength);
        CHECK(length > 0.9f * kShaftScatterLength);
        // The acceptance props at a low sun: soldier 7 m, Jeep 10 m, M113 16 m.
        // At full weight they were veiled grey.
        CHECK(ShaftInscatter(7.0f, length) < 0.03f);
        CHECK(ShaftInscatter(10.0f, length) < 0.04f);
        CHECK(ShaftInscatter(16.0f, length) < 0.06f);
        // Distant ground and the sky keep the shafts.
        CHECK(ShaftInscatter(1000.0f, length) > 0.95f);
        CHECK(ShaftInscatter(std::numeric_limits<float>::infinity(), length) == 1.0f);
        float prev = -1.0f;
        for (float d = 0.0f; d < 2000.0f; d += 50.0f)
        {
            CHECK(ShaftInscatter(d, length) > prev);
            prev = ShaftInscatter(d, length);
        }
    }
    CHECK(ShaftScatterLength(0.0f) == kShaftScatterLength); // no fog range yet
    // Thick fog scatters closer to the eye.
    CHECK(ShaftScatterLength(150.0f) == Catch::Approx(50.0f));
    CHECK(ShaftInscatter(30.0f, ShaftScatterLength(150.0f)) > 2.0f * ShaftInscatter(30.0f, ShaftScatterLength(880.0f)));

    // The composite applies the same falloff to the shaft term, with the
    // length the engine takes from ShaftScatterLength.
    const std::string composite = gl::PostShaderSource(gl::PostShader::Composite);
    REQUIRE_FALSE(composite.empty());
    CHECK(composite.find("uniform sampler2D depthTex;") != std::string::npos);
    CHECK(composite.find("1.0 - exp(-max(dist, 0.0) * depthMap.z)") != std::string::npos);
    CHECK(composite.find("shaft *= shaftInscatter()") != std::string::npos);
    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    CHECK(post.find("1.0f / scatterLength") != std::string::npos);
}

TEST_CASE("Remaster LitConstants GLSL block matches the LitConst slot layout", "[Graphics][Shaders][remaster]")
{
    namespace LC = gl::LitConst;
    // Every member of the block, in order, with its LitConst slot and size.
    struct Member
    {
        const char* name;
        int slot;
        int count;
    };
    static const Member members[] = {
        {"litSunDir", LC::SunDir, 1},
        {"litSun", LC::Sun, 1},
        {"litZenith", LC::Zenith, 1},
        {"litHorizon", LC::Horizon, 1},
        {"litGround", LC::Ground, 1},
        {"litMoonDir", LC::MoonDir, 1},
        {"litMoon", LC::Moon, 1},
        {"litCamPos", LC::CamPos, 1},
        {"litMatDiffuse", LC::MatDiffuse, 1},
        {"litMatAmbient", LC::MatAmbient, 1},
        {"litMatForced", LC::MatForced, 1},
        {"litMatEmissive", LC::MatEmissive, 1},
        {"litMatSpecular", LC::MatSpecular, 1},
        {"litTerrain", LC::Terrain, 1},
        {"litTerrainSel0", LC::TerrainSel0, 1},
        {"litTerrainSel1", LC::TerrainSel1, 1},
        {"litTerrainSun", LC::TerrainSun, 1},
        {"litTerrainLift", LC::TerrainLift, 1},
        {"litCloud", LC::Cloud, 1},
        {"litCloudOffset", LC::CloudOffset, 1},
        {"litShadowDelta", LC::ShadowDelta, 1},
        {"litFlags", LC::Flags, 1},
        {"litLightCount", LC::LightCount, 1},
        {"litHorizonSun", LC::HorizonSun, 1},
        {"litFogSun", LC::FogSun, 1},
        {"litFogCap", LC::FogCap, 1},
        {"litLightPos", LC::LightPos, LC::MaxLights},
        {"litLightColor", LC::LightColor, LC::MaxLights},
        {"litLightAmb", LC::LightAmb, LC::MaxLights},
        {"litLightDir", LC::LightDir, LC::MaxLights},
    };
    // Walks the members of one declaration, slot by slot.
    auto checkBlock = [](const std::string& block)
    {
        static const std::regex member(R"(vec4\s+(\w+)(?:\[(\d+)\])?\s*;)");
        int at = 0;
        size_t i = 0;
        for (auto it = std::sregex_iterator(block.begin(), block.end(), member); it != std::sregex_iterator(); ++it)
        {
            const std::string name = (*it)[1].str();
            const int count = (*it)[2].matched ? std::stoi((*it)[2].str()) : 1;
            CAPTURE(name, at);
            REQUIRE(i < std::size(members));
            CHECK(name == members[i].name);
            CHECK(at == members[i].slot);
            CHECK(count == members[i].count);
            at += count;
            ++i;
        }
        CHECK(i == std::size(members));
        CHECK(at == LC::Count);
    };
    auto blockAt = [](const std::string& text, size_t open)
    {
        const size_t close = text.find("};", open);
        return close == std::string::npos ? std::string() : text.substr(open, close - open);
    };

    const std::string shared = gl::LitConstantsBlock();
    const size_t sharedOpen = shared.find("uniform LitConstants {");
    REQUIRE(sharedOpen != std::string::npos);
    checkBlock(blockAt(shared, sharedOpen));

    // Every declaration in the backend's sources, slot by slot, and there is
    // only the one: a hand copy (four prototypes pasted their own into post
    // and vertex shaders) would drift from the layout.
    int declarations = 0;
    for (const auto& entry : std::filesystem::directory_iterator(Gl33Dir()))
    {
        const auto ext = entry.path().extension().string();
        if (!entry.is_regular_file() || (ext != ".cpp" && ext != ".hpp"))
            continue;
        const std::string text = ReadTextFile(entry.path());
        for (size_t at = text.find("uniform LitConstants {"); at != std::string::npos;
             at = text.find("uniform LitConstants {", at + 1))
        {
            CAPTURE(entry.path().filename().string());
            checkBlock(blockAt(text, at));
            ++declarations;
        }
    }
    CHECK(declarations == 1);

    // A program that reads the rig pastes the shared block whole, once.
    for (const std::string& program : AssembledPrograms())
        if (program.find("uniform LitConstants") != std::string::npos)
            CHECK(CountOf(program, shared) == 1);
    CHECK(CountOf(gl::RemasterFragmentCommon(), shared) == 1);

    // The planned slots continue the declared ones.
    CHECK(gl::LitConstPlanned::MatClass == LC::Count);

    // The decoded horizon range in the shader is the encoder's.
    std::ostringstream range;
    range << "mix(" << std::fixed;
    range.precision(1);
    range << TerrainHorizonMap::kMinAngleDeg << ", " << TerrainHorizonMap::kMaxAngleDeg << ",";
    CHECK(Piece(gl::FragmentPiece::TerrainClouds).find(range.str()) != std::string::npos);
}

TEST_CASE("Remaster texture units come from one table that binds every sampler", "[Graphics][Shaders][remaster]")
{
    // The table's units are distinct, clear of the material units 0 and 1 and
    // the upload unit 7, and within the 16 units GL 3.3 guarantees.
    std::set<int> units;
    std::set<std::string> names;
    for (const auto& s : gl::kLitSamplers)
    {
        CAPTURE(s.name, s.unit);
        CHECK(s.unit >= 2);
        CHECK(s.unit != 7);
        CHECK(s.unit <= 15);
        CHECK(units.insert(s.unit).second);
        CHECK(names.insert(s.name).second);
    }
    // The shadow-map audits of test_gl_state_cache_audit.cpp pin binds to this
    // constant; the shaders' comments and the classic programs say unit 2.
    CHECK(gl::kShadowMapUnit == 2);

    // Every sampler a remastered program declares has its unit in the table
    // (tex0 and tex1 are the material units): an unbound sampler reads unit 0.
    std::set<std::string> declared;
    static const std::regex sampler(R"(uniform\s+sampler\w+\s+(\w+)\s*;)");
    for (const std::string& program : RemasterPrograms())
        for (auto it = std::sregex_iterator(program.begin(), program.end(), sampler); it != std::sregex_iterator(); ++it)
            declared.insert((*it)[1].str());
    for (const std::string& name : declared)
    {
        CAPTURE(name);
        CHECK((name == "tex0" || name == "tex1" || names.count(name) == 1));
    }
    // And every row names a sampler the remastered programs declare.
    for (const std::string& name : names)
    {
        CAPTURE(name);
        CHECK(declared.count(name) == 1);
    }
}
