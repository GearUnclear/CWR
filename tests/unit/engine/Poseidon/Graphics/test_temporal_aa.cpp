#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Remaster/TemporalAA.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <utility>

// Temporal anti-aliasing (Graphics/Remaster/TemporalAA.hpp): the jitter
// sequence and its range, the reprojection the resolve shader applies (its C++
// mirror against hand values and against the camera transforms it stands for),
// the history chain, and the backend contracts that keep the jitter off the HUD
// and the shadow-map pass.

using namespace Poseidon::remaster;
namespace gl = Poseidon::gl33lighting;

namespace
{

std::string ReadTextFile(const std::filesystem::path& p)
{
    std::ifstream f(p);
    if (!f.is_open())
        return {};
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path EngineDir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine";
}

std::filesystem::path Gl33Dir()
{
    return EngineDir() / "PoseidonGL33";
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

// Row-vector view rotation (p_view = p_rel * R) of a camera turned by `yawDeg`
// about the vertical and pitched by `pitchDeg`, rows _11.._33.
void ViewRotation(double yawDeg, double pitchDeg, double r[9])
{
    const double y = yawDeg * 3.14159265358979 / 180.0;
    const double p = pitchDeg * 3.14159265358979 / 180.0;
    // Camera axes in world space (x right, y up, z forward).
    const double fwd[3] = {std::sin(y) * std::cos(p), std::sin(p), std::cos(y) * std::cos(p)};
    const double right[3] = {std::cos(y), 0.0, -std::sin(y)};
    const double up[3] = {fwd[1] * right[2] - fwd[2] * right[1], fwd[2] * right[0] - fwd[0] * right[2],
                          fwd[0] * right[1] - fwd[1] * right[0]};
    // p_view.c = p_rel . axis_c: the axes are the columns of R.
    for (int row = 0; row < 3; ++row)
    {
        r[row * 3 + 0] = right[row];
        r[row * 3 + 1] = up[row];
        r[row * 3 + 2] = fwd[row];
    }
}

TaaCamera MakeCamera(double yawDeg, double pitchDeg, double x, double y, double z, double p11 = 1.43, double p22 = 2.54)
{
    TaaCamera c;
    ViewRotation(yawDeg, pitchDeg, c.rot);
    c.pos[0] = x;
    c.pos[1] = y;
    c.pos[2] = z;
    c.p11 = p11;
    c.p22 = p22;
    return c;
}

// p_view of world point w for camera c.
void ToView(const TaaCamera& c, const double w[3], double out[3])
{
    const double rel[3] = {w[0] - c.pos[0], w[1] - c.pos[1], w[2] - c.pos[2]};
    for (int col = 0; col < 3; ++col)
        out[col] = rel[0] * c.rot[0 * 3 + col] + rel[1] * c.rot[1 * 3 + col] + rel[2] * c.rot[2 * 3 + col];
}

// The shader's rows applied to p (float rows, double arithmetic).
void Apply(const TaaReprojection& r, const double p[3], double out[3])
{
    for (int c = 0; c < 3; ++c)
        out[c] = r.rows[c][0] * p[0] + r.rows[c][1] * p[1] + r.rows[c][2] * p[2] + r.rows[c][3];
}

} // namespace

TEST_CASE("TAA jitter: Halton(2, 3) over 16 frames, inside the pixel", "[remaster][TemporalAA]")
{
    CHECK(Halton(1, 2) == 0.5f);
    CHECK(Halton(2, 2) == 0.25f);
    CHECK(Halton(3, 2) == 0.75f);
    CHECK(Halton(1, 3) == Catch::Approx(1.0 / 3.0));
    CHECK(Halton(2, 3) == Catch::Approx(2.0 / 3.0));
    CHECK(Halton(4, 3) == Catch::Approx(4.0 / 9.0));
    CHECK(Halton(0, 2) == 0.0f);

    std::set<std::pair<float, float>> seen;
    double sx = 0.0, sy = 0.0;
    for (std::uint32_t f = 0; f < kTaaJitterCycle; ++f)
    {
        const TaaOffset j = TaaJitterPixels(f);
        CAPTURE(f, j.x, j.y);
        CHECK(j.x >= -0.5f);
        CHECK(j.x < 0.5f);
        CHECK(j.y >= -0.5f);
        CHECK(j.y < 0.5f);
        CHECK(seen.insert({j.x, j.y}).second); // 16 distinct positions
        sx += j.x;
        sy += j.y;
        // The cycle repeats.
        const TaaOffset again = TaaJitterPixels(f + kTaaJitterCycle);
        CHECK(again.x == j.x);
        CHECK(again.y == j.y);
    }
    // Over a cycle the offsets centre on the pixel (no net shift of the image).
    CHECK(std::fabs(sx / kTaaJitterCycle) < 0.04);
    CHECK(std::fabs(sy / kTaaJitterCycle) < 0.04);
    // Index 0 of the sequence (the pixel corner, 0 - 0.5 on both axes) is not used.
    const TaaOffset first = TaaJitterPixels(0);
    CHECK(first.x == 0.0f);
    CHECK(first.y == Catch::Approx(1.0 / 3.0 - 0.5));
}

TEST_CASE("TAA jitter: an NDC offset of the 3D viewport, only in a TAA frame's world pass", "[remaster][TemporalAA]")
{
    const TaaOffset px{0.25f, -0.5f};
    const TaaOffset ndc = TaaJitterNdc(px, 1920, 1080);
    CHECK(ndc.x == Catch::Approx(0.5 / 1920.0));
    CHECK(ndc.y == Catch::Approx(-1.0 / 1080.0));
    CHECK(TaaJitterNdc(px, 0, 1080).x == 0.0f);
    CHECK(TaaJitterNdc(px, 1920, 0).y == 0.0f);

    // The HUD, dialogs and 3D previews (no world pass) and every
    // pass of a frame without TAA get exactly zero, which leaves the vertex
    // shaders' positions as they were (the shaders skip the add at zero).
    for (bool world : {false, true})
        for (bool taa : {false, true})
        {
            CAPTURE(world, taa);
            const TaaOffset j = TaaPassJitterNdc(world, taa, px, 1920, 1080);
            if (world && taa)
            {
                CHECK(j.x == ndc.x);
                CHECK(j.y == ndc.y);
            }
            else
            {
                CHECK(j.x == 0.0f);
                CHECK(j.y == 0.0f);
            }
        }
}

TEST_CASE("TAA reprojection: hand values", "[remaster][TemporalAA]")
{
    // A camera that did not move maps every point to itself.
    const TaaCamera still = MakeCamera(30.0, -5.0, 4674.0, 61.0, 10156.0);
    TaaReprojection r = BuildTaaReprojection(still, still);
    CHECK(r.valid);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 4; ++j)
            CHECK(r.rows[i][j] == Catch::Approx(i == j ? 1.0 : 0.0).margin(1e-6));

    // The camera stepped 1 m right (identity rotation): a point is 1 m further
    // right in the previous view, where the camera stood 1 m to the left.
    TaaCamera prev;
    TaaCamera cur;
    cur.pos[0] = 1.0;
    r = BuildTaaReprojection(prev, cur);
    CHECK(r.valid);
    CHECK(r.move == Catch::Approx(1.0));
    CHECK(r.rows[0][3] == Catch::Approx(1.0));
    CHECK(r.rows[1][3] == Catch::Approx(0.0).margin(1e-9));
    CHECK(r.rows[2][3] == Catch::Approx(0.0).margin(1e-9));

    // The camera turned 45 degrees right on the spot: a point straight ahead
    // now was 45 degrees right of the old view direction, at the same distance.
    const TaaCamera ahead = MakeCamera(0.0, 0.0, 0, 0, 0);
    const TaaCamera turned = MakeCamera(45.0, 0.0, 0, 0, 0);
    r = BuildTaaReprojection(ahead, turned);
    CHECK(r.valid);
    CHECK(r.forwardCos == Catch::Approx(std::sqrt(0.5)));
    const double p[3] = {0.0, 0.0, 10.0};
    double q[3];
    Apply(r, p, q);
    CHECK(q[0] == Catch::Approx(10.0 * std::sqrt(0.5)));
    CHECK(q[1] == Catch::Approx(0.0).margin(1e-6));
    CHECK(q[2] == Catch::Approx(10.0 * std::sqrt(0.5)));
    // Projected with the previous projection: x * p11 / z.
    double ndc[2];
    REQUIRE(TaaReprojectNdc(r, p, ndc));
    CHECK(ndc[0] == Catch::Approx(turned.p11 * 1.0)); // the previous camera's p11 (same lens)
    CHECK(ndc[1] == Catch::Approx(0.0).margin(1e-6));
    // A point behind the previous camera has no history.
    const double behind[3] = {0.0, 0.0, -10.0};
    CHECK_FALSE(TaaReprojectNdc(r, behind, ndc));
}

TEST_CASE("TAA reprojection: the shader's rows take this frame's view position to the previous one's",
          "[remaster][TemporalAA]")
{
    // Camera pairs as a frame sees them: a walk, a strafe with a pan, a jeep
    // ride far from the map origin (float positions at 12 km), a zoom.
    struct Pair
    {
        TaaCamera prev, cur;
    };
    const Pair pairs[] = {
        {MakeCamera(75.0, 3.0, 4674.0, 61.75, 10156.0), MakeCamera(75.0, 3.0, 4674.03, 61.75, 10156.0)},
        {MakeCamera(338.8, -3.0, 8350.0, 20.0, 2950.0), MakeCamera(338.9, -3.0, 8350.0, 20.0, 2950.0)},
        {MakeCamera(120.0, -2.0, 12000.0, 80.0, 9000.0), MakeCamera(121.5, -1.0, 12000.4, 80.05, 9000.3)},
        {MakeCamera(200.0, 10.0, 600.0, 15.0, 700.0, 1.43, 2.54),
         MakeCamera(200.0, 10.0, 600.0, 15.0, 700.0, 4.0, 7.1)},
    };
    const double points[][3] = {{0, 0, 0}, {25, 1, 40}, {-300, 50, 900}, {5, -2, 3}};
    for (const Pair& pr : pairs)
    {
        const TaaReprojection r = BuildTaaReprojection(pr.prev, pr.cur);
        CHECK(r.valid);
        for (const auto& offset : points)
        {
            // A world point some way in front of the current camera.
            double fwd[3];
            for (int i = 0; i < 3; ++i)
                fwd[i] = pr.cur.rot[i * 3 + 2];
            const double w[3] = {pr.cur.pos[0] + fwd[0] * 60.0 + offset[0], pr.cur.pos[1] + fwd[1] * 60.0 + offset[1],
                                 pr.cur.pos[2] + fwd[2] * 60.0 + offset[2]};
            double pc[3], pp[3], got[3];
            ToView(pr.cur, w, pc);
            ToView(pr.prev, w, pp);
            Apply(r, pc, got);
            const double size = std::sqrt(pc[0] * pc[0] + pc[1] * pc[1] + pc[2] * pc[2]);
            CAPTURE(pc[0], pc[1], pc[2], pp[0], pp[1], pp[2], got[0], got[1], got[2]);
            for (int i = 0; i < 3; ++i)
                CHECK(std::fabs(got[i] - pp[i]) < 2e-6 * size + 1e-4);
            // On screen, the previous frame's position is right to well under a
            // hundredth of a pixel at 1080p.
            double ndc[2];
            if (pp[2] > 1.0 && TaaReprojectNdc(r, pc, ndc))
            {
                CHECK(std::fabs(ndc[0] - pp[0] * pr.prev.p11 / pp[2]) * 960.0 < 0.01);
                CHECK(std::fabs(ndc[1] - pp[1] * pr.prev.p22 / pp[2]) * 540.0 < 0.01);
            }
        }
    }
}

TEST_CASE("TAA reprojection: a cut or a singular view drops the history", "[remaster][TemporalAA]")
{
    const TaaCamera a = MakeCamera(10.0, 0.0, 100.0, 5.0, 100.0);
    TaaCamera b = a;
    b.pos[0] += kTaaCutDistance - 1.0;
    CHECK(BuildTaaReprojection(a, b).valid);
    b.pos[0] = a.pos[0] + kTaaCutDistance + 0.1; // a teleport
    CHECK_FALSE(BuildTaaReprojection(a, b).valid);
    CHECK(BuildTaaReprojection(a, MakeCamera(10.0 + 59.0, 0.0, 100.0, 5.0, 100.0)).valid);
    CHECK_FALSE(
        BuildTaaReprojection(a, MakeCamera(10.0 + 61.0, 0.0, 100.0, 5.0, 100.0)).valid); // a cut to another view
    TaaCamera flat = a;
    for (double& v : flat.rot)
        v = 0.0;
    CHECK_FALSE(BuildTaaReprojection(a, flat).valid);
}

TEST_CASE("TAA history: only the last frame's, at this size, without a cut", "[remaster][TemporalAA]")
{
    TaaHistory h;
    CHECK_FALSE(TaaHistoryUsable(h, 101, 1920, 1080, true)); // none written yet
    h.exists = true;
    h.frame = 100;
    h.w = 1920;
    h.h = 1080;
    CHECK(TaaHistoryUsable(h, 101, 1920, 1080, true));
    // A frame without a resolve in between (an interior, optics, night vision,
    // the switch or the post effects off): stale.
    CHECK_FALSE(TaaHistoryUsable(h, 102, 1920, 1080, true));
    CHECK_FALSE(TaaHistoryUsable(h, 100, 1920, 1080, true));
    CHECK_FALSE(TaaHistoryUsable(h, 101, 1600, 900, true));   // resized
    CHECK_FALSE(TaaHistoryUsable(h, 101, 1920, 1080, false)); // camera cut
    h.frame = 0xffffffffu;
    CHECK(TaaHistoryUsable(h, 0, 1920, 1080, true)); // the counter wraps
}

TEST_CASE("TAA shaders: the jitter only in the remastered world programs, and only when set",
          "[remaster][TemporalAA][Shaders]")
{
    // Every remastered vertex shader adds the jitter once, guarded so a zero
    // jitter leaves the position exactly as computed (the switch's "off" is
    // the look before TAA).
    const std::string add = "if (vpScale.z != 0.0 || vpScale.w != 0.0) gl_Position.xy += vpScale.zw * gl_Position.w;";
    int vertex = 0;
    for (int vs = 0; vs < 16; ++vs)
    {
        const char* src = gl::RemasterVertexSource(vs);
        if (!src)
            continue;
        CAPTURE(vs);
        CHECK(CountOf(src, add) == 1);
        CHECK(CountOf(src, "vpScale.zw") == 1);
        ++vertex;
    }
    CHECK(vertex == 3);
    // Classic programs, the shadow-map depth pass and the post pass never
    // read it: Classic stays the original look and the shadow map unjittered.
    for (const char* file : {"EngineGL33_Shaders.cpp", "EngineGL33_ShadowDepth.cpp", "EngineGL33_PostShaders.cpp"})
    {
        CAPTURE(file);
        const std::string text = ReadTextFile(Gl33Dir() / file);
        REQUIRE_FALSE(text.empty());
        CHECK(text.find("vpScale.zw") == std::string::npos);
    }
    CHECK(ReadTextFile(Gl33Dir() / "EngineGL33_ShadowDepth.cpp").find("vpScale") == std::string::npos);
}

TEST_CASE("TAA shaders: the resolve and sharpen contracts", "[remaster][TemporalAA][Shaders]")
{
    const std::string taa = gl::PostShaderSource(gl::PostShader::Taa);
    const std::string sharpen = gl::PostShaderSource(gl::PostShader::TaaSharpen);
    REQUIRE_FALSE(taa.empty());
    REQUIRE_FALSE(sharpen.empty());
    CHECK(std::string(gl::PostShaderName(gl::PostShader::Taa)) == "psPostTaa");
    CHECK(std::string(gl::PostShaderName(gl::PostShader::TaaSharpen)) == "psPostTaaSharpen");
    // Its five inputs, on the post units EngineGL33_PostTemporal.cpp binds.
    for (const char* sampler : {"curTex", "depthTex", "histTex", "worldTex", "opaqueTex"})
    {
        CAPTURE(sampler);
        CHECK(taa.find(std::string("uniform sampler2D ") + sampler + ";") != std::string::npos);
    }
    CHECK(CountOf(taa, "uniform sampler2D") == 5);
    // The nearest depth of the 3x3 drives the reprojection, the history is a
    // Catmull-Rom read clipped into this frame's YCoCg box, the depth test
    // uses the stored view depth with 3% + 0.15 m, and the view depth goes
    // out with the colour.
    CHECK(taa.find("if (dz < closest) { closest = dz; closestOff = ivec2(x, y); }") != std::string::npos);
    // The reconstruction kernel sets how soft the converged frame is: exp(-4 d^2)
    // (5% softer on a test view against 15% with exp(-2.29 d^2)).
    CHECK(taa.find("float w = exp(-4.0 * dot(d, d));") != std::string::npos);
    CHECK(taa.find("vec3 sampleHistory(vec2 uv)") != std::string::npos);
    CHECK(taa.find("clipToBox(max(mu - sigma, cmin), min(mu + sigma, cmax), toYCoCg(hist))") != std::string::npos);
    CHECK(taa.find("float tol = 0.03 * pp.z + 0.15;") != std::string::npos);
    CHECK(taa.find("fragColor = vec4(max(res, vec3(0.0)), min(viewDepth(d0), 65000.0));") != std::string::npos);
    // The particle mask: the translucent objects' change to the world, by the
    // thresholds of TemporalAA.hpp (passed as uniforms).
    CHECK(taa.find("abs(texelFetch(worldTex, ip, 0).rgb - texelFetch(opaqueTex, ip, 0).rgb)") != std::string::npos);
    CHECK(taa.find("alpha = max(alpha, r * reactive.z);") != std::string::npos);
    CHECK(kTaaReactiveLow < kTaaReactiveHigh);
    CHECK(kTaaReactiveMaxAlpha > 0.5f);
    CHECK(kTaaReactiveMaxAlpha <= 1.0f);
    // The sharpen cannot overshoot the local cross by more than a quarter of
    // its range (no halos) and dithers back into 8 bits.
    CHECK(sharpen.find("vec3 margin = 0.25 * (mx - mn);") != std::string::npos);
    CHECK(sharpen.find("clamp(c + sharpen.x * (c - 0.25 * (n + s + e + w)), mn - margin, mx + margin)") !=
          std::string::npos);
    CHECK(sharpen.find("(a + b - 1.0) / 255.0") != std::string::npos);
}

TEST_CASE("TAA backend: the jitter and the texture LOD bias never reach the HUD",
          "[remaster][TemporalAA][GL33][StateCacheAudit]")
{
    // EndWorldScene takes both off before anything can return early, so the
    // map, HUD and dialogs drawn after it are never jittered; the next frame's
    // InitDraw does it again for a frame that never reached EndWorldScene.
    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    const std::string end = FunctionBody(post, "void EngineGL33::EndWorldScene()");
    REQUIRE_FALSE(end.empty());
    const size_t clear = end.find("SetTaaJitterState({}, 0.0f);");
    REQUIRE(clear != std::string::npos);
    const size_t firstReturn = end.find("return");
    REQUIRE(firstReturn != std::string::npos);
    CHECK(clear < firstReturn);
    const std::string life = ReadTextFile(Gl33Dir() / "EngineGL33_Lifecycle.cpp");
    CHECK(FunctionBody(life, "void EngineGL33::InitDraw(").find("SetTaaJitterState({}, 0.0f);") != std::string::npos);

    // The world's screen-space draws (smoke, rain) take the jitter the world
    // passes hold, and nothing else writes vpScale.zw.
    const std::string shaders = ReadTextFile(Gl33Dir() / "EngineGL33_Shaders.cpp");
    const std::string screen = FunctionBody(shaders, "void EngineGL33::UploadVSScreenConstants()");
    CHECK(screen.find("_taa.ndcApplied.x, _taa.ndcApplied.y") != std::string::npos);
    CHECK(CountOf(shaders, "VSConst::SlotVpScale * 4 + 2") == 1);

    // One rule decides a pass's jitter (TaaPassJitterNdc), applied at every
    // world pass once the frame gates are known.
    const std::string temporal = ReadTextFile(Gl33Dir() / "EngineGL33_PostTemporal.cpp");
    REQUIRE_FALSE(temporal.empty());
    CHECK(FunctionBody(temporal, "void EngineGL33::ApplyTaaJitter()").find("rm::TaaPassJitterNdc(") !=
          std::string::npos);
    const std::string queue = ReadTextFile(Gl33Dir() / "EngineGL33_Queue.cpp");
    const std::string begin = FunctionBody(queue, "void EngineGL33::BeginPass(PassId passId)");
    const size_t gates = begin.find("UploadLitFrameConstants();");
    const size_t jitter = begin.find("ApplyTaaJitter();");
    REQUIRE(gates != std::string::npos);
    REQUIRE(jitter != std::string::npos);
    CHECK(gates < jitter);

    // The LOD bias has one writer, which also takes it off.
    int biasWriters = 0;
    for (const auto& entry : std::filesystem::directory_iterator(Gl33Dir()))
        if (entry.path().extension() == ".cpp")
            biasWriters += CountOf(ReadTextFile(entry.path()), "GL_TEXTURE_LOD_BIAS");
    CHECK(biasWriters == 1);
    CHECK(FunctionBody(temporal, "void EngineGL33::SetTaaJitterState(").find("GL_TEXTURE_LOD_BIAS") !=
          std::string::npos);
}

TEST_CASE("TAA backend: the opaque world is copied between the opaque and the translucent objects",
          "[remaster][TemporalAA][GL33]")
{
    // The particle mask compares the world as drawn with the world before the
    // translucent objects: the scene calls the hook once, after the opaque
    // objects, the surface overlays and the shadows, right before the
    // back-to-front loop of the alpha objects.
    const std::string scene = ReadTextFile(EngineDir() / "Poseidon" / "World" / "Scene" / "SceneDraw.cpp");
    REQUIRE_FALSE(scene.empty());
    CHECK(CountOf(scene, "GEngine->BeginTranslucentObjects();") == 1);
    const std::string pass2 = FunctionBody(scene, "void Scene::DrawObjectsAndShadowsPass2()");
    const size_t hook = pass2.find("GEngine->BeginTranslucentObjects();");
    REQUIRE(hook != std::string::npos);
    CHECK(hook > pass2.find("RenderShadowMapDepthPass(nDraw);"));
    CHECK(hook < pass2.find("QSort(_drawMergers.Data(), _drawMergers.Size(), CmpRevAlphaSortObj);"));
    // The copy needs no new target mid-frame (that would reset a cropped
    // world's viewport) and keeps the world's viewport.
    const std::string temporal = ReadTextFile(Gl33Dir() / "EngineGL33_PostTemporal.cpp");
    const std::string copy = FunctionBody(temporal, "void EngineGL33::BeginTranslucentObjects()");
    REQUIRE_FALSE(copy.empty());
    CHECK(copy.find("EnsurePostResources") == std::string::npos);
    CHECK(copy.find("glViewport") == std::string::npos);
    CHECK(copy.find("BindFrameRenderTarget") == std::string::npos);
}
