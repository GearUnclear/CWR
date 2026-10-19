#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/ScreenReflections.hpp>
#include <Poseidon/Graphics/Rendering/BuildRenderPassDescriptor.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// Screen-space reflections (feature switch ssr, INTEGRATION-PLAN.md section 5
// B6; Graphics/Remaster/ScreenReflections.hpp): the stencil tag code table and
// the per-draw stencil state, the march's hit logic as a CPU mirror of the
// shader on a synthetic depth row, the water composite that swaps the sea's
// sky share for the reflection, and the contracts that tie the shaders and the
// backend to them.

using namespace Poseidon::remaster;
namespace render = Poseidon::render;
namespace gl = Poseidon::gl33lighting;

namespace
{

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

int CountOf(const std::string& haystack, const std::string& needle)
{
    int n = 0;
    for (size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + needle.size()))
        ++n;
    return n;
}

// The text of a GLSL function from its signature line to its closing brace.
std::string FunctionText(const std::string& src, const std::string& signature)
{
    const size_t at = src.find(signature);
    if (at == std::string::npos)
        return {};
    const size_t end = src.find("\n}\n", at);
    return end == std::string::npos ? std::string() : src.substr(at, end + 3 - at);
}

render::RenderPassDescriptor Descriptor(render::Backend backend, render::Routing routing = render::Routing::None,
                                        bool routedSea = false)
{
    render::LegacySpec spec;
    spec.backend = backend;
    spec.routing = routing;
    render::BuildContext ctx;
    ctx.isIn3DPass = true;
    ctx.isMultitexturing = true;
    ctx.remasteredSea = routedSea;
    return render::BuildRenderPassDescriptor(spec, ctx);
}

// The draws of a world pass, as the landscape and the objects make them.
struct DrawCase
{
    const char* what;
    render::RenderPassDescriptor d;
    bool land;
};

std::vector<DrawCase> WorldDraws()
{
    using B = render::Backend;
    using R = render::Routing;
    return {
        {"land", Descriptor(B::DetailTexture), true},
        {"sea (water program)", Descriptor(B::SpecularTexture, R::None, true), false},
        {"sea (detail program)", Descriptor(B::SpecularTexture), false},
        {"grass layer", Descriptor(B::GrassTexture), false},
        {"road", Descriptor(B::NoZWrite, R::OnSurface), false},
        {"opaque object", Descriptor(B::None), false},
        {"cutout foliage", Descriptor(B::IsTransparent), false},
        {"bush card (alpha texture)", Descriptor(B::IsAlpha), false},
        {"particle", Descriptor(B::IsAlpha | B::NoZWrite), false},
        {"flare", Descriptor(B::NoZBuf), false},
        {"projected shadow", Descriptor(B::IsShadow), false},
    };
}

SsrStencil For(const std::vector<DrawCase>& draws, const char* what, bool tagging = true)
{
    for (const DrawCase& c : draws)
        if (std::string_view(c.what) == what)
            return SsrStencilForDraw(c.d, c.land, tagging);
    FAIL("no draw " << what);
    return {};
}

} // namespace

TEST_CASE("Reflection tags: the top bits name the surface and the cover, clear of the projected-shadow counter",
          "[remaster][ssr]")
{
    // Classic's projected shadows count in the low bits (EQUAL 0, then INCR):
    // no tag bit may overlap them.
    for (std::uint8_t tag : {ssrtag::kTerrain, ssrtag::kWater, ssrtag::kRoad, ssrtag::kCover})
    {
        CAPTURE(int(tag));
        CHECK((tag & ssrtag::kShadowBits) == 0);
    }
    CHECK((ssrtag::kSurfaceBits & ssrtag::kCover) == 0);
    CHECK((ssrtag::kSurfaceBits | ssrtag::kCover | ssrtag::kShadowBits) == 0xFF);
    // The road is both surface bits: the readout adds the bits up.
    CHECK(ssrtag::kRoad == (ssrtag::kTerrain | ssrtag::kWater));
    CHECK(SsrSurfaceOfStencil(0x00) == SsrSurface::None);
    CHECK(SsrSurfaceOfStencil(ssrtag::kTerrain | 0x01) == SsrSurface::Terrain);
    CHECK(SsrSurfaceOfStencil(ssrtag::kWater | ssrtag::kCover) == SsrSurface::Water);
    CHECK(SsrSurfaceOfStencil(ssrtag::kRoad) == SsrSurface::Road);
    CHECK(SsrCoveredStencil(ssrtag::kTerrain | ssrtag::kCover));
    CHECK_FALSE(SsrCoveredStencil(ssrtag::kRoad | ssrtag::kShadowBits));
}

TEST_CASE("Reflection tags: each world draw's stencil state", "[remaster][ssr]")
{
    const auto draws = WorldDraws();
    // The descriptors are the ones the backend gets.
    REQUIRE(draws[1].d.shader == render::ShaderFamily::Water);
    REQUIRE(draws[2].d.shader == render::ShaderFamily::Detail);
    REQUIRE(draws[4].d.surface == render::SurfaceMode::OnSurface);
    REQUIRE(draws[7].d.blend == render::BlendMode::AlphaBlend);
    REQUIRE(draws[7].d.depth == render::DepthMode::Normal);
    REQUIRE(draws[8].d.depth == render::DepthMode::ReadOnly);
    REQUIRE(draws[10].d.depth == render::DepthMode::Shadow);

    // Surfaces write their tag into every bit.
    CHECK(For(draws, "land") == SsrStencil{ssrtag::kTerrain, 0xFF, 0xFF});
    CHECK(For(draws, "grass layer") == SsrStencil{ssrtag::kTerrain, 0xFF, 0xFF});
    CHECK(For(draws, "sea (water program)") == SsrStencil{ssrtag::kWater, 0xFF, 0xFF});
    CHECK(For(draws, "road") == SsrStencil{ssrtag::kRoad, 0xFF, 0xFF});
    // The sea drawn by the detail program (seaWaves off) is no surface: its
    // reflection would replace a sky term that program does not have.
    CHECK(For(draws, "sea (detail program)") == SsrStencil{0, 0xFF, 0xFF});
    // An opaque draw that writes depth hides the surface.
    CHECK(For(draws, "opaque object") == SsrStencil{0, 0xFF, 0xFF});
    CHECK(For(draws, "cutout foliage") == SsrStencil{0, 0xFF, 0xFF});
    // A blended draw that writes depth covers it, keeping the surface bits.
    CHECK(For(draws, "bush card (alpha texture)") ==
          SsrStencil{ssrtag::kCover, ssrtag::kCover | ssrtag::kShadowBits, 0xFF});
    // A draw without depth writes resets the shadow bits only, as before.
    CHECK(For(draws, "particle") == SsrStencil{0, ssrtag::kShadowBits, 0xFF});
    CHECK(For(draws, "flare") == SsrStencil{0, ssrtag::kShadowBits, 0xFF});
    // The projected shadows compare the shadow bits only.
    CHECK(For(draws, "projected shadow") == SsrStencil{0, 0xFF, ssrtag::kShadowBits});

    // Without tagging (the switch off, Classic, the HUD) every draw gets the
    // state of before: REPLACE 0 into every bit, the shadows compare every bit.
    for (const DrawCase& c : draws)
    {
        CAPTURE(c.what);
        CHECK(SsrStencilForDraw(c.d, c.land, false) == SsrStencil{});
    }
    // The depth-stencil bundles' default tag is that state too.
    const std::string ds = ReadTextFile(std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine" / "Poseidon" /
                                        "Graphics" / "Core" / "GLDepthStencilState.hpp");
    for (const char* member : {"GLint ref = 0;", "GLuint writeMask = 0xFF;", "GLuint shadowCompareMask = 0xFF;"})
    {
        CAPTURE(member);
        CHECK(CountOf(ds, member) == 1);
    }
}

TEST_CASE("Reflection tags: a pixel's stencil through a world pass keeps the surface it shows", "[remaster][ssr]")
{
    const auto draws = WorldDraws();
    auto draw = [&](std::uint8_t stored, const char* what)
    {
        const SsrStencil s = For(draws, what);
        if (std::string_view(what) == "projected shadow")
        {
            bool passed = false;
            return SsrShadowDraw(stored, s, passed);
        }
        return SsrReplace(stored, s);
    };
    // The land, a bush card over it, smoke over that: still land, covered.
    std::uint8_t px = draw(0, "land");
    CHECK(px == ssrtag::kTerrain);
    px = draw(px, "bush card (alpha texture)");
    CHECK(SsrSurfaceOfStencil(px) == SsrSurface::Terrain);
    CHECK(SsrCoveredStencil(px));
    px = draw(px, "particle");
    CHECK(SsrSurfaceOfStencil(px) == SsrSurface::Terrain);
    CHECK(SsrCoveredStencil(px));
    // A road drawn on the land takes the pixel, uncovered.
    px = draw(ssrtag::kTerrain, "road");
    CHECK(px == ssrtag::kRoad);
    // An opaque object in front clears it.
    CHECK(draw(px, "opaque object") == 0);

    // A projected shadow still lands once on a tagged surface, and only once.
    bool passed = false;
    const SsrStencil shadow = For(draws, "projected shadow");
    std::uint8_t sea = draw(0, "sea (water program)");
    sea = SsrShadowDraw(sea, shadow, passed);
    CHECK(passed);
    CHECK(SsrSurfaceOfStencil(sea) == SsrSurface::Water);
    CHECK((sea & ssrtag::kShadowBits) == 1);
    SsrShadowDraw(sea, shadow, passed);
    CHECK_FALSE(passed);
    // A draw without depth writes resets the counter for the next caster, as
    // REPLACE 0 did before the tags, and keeps the tag.
    sea = draw(sea, "flare");
    CHECK(sea == ssrtag::kWater);
    SsrShadowDraw(sea, shadow, passed);
    CHECK(passed);

    // Untagged (the classic sequence): the counter blocks a second shadow and
    // REPLACE 0 resets it, exactly as before.
    const SsrStencil plain{};
    std::uint8_t classic = SsrReplace(0x37, plain);
    CHECK(classic == 0);
    classic = SsrShadowDraw(classic, plain, passed);
    CHECK(passed);
    SsrShadowDraw(classic, plain, passed);
    CHECK_FALSE(passed);
}

TEST_CASE("Reflection march: the CPU mirror finds the wall, passes thin gaps and lets cards through", "[remaster][ssr]")
{
    // A ray from 10 m to 60 m view depth across 64 texels (texel centres 0.5
    // to 63.5), the sky behind, a wall 30 m away at texels 45 to 60. The ray
    // reaches 30 m at t = (1/10 - 1/30) / (1/10 - 1/60) = 0.8 (texel 51).
    const int n = 64;
    const int wallA = 45, wallB = 60;
    auto wallRow = [&](float depth)
    {
        std::vector<float> row(n, 1.0e6f);
        for (int x = wallA; x <= wallB; ++x)
            row[x] = depth;
        return row;
    };
    const float rz = 0.8f; // view z per metre along the ray
    std::vector<float> row = wallRow(30.0f);
    SsrRowHit hit = SsrMarchRow(row.data(), nullptr, n, 10.0f, 60.0f, rz, 0.5f);
    REQUIRE(hit.kind == SsrHit::Surface);
    CHECK(hit.t == Catch::Approx(0.8f).margin(0.03f));
    CHECK(hit.travel == Catch::Approx((30.0f - 10.0f) / rz).margin(1.5f));
    // Any jitter of the march start finds the same wall.
    for (float j : {0.0f, 0.25f, 0.99f})
    {
        CAPTURE(j);
        const SsrRowHit h = SsrMarchRow(row.data(), nullptr, n, 10.0f, 60.0f, rz, j);
        CHECK(h.kind == SsrHit::Surface);
        CHECK(h.t == Catch::Approx(0.8f).margin(0.03f));
    }

    // A wall well in front of the ray (closer to the eye than the ray by more
    // than the thickness) is a gap the ray passes behind: no hit; the row
    // ends on the sky.
    row = wallRow(3.0f);
    const SsrRowHit gap = SsrMarchRow(row.data(), nullptr, n, 10.0f, 60.0f, rz, 0.5f);
    CHECK(gap.kind == SsrHit::Sky);
    // That sky may be what the wall hid (a soldier in front of a tent): the
    // ray spent many steps behind it, and a sky end after them is not trusted.
    CHECK(gap.hidden >= 4);
    CHECK(SsrHiddenTrust(gap.hidden) == 0.0f);
    // A thin pole in front costs a step at most, and keeps the sky.
    std::vector<float> pole(n, 1.0e6f);
    pole[30] = 3.0f;
    const SsrRowHit poleHit = SsrMarchRow(pole.data(), nullptr, n, 10.0f, 60.0f, rz, 0.5f);
    CHECK(poleHit.kind == SsrHit::Sky);
    CHECK(poleHit.hidden <= 1);
    CHECK(SsrHiddenTrust(poleHit.hidden) == 1.0f);

    // The same wall drawn as water does not stop the ray; as a road it does.
    row = wallRow(30.0f);
    std::vector<std::uint8_t> tags(n, 0);
    auto tagWall = [&](std::uint8_t tag)
    {
        for (int x = wallA; x <= wallB; ++x)
            tags[x] = tag;
    };
    tagWall(ssrtag::kWater);
    const SsrRowHit water = SsrMarchRow(row.data(), tags.data(), n, 10.0f, 60.0f, rz, 0.5f);
    CHECK(water.kind == SsrHit::Sky);
    CHECK_FALSE(water.cover);
    tagWall(ssrtag::kRoad);
    CHECK(SsrMarchRow(row.data(), tags.data(), n, 10.0f, 60.0f, rz, 0.5f).kind == SsrHit::Surface);

    // Drawn by a blended panel (the cover bit) it does not stop the ray
    // either, but the march keeps it: where the ray met it, and the first
    // uncovered texel after it (the sky next to the panel), whose colour tells
    // how much of what lies beyond the panel hides (SsrCoverOpacity).
    tagWall(ssrtag::kTerrain | ssrtag::kCover);
    for (float j : {0.0f, 0.5f, 0.99f})
    {
        CAPTURE(j);
        const SsrRowHit card = SsrMarchRow(row.data(), tags.data(), n, 10.0f, 60.0f, rz, j);
        CHECK(card.kind == SsrHit::Sky);
        REQUIRE(card.cover);
        CHECK(card.coverT == Catch::Approx(0.8f).margin(0.03f));
        CHECK(card.coverTravel == Catch::Approx((30.0f - 10.0f) / rz).margin(1.5f));
        REQUIRE(card.afterT > card.coverT);
        const int after = static_cast<int>(0.5f + 63.0f * card.afterT);
        CHECK(after > wallB);
        CHECK(after <= wallB + 2);
        // The sky next to a free-standing card: the colours decide.
        CHECK_FALSE(card.afterInFront);
    }
    // A window (texels 47 to 53) set in a solid body at the same depth (54 to
    // 60, uncovered): after the window the ray is behind that body, the first
    // uncovered texel stands in front of it, and the window is opaque (no sky
    // through a vehicle's rear window: the ray would meet its inside).
    std::vector<std::uint8_t> windowTags(n, 0);
    for (int x = 47; x <= 53; ++x)
        windowTags[x] = ssrtag::kTerrain | ssrtag::kCover;
    for (float j : {0.0f, 0.5f})
    {
        CAPTURE(j);
        const SsrRowHit window = SsrMarchRow(row.data(), windowTags.data(), n, 10.0f, 60.0f, rz, j);
        REQUIRE(window.cover);
        CHECK(window.afterT > window.coverT);
        CHECK(static_cast<int>(0.5f + 63.0f * window.afterT) >= 54);
        CHECK(window.afterInFront);
    }
    // A panel that fills the rest of the ray: nothing next to it to compare,
    // so the caller takes it as opaque (afterT -1).
    std::vector<float> panel(n, 1.0e6f);
    std::vector<std::uint8_t> panelTags(n, 0);
    for (int x = wallA; x < n; ++x)
    {
        panel[x] = 30.0f;
        panelTags[x] = ssrtag::kTerrain | ssrtag::kCover;
    }
    const SsrRowHit full = SsrMarchRow(panel.data(), panelTags.data(), n, 10.0f, 60.0f, rz, 0.5f);
    CHECK(full.kind != SsrHit::Surface);
    CHECK(full.cover);
    CHECK(full.afterT < 0.0f);

    // Right after the start the ray slips behind a surface close in front
    // (under a vehicle): no information there.
    std::vector<float> under(n, 1.0e6f);
    for (int x = 2; x <= 5; ++x)
        under[x] = 5.0f;
    const SsrRowHit under0 = SsrMarchRow(under.data(), nullptr, n, 10.0f, 60.0f, rz, 0.5f);
    CHECK(under0.kind == SsrHit::BehindNear);
    // Where it went behind: the first texel of that surface, close to the start;
    // its hidden side shows fully there and fades out by kSsrBehindNear.
    const int at = static_cast<int>(0.5f + 63.0f * under0.t);
    CHECK(at >= 2);
    CHECK(at <= 5);
    CHECK(under0.travel < 0.5f * kSsrBehindNear);
    CHECK(SsrUndersideFade(under0.travel) == 1.0f);
    CHECK(SsrUndersideFade(kSsrBehindNear) == 0.0f);
    CHECK(SsrUndersideFade(0.75f * kSsrBehindNear) == Catch::Approx(0.5f));

    // Ground all the way, behind the ray's depth by more than the thickness
    // and never in front of it: the ray leaves the row.
    std::vector<float> ground(n, 500.0f);
    CHECK(SsrMarchRow(ground.data(), nullptr, n, 10.0f, 60.0f, rz, 0.5f).kind == SsrHit::LeftFrame);
}

TEST_CASE("Reflection march: a grazing ray does not meet its own ground", "[remaster][ssr]")
{
    // A wet road seen at a grazing angle. The reflected ray climbs off the road
    // a few centimetres per pixel, and the half-resolution depth holds the
    // nearest sample of every other 2 x 2 block: there the road stands just in
    // front of the ray (within the march's thickness), elsewhere just behind.
    const int n = 64;
    const float z0 = 15.0f, z1 = 60.0f, rz = 0.95f;
    auto rayZ = [&](float t) { return 1.0f / (1.0f / z0 + (1.0f / z1 - 1.0f / z0) * t); };
    std::vector<float> road(n), onPlane(n, 0.01f);
    for (int x = 0; x < n; ++x)
        road[x] = rayZ(static_cast<float>(x) / 63.0f) + ((x & 1) ? -0.05f : 0.2f);
    road[n - 1] = 1.0e6f; // the sky at the row's end
    for (float j : {0.0f, 0.25f, 0.5f, 0.75f, 0.99f})
    {
        CAPTURE(j);
        // Without the plane test the ray stops on its own road (the dark dots
        // on the wet road, in the march start's interleaved pattern).
        CHECK(SsrMarchRow(road.data(), nullptr, n, z0, z1, rz, j).kind == SsrHit::Surface);
        // Those texels lie on the start's plane: the ray goes on to the sky.
        CHECK(SsrMarchRow(road.data(), nullptr, n, z0, z1, rz, j, onPlane.data()).kind == SsrHit::Sky);
    }
    CHECK(SsrSelfTolerance(z0) > 0.01f);
    CHECK(SsrSelfTolerance(z0) == Catch::Approx(kSsrSelfBase + kSsrSelfPerM * z0));
    CHECK(SsrSelfTolerance(100.0f) > SsrSelfTolerance(z0));

    // A vehicle standing on the road (texels 40 to 50, a metre and more above
    // the plane) still stops the ray, where the ray reaches its depth.
    const float tV = 45.0f / 63.0f;
    for (int x = 40; x <= 50; ++x)
    {
        road[x] = rayZ(tV);
        onPlane[x] = 1.2f;
    }
    const SsrRowHit vehicle = SsrMarchRow(road.data(), nullptr, n, z0, z1, rz, 0.5f, onPlane.data());
    REQUIRE(vehicle.kind == SsrHit::Surface);
    CHECK(vehicle.t == Catch::Approx(tV).margin(0.03f));
}

TEST_CASE("Reflection panels: a panel hides what lies beyond as much as it differs from what is next to it",
          "[remaster][ssr]")
{
    // A bush card's transparent part shows the ground behind it, about the
    // colour of the ground next to the card: the ray passes (no box under the
    // bush). A canvas against the sky: the canvas is what the reflection shows
    // (no pale box of sky in a vehicle's reflection).
    const float ground[3] = {0.25f, 0.30f, 0.15f};
    const float groundNextTo[3] = {0.27f, 0.31f, 0.16f};
    const float canvas[3] = {0.33f, 0.35f, 0.24f};
    const float sky[3] = {0.62f, 0.63f, 0.60f};
    CHECK(SsrCoverOpacity(ground, groundNextTo) == 0.0f);
    CHECK(SsrCoverOpacity(sky, sky) == 0.0f);
    CHECK(SsrCoverOpacity(canvas, sky) == 1.0f);
    CHECK(SsrCoverOpacity(sky, canvas) == 1.0f);
    // A hue change at the same brightness counts (the largest channel).
    const float red[3] = {0.5f, 0.3f, 0.3f};
    const float green[3] = {0.3f, 0.5f, 0.3f};
    CHECK(SsrCoverOpacity(red, green) == 1.0f);
    // From see-through to opaque between the two distances, never backwards.
    float prev = -1.0f;
    for (float d = 0.0f; d <= 0.2f; d += 0.005f)
    {
        const float a[3] = {0.4f + d, 0.4f, 0.4f};
        const float b[3] = {0.4f, 0.4f, 0.4f};
        const float o = SsrCoverOpacity(a, b);
        CHECK(o >= prev);
        prev = o;
        if (d <= kSsrCoverSeeFrom - 1e-4f)
            CHECK(o == 0.0f);
        if (d >= kSsrCoverSeeTo + 1e-4f)
            CHECK(o == 1.0f);
    }
}

TEST_CASE("Reflection streak: the taps never stand farther apart than the mip level they read", "[remaster][ssr]")
{
    const float maxLod = 6.0f;
    for (float pxV : {0.0f, 0.5f, 2.0f, 10.0f, 30.0f, 72.0f, 200.0f})
        for (float pxH : {0.05f, 0.3f, 1.0f, 4.0f, 20.0f, 100.0f})
        {
            CAPTURE(pxV, pxH);
            const SsrStreak s = SsrStreakTaps(pxV, pxH, maxLod);
            CHECK(s.taps >= 1);
            CHECK(s.taps <= kSsrStreakTaps);
            CHECK(s.stepPx * static_cast<float>(s.taps) == Catch::Approx(std::min(pxV, kSsrStreakMaxPx)).margin(1e-4));
            // No gaps between the taps (a ladder of copies), unless the
            // coarsest level is reached.
            if (s.lod < maxLod)
                CHECK(s.stepPx <= std::max(std::exp2(s.lod), 1.0f) + 1e-3f);
            // Never sharper than the horizontal spread asks.
            CHECK(s.lod >= std::clamp(std::log2(std::max(pxH, 1.0f)), 0.0f, maxLod) - 1e-5f);
        }
    // The ground's streak (pxV = 2 px, pxH = 0.7 px): 3 taps a side at the
    // horizontal spread's level, as before.
    for (float px : {2.0f, 5.0f, 10.0f, 30.0f})
    {
        CAPTURE(px);
        const SsrStreak s = SsrStreakTaps(2.0f * px, 0.7f * px, maxLod);
        CHECK(s.taps == 3);
        CHECK(s.stepPx == Catch::Approx(2.0f * px / 3.0f));
        CHECK(s.lod == Catch::Approx(std::log2(0.7f * px)));
    }
    // Water at a grazing angle (the horizontal spread a tenth of the vertical):
    // taps 2 pixels apart at level 1, not 7 pixels apart at level 1.
    const SsrStreak w = SsrStreakTaps(20.0f, 2.0f, maxLod);
    CHECK(w.taps == 10);
    CHECK(w.stepPx == Catch::Approx(2.0f));
    CHECK(w.lod == Catch::Approx(1.0f));
}

TEST_CASE("Reflection edges: a reflection fades out toward the frame's edges", "[remaster][ssr]")
{
    CHECK(SsrEdgeFade(0.5f, 0.5f) == 1.0f);
    CHECK(SsrEdgeFade(0.0f, 0.5f) == 0.0f);
    CHECK(SsrEdgeFade(0.5f, 1.0f) == 0.0f);
    CHECK(SsrEdgeFade(kSsrEdgeFade, 0.5f) == Catch::Approx(1.0f));
    CHECK(SsrEdgeFade(0.5f * kSsrEdgeFade, 0.5f) == Catch::Approx(0.5f));
    float prev = -1.0f;
    for (float u = 0.0f; u <= kSsrEdgeFade; u += 0.005f)
    {
        CHECK(SsrEdgeFade(u, 0.5f) >= prev);
        prev = SsrEdgeFade(u, 0.5f);
    }
}

TEST_CASE("Reflection blur: a rough reflection's outline becomes a ramp, a puddle's mirror stays sharp",
          "[remaster][ssr]")
{
    // A row of rough wet ground at one depth: the rays of texels 0 to 31 meet a
    // vehicle (dark), those of 32 to 63 the sky (bright). One ray per texel
    // gives a hard step: the box outline under a vehicle.
    const int n = 64;
    std::vector<float> value(n), z(n, 10.0f), spread(n, 8.0f), out(n);
    std::vector<std::uint8_t> surface(n, 1);
    for (int i = 0; i < n; ++i)
        value[i] = i < 32 ? 0.1f : 0.8f;
    SsrBlurRow(value.data(), surface.data(), z.data(), spread.data(), n, out.data());
    // A ramp as wide as the spread on each side of the step, never overshooting.
    for (int i = 1; i < n; ++i)
    {
        CAPTURE(i);
        CHECK(out[i] >= out[i - 1] - 1e-6f);
    }
    CHECK(out[31] > 0.1f + 0.2f);
    CHECK(out[32] < 0.8f - 0.2f);
    CHECK(out[31] == Catch::Approx(0.1f + 0.8f - out[32]).margin(1e-5f));
    CHECK(out[23] == Catch::Approx(0.1f)); // beyond the spread: untouched
    CHECK(out[40] == Catch::Approx(0.8f));

    // A narrow spread (the ray met what it reflects close to where it left
    // the ground, at a vehicle's wheels) keeps the step: contact stays sharp.
    // Below kSsrBlurMinSpread, and on untagged texels, nothing moves.
    std::fill(spread.begin(), spread.end(), 0.5f);
    SsrBlurRow(value.data(), surface.data(), z.data(), spread.data(), n, out.data());
    CHECK(out == value);
    std::fill(spread.begin(), spread.end(), 8.0f);
    std::vector<std::uint8_t> none(n, 0);
    SsrBlurRow(value.data(), none.data(), z.data(), spread.data(), n, out.data());
    CHECK(out == value);

    // A puddle (a sharp mirror, narrow spread) in rough ground: it keeps its
    // own value, and the rough texels beside it do not take its dark mirror.
    std::vector<float> road(n, 0.5f), roadSpread(n, 8.0f);
    for (int i = 28; i <= 35; ++i)
    {
        road[i] = 0.05f;
        roadSpread[i] = 0.5f;
    }
    SsrBlurRow(road.data(), surface.data(), z.data(), roadSpread.data(), n, out.data());
    for (int i = 28; i <= 35; ++i)
        CHECK(out[i] == 0.05f);
    CHECK(out[27] > 0.49f);
    CHECK(out[36] > 0.49f);
    CHECK(SsrBlurSpreadWeight(0.5f, 8.0f) < 0.01f);
    CHECK(SsrBlurSpreadWeight(8.0f, 8.0f) == 1.0f);
    CHECK(SsrBlurSpreadWeight(12.0f, 8.0f) == 1.0f);

    // A surface boundary (a road beside terrain) and a depth jump stop the
    // gather.
    std::vector<std::uint8_t> twoSurfaces(n, 3);
    for (int i = 32; i < n; ++i)
        twoSurfaces[i] = 1;
    SsrBlurRow(value.data(), twoSurfaces.data(), z.data(), spread.data(), n, out.data());
    for (int i = 0; i < n; ++i)
    {
        CAPTURE(i);
        CHECK(out[i] == Catch::Approx(value[i]));
    }
    std::vector<float> step(n, 10.0f);
    for (int i = 32; i < n; ++i)
        step[i] = 30.0f;
    SsrBlurRow(value.data(), surface.data(), step.data(), spread.data(), n, out.data());
    CHECK(out[31] == Catch::Approx(0.1f).margin(0.002f));
    CHECK(out[32] == Catch::Approx(0.8f).margin(0.002f));
    CHECK(SsrBlurDepthWeight(10.0f, 10.0f) == 1.0f);
    CHECK(SsrBlurDepthWeight(30.0f, 10.0f) < 0.001f);

    // The spread: half the horizontal width in full-resolution pixels, in
    // half-resolution texels, capped.
    CHECK(SsrBlurSpread(10.0f) == 5.0f);
    CHECK(SsrBlurSpread(1000.0f) == kSsrBlurMaxSpread);
    CHECK(SsrBlurSpread(-1.0f) == 0.0f);
    // A tap per texel of the spread, the same profile at any width.
    CHECK(SsrBlurTaps(0.75f) == 1);
    CHECK(SsrBlurTaps(8.0f) == 8);
    CHECK(SsrBlurTaps(8.2f) == 9);
    CHECK(SsrBlurTaps(kSsrBlurMaxSpread) == kSsrBlurMaxTaps);
    CHECK(SsrBlurTaps(1000.0f) == kSsrBlurMaxTaps);
    for (int taps : {1, 8, 24})
    {
        CAPTURE(taps);
        CHECK(SsrBlurTapWeight(0, taps) == 1.0f);
        CHECK(SsrBlurTapWeight(taps, taps) == Catch::Approx(std::exp(-2.0f)));
    }
}

TEST_CASE("Reflection blur: a texel-scale pattern in the trace averages out instead of being copied", "[remaster][ssr]")
{
    // Every third texel of a rough reflection dark (rays that met something by
    // chance), under the widest spread. Taps three texels apart (8 taps over 24
    // texels) all land on the same phase of the pattern: the dark texels stay
    // dark and the others stay bright, a dot lattice. A tap per texel takes
    // every phase.
    const int n = 72;
    std::vector<float> dots(n, 0.8f), z(n, 10.0f), spread(n, kSsrBlurMaxSpread), out(n);
    std::vector<std::uint8_t> surface(n, 1);
    for (int i = 0; i < n; i += 3)
        dots[i] = 0.1f;
    const float mean = (2.0f * 0.8f + 0.1f) / 3.0f;
    SsrBlurRow(dots.data(), surface.data(), z.data(), spread.data(), n, out.data());
    for (int i = 24; i < n - 24; ++i)
    {
        CAPTURE(i);
        CHECK(out[i] == Catch::Approx(mean).margin(0.03f));
    }
}

TEST_CASE("Reflection on water: the hit takes the sea's sky share, and a ray that sees sky changes nothing",
          "[remaster][ssr]")
{
    // The sea program's colour, one channel: body (1 - w) + S w.
    const float body = 0.06f, S = 0.5f, R = 0.08f;
    auto apply = [](float lin, const SsrWaterBlend& b, float r, float s)
    { return lin * (b.keep + b.swap * SsrWaterRatio(r, s)); };
    for (float w : {0.05f, 0.3f, 0.6f, 0.9f})
    {
        CAPTURE(w);
        const float lin = body * (1.0f - w) + S * w;
        // The sky's share of the pixel's light, as the pass estimates it.
        const float share = S * w / lin;
        // ssrWater 1: the reflection takes exactly the sky's share.
        SsrWaterBlend b = SsrWaterComposite(share, 1.0f, 1.0f);
        if (share <= kSsrWaterSwapMax)
            CHECK(apply(lin, b, R, S) == Catch::Approx(body * (1.0f - w) + R * w));
        // The same sky as the sea's: the frame of before.
        CHECK(apply(lin, b, S, S) == Catch::Approx(lin));
        // No fade, no change.
        b = SsrWaterComposite(share, 1.5f, 0.0f);
        CHECK(b.keep == 1.0f);
        CHECK(b.swap == 0.0f);
        // Half the fade, half the way.
        b = SsrWaterComposite(share, 1.0f, 0.5f);
        const SsrWaterBlend full = SsrWaterComposite(share, 1.0f, 1.0f);
        CHECK(apply(lin, b, R, S) == Catch::Approx(0.5f * (lin + apply(lin, full, R, S))));
    }
    // A facet that shows more sky shows more of the reflection: the sea's own
    // waves break the reflection up.
    const SsrWaterBlend b = SsrWaterComposite(0.7f, 1.0f, 1.0f);
    const float bright = 0.4f, dark = 0.2f; // two facets around the mean
    CHECK(bright - apply(bright, b, R, S) > dark - apply(dark, b, R, S));
    // Never negative, and a sky that lacks a channel caps the ratio.
    for (float s : {0.0f, 1e-6f, 0.01f, 0.5f})
        for (float r : {0.0f, 0.1f, 2.0f})
        {
            CAPTURE(s, r);
            CHECK(SsrWaterRatio(r, s) >= 0.0f);
            CHECK(SsrWaterRatio(r, s) <= kSsrWaterRatioMax);
            CHECK(apply(0.3f, b, r, s) >= 0.0f);
        }
    // ssrWater scales the swapped share, capped.
    CHECK(SsrWaterComposite(0.4f, 2.0f, 1.0f).swap == Catch::Approx(0.8f));
    CHECK(SsrWaterComposite(0.8f, 2.0f, 1.0f).swap == Catch::Approx(kSsrWaterSwapMax));
    CHECK(SsrWaterComposite(0.8f, 0.0f, 1.0f).swap == 0.0f);
}

TEST_CASE("Reflection switch: landed with its two sub-knobs and its post stages", "[remaster][ssr][FeatureSwitches]")
{
    const FeatureInfo& info = GetFeatureInfo(Feature::Ssr);
    CHECK(info.landed);
    CHECK(std::string_view(info.step) == "B6");
    CHECK(std::string_view(info.gpuSection) == "ssr");
    struct Sub
    {
        const char* key;
        float def;
    };
    for (const Sub& s : {Sub{"ssrWater", 1.5f}, Sub{"ssrWet", -1.0f}})
    {
        INFO(s.key);
        const LightingKey* k = FindLightingKey(s.key);
        REQUIRE(k != nullptr);
        CHECK(k->kind == KeyKind::Knob);
        REQUIRE(k->parent != nullptr);
        CHECK(std::string_view(k->parent) == "ssr");
        CHECK(std::string_view(k->group) == "ssr");
        CHECK(KeyDefault(*k) == s.def);
    }
    // The diagnostic views are in the overlay's list, after B2's albedo (7)
    // and B8's air (8) views (B4's combat mask follows them).
    const LightingKey* debug = FindLightingKey("debug");
    REQUIRE(debug != nullptr);
    CHECK(debug->maxValue >= static_cast<float>(kDebugViewSsrReflection));
    CHECK(kDebugViewSsrTags == 9);
    CHECK(kDebugViewSsrReflection == 10);
}

TEST_CASE("Reflection shaders: the march, the constants and the sea's sky are the engine's",
          "[Graphics][Shaders][remaster][ssr]")
{
    const std::string trace = gl::PostShaderSource(gl::PostShader::SsrTrace);
    const std::string composite = gl::PostShaderSource(gl::PostShader::SsrComposite);
    const std::string share = gl::PostShaderSource(gl::PostShader::SsrShare);
    REQUIRE_FALSE(trace.empty());
    REQUIRE_FALSE(composite.empty());
    CHECK(std::string_view(gl::PostShaderName(gl::PostShader::SsrTrace)) == "psPostSsrTrace");
    CHECK(std::string_view(gl::PostShaderName(gl::PostShader::SsrComposite)) == "psPostSsrComposite");
    CHECK(std::string_view(gl::PostShaderName(gl::PostShader::SsrTag)) == "psPostSsrTag");
    CHECK(std::string_view(gl::PostShaderName(gl::PostShader::SsrShare)) == "psPostSsrShare");

    // The constants SsrMarchRow and the composites use.
    auto num = [](float v)
    {
        std::ostringstream s;
        s << v;
        std::string t = s.str();
        if (t.find('.') == std::string::npos && t.find('e') == std::string::npos)
            t += ".0";
        return t;
    };
    const std::pair<std::string, std::string> constants[] = {
        {"const int kSsrSteps = ", std::to_string(kSsrSteps) + ";"},
        {"const float kSsrStride = ", num(kSsrStride) + ";"},
        {"const float kSsrStartSkip = ", num(kSsrStartSkip) + ";"},
        {"const int kSsrRefine = ", std::to_string(kSsrRefine) + ";"},
        {"const float kSsrThickBase = ", num(kSsrThickBase) + ";"},
        {"const float kSsrThickPerM = ", num(kSsrThickPerM) + ";"},
        {"const float kSsrBehindNear = ", num(kSsrBehindNear) + ";"},
        {"const float kSsrSkyDepth = ", "100000.0;"},
        {"const float kSsrEdgeFade = ", num(kSsrEdgeFade) + ";"},
        {"const float kSsrWaterSwapMax = ", num(kSsrWaterSwapMax) + ";"},
        {"const float kSsrWaterRatioMax = ", num(kSsrWaterRatioMax) + ";"},
        {"const float kSsrWaterRipple = ", num(kSsrWaterRipple) + ";"},
        {"const float kSsrWetRoadDark = ", num(kSsrWetRoadDark) + ";"},
        {"const float kSsrWetGroundDark = ", num(kSsrWetGroundDark) + ";"},
        {"const float kSsrWetGrassDark = ", num(kSsrWetGrassDark) + ";"},
        {"const float kSsrPuddleDark = ", num(kSsrPuddleDark) + ";"},
        {"const float kSsrPuddleRoad = ", num(kSsrPuddleRoad) + ";"},
        {"const float kSsrPuddleGround = ", num(kSsrPuddleGround) + ";"},
        {"const float kSsrPuddleFadeStart = ", num(kSsrPuddleFadeStart) + ";"},
        {"const float kSsrPuddleFadeEnd = ", num(kSsrPuddleFadeEnd) + ";"},
        {"const float kSsrUnderside = ", num(kSsrUnderside) + ";"},
        {"const float kSsrHiddenFrom = ", num(kSsrHiddenFrom) + ";"},
        {"const float kSsrHiddenTo = ", num(kSsrHiddenTo) + ";"},
        {"const float kSsrSelfBase = ", num(kSsrSelfBase) + ";"},
        {"const float kSsrSelfPerM = ", num(kSsrSelfPerM) + ";"},
        {"const float kSsrCoverSeeFrom = ", num(kSsrCoverSeeFrom) + ";"},
        {"const float kSsrCoverSeeTo = ", num(kSsrCoverSeeTo) + ";"},
        {"const float kSsrCoverLod = ", num(kSsrCoverLod) + ";"},
        {"const float kSsrStreakMaxPx = ", num(kSsrStreakMaxPx) + ";"},
        {"const int kSsrStreakTaps = ", std::to_string(kSsrStreakTaps) + ";"},
    };
    for (const auto& [decl, value] : constants)
    {
        CAPTURE(decl, value);
        CHECK(CountOf(trace, decl + value) == 1);
    }
    CHECK(kSsrSkyDepth == 100000.0f);
    CHECK(CountOf(share, "const int kSsrShareTaps = " + std::to_string(kSsrShareTaps) + ";") == 1);

    // The march's hit test and refine as SsrMarchRow does them: water never
    // stops it, nor its own surface (the plane test, SsrSelfTolerance), and a
    // covered panel is kept with the first uncovered texel after it.
    for (const char* line : {
             "float thick = kSsrThickBase + kSsrThickPerM * sz;",
             "bool cand = max(zPrev, z) >= sz && min(zPrev, z) <= sz + thick;",
             "if ((tg & 0xC0) == 0x80) cand = false;",
             "else if (plane.w > 0.5 && dot(plane.xyz, viewOfUv(uvOfHalf(s), sz) - p0) < kSsrSelfBase + kSsrSelfPerM "
             "* sz) cand = false;",
             "bool covered = (tg & 0x20) != 0;",
             "if (cover.w == 1.0 && !covered) { afterS = s; cover.w = sz < kSsrSkyDepth && min(zPrev, z) > sz + thick "
             "? 3.0 : 2.0; }",
             "float tk = ssrRefine(tPrev, t, s0, dS, k0, k1);",
             "float b = ssrRefine(tPrev, t, s0, dS, k0, k1);",
             "bool occluded = !cand && sz < kSsrSkyDepth && min(zPrev, z) > sz + thick;",
             "if (occluded && !behind && tr < kSsrBehindNear) {",
             "float trust = 1.0 - smoothstep(kSsrHiddenFrom, kSsrHiddenTo, hidden);",
             "float t = min(dt * (float(i) + jitter) + startT, tc);",
             "int n = int(clamp(span * tc / kSsrStride, 4.0, float(kSsrSteps)));",
             "float startT = kSsrStartSkip / max(span, 1e-3);",
             "if (1.0 / mix(k0, k1, m) >= sceneZ(s0 + dS * m)) b = m; else a = m;",
         })
    {
        CAPTURE(line);
        CHECK(CountOf(trace, line) == 1);
    }
    // A ray that slipped behind a surface sees that surface' hidden side, on
    // water and on the ground alike (the ground's own colour drew pale boxes).
    CHECK(CountOf(trace, "R = toLinear(textureLod(mipTex, uvOfHalf(hitS), 1.0).rgb) * kSsrUnderside;") == 2);
    CHECK(CountOf(trace, "R = toLinear(c0)") == 0);
    CHECK(CountOf(trace, "conf = 1.0 - smoothstep(0.5 * kSsrBehindNear, kSsrBehindNear, travel);") == 2);
    // The ground's ray tests its own plane (the depth normal it starts on);
    // water's needs none.
    CHECK(CountOf(trace, "vec3 nV = depthNormal(pick, p);") == 1);
    CHECK(CountOf(trace, "ssrMarch(p, normalize(w2v * rW), vec4(nV, 1.0), jitter,") == 1);
    CHECK(CountOf(trace, "ssrMarch(p, normalize(w2v * rM), vec4(0.0), jitter,") == 1);
    // A covered panel lies over what the ray found beyond as much as it hides
    // (SsrCoverOpacity), on the ground and on water.
    for (const char* line : {
             "if (cover.w < 0.5 || (kind == 3 && cover.z > travel)) return 0.0;",
             "if (cover.w < 1.5 || cover.w > 2.5) return 1.0;",
             "return smoothstep(kSsrCoverSeeFrom, kSsrCoverSeeTo, max(d.r, max(d.g, d.b)));",
             "Rmix = mix(Rmix, streakSample(uvK, pxK * 2.0, pxK * 0.7), wK);",
             "use = mix(use, 1.0, wK);",
             "R = (wK * RK + (1.0 - wK) * conf * R) / max(c, 1e-4);",
         })
    {
        CAPTURE(line);
        CHECK(CountOf(trace, line) == 1);
    }
    CHECK(CountOf(trace, "float a = coverOpacity(cover, afterS, kind, travel);") == 2);
    // The streak's taps as SsrStreakTaps lays them out.
    for (const char* line : {
             "float spanPx = min(pxV, kSsrStreakMaxPx);",
             "float n = clamp(ceil(spanPx / max(pxH, 1.0)), 1.0, float(kSsrStreakTaps));",
             "float stepPx = spanPx / n;",
             "float lod = clamp(log2(max(max(pxH, stepPx), 1.0)), 0.0, maxLod);",
         })
    {
        CAPTURE(line);
        CHECK(CountOf(trace, line) == 1);
    }
    // The tag bits as the code table lays them out.
    CHECK(ssrtag::kWater == 0x80);
    CHECK(ssrtag::kCover == 0x20);
    CHECK(ssrtag::kSurfaceBits == 0xC0);

    // The sea's sky share: the shared block and sky piece pasted once, the
    // sea's own sky reflection with its tint and its far fade (the water
    // program's constants), and the composite of SsrWaterComposite.
    CHECK(CountOf(trace, gl::LitConstantsBlock()) == 1);
    CHECK(CountOf(trace, gl::RemasterFragmentPiece(gl::FragmentPiece::Sky)) == 1);
    CHECK(trace.find("mix(skyReflection(rS), fogColor.rgb, litWater0.w)") != std::string::npos);
    CHECK(trace.find("float skyMean = litWater0.z * mix(kSeaFarSkyFloor, 1.0, 1.0 - smoothstep(kSeaFarSkyStart, "
                     "kSeaFarSkyEnd, dist));") != std::string::npos);
    const std::string water = gl::RemasterFragmentMain(3); // PSWater
    for (const char* c : {"const float kSeaFarSkyStart = 1000.0;", "const float kSeaFarSkyEnd = 3000.0;",
                          "const float kSeaFarSkyFloor = 0.5;"})
    {
        CAPTURE(c);
        CHECK(water.find(c) != std::string::npos);
        CHECK(trace.find(c) != std::string::npos);
    }
    CHECK(water.find("mix(1.0, 0.82, smoothstep(0.15, 0.9, r.y))") != std::string::npos);
    CHECK(trace.find("mix(1.0, 0.82, smoothstep(0.15, 0.9, rS.y))") != std::string::npos);
    CHECK(trace.find("float x = min(share * max(ssrParams.x, 0.0), kSsrWaterSwapMax);") != std::string::npos);
    CHECK(trace.find("vec3 ratio = clamp(R / max(S, vec3(1e-4)), 0.0, kSsrWaterRatioMax);") != std::string::npos);
    CHECK(trace.find("add = vec3(1.0 - fade * x) + fade * x * ratio;") != std::string::npos);
    CHECK(composite.find("vec3 lin = surface == 2 ? toLinearC(c.rgb) * add : toLinearC(c.rgb) * m + add;") !=
          std::string::npos);
    // Only a surface hit (or the dark under a hull) changes the sea; sky and
    // frame exits leave the sea program's own sky.
    CHECK(trace.find("if (kind == 1) {") != std::string::npos);
    // The ripples at the calm share of the research's slopes.
    CHECK(CountOf(trace, "float s = S[i] * kSsrWaterRipple;") == 1);
    // A ray that leaves the frame takes the frame's own open sky where there is
    // some above where it points (a hill in the way), as a ray that ends on
    // open sky inside the frame does: no pale patch where the two kinds meet.
    CHECK(CountOf(trace, "vec2 uk = vec2(uc.x, mix(uc.y, top, float(k) / float(kSsrSkySearch)));") == 1);
    CHECK(CountOf(trace, "if (sceneZ(halfOfUv(uk)) >= kSsrSkyDepth) {") == 1);

    // The colour helpers are the prelude's, word for word.
    const std::string inputs = gl::RemasterFragmentPiece(gl::FragmentPiece::Inputs);
    for (const char* sig : {"vec3 tonemapNeutral(vec3 c) {"})
    {
        CAPTURE(sig);
        const std::string ours = FunctionText(trace, sig);
        REQUIRE_FALSE(ours.empty());
        CHECK(ours == FunctionText(inputs, sig));
    }
    for (const char* line : {"vec3 toLinear(vec3 c) { return pow(max(c, vec3(0.0)), vec3(2.2)); }",
                             "vec3 toDisplay(vec3 c) { return pow(max(c, vec3(0.0)), vec3(1.0 / 2.2)); }",
                             "const vec3 kLuma = vec3(0.2126, 0.7152, 0.0722);"})
    {
        CAPTURE(line);
        CHECK(CountOf(inputs, line) == 1);
        CHECK(CountOf(trace, line) == 1);
    }

    // The composite keeps a texel only for the pixel's own surface, weighs it
    // by depth, and keeps the frame's alpha (the sky light share AO reads).
    CHECK(composite.find("if (ts != surface) continue;") != std::string::npos);
    CHECK(composite.find("w /= 1.0 + dz * dz;") != std::string::npos);
    CHECK(composite.find("fragColor = vec4(toDisplayC(max(lin, vec3(0.0))), c.a);") != std::string::npos);
    CHECK(composite.find("int surface = (tg & 0x20) != 0 ? 0 : (tg >> 6) & 3;") != std::string::npos);

    // The rough blur: the trace writes each texel's spread as SsrBlurSpread
    // does (on water and on the ground), and the gather is SsrBlurRow's, with
    // the composite's depth weight.
    const std::string blur = gl::PostShaderSource(gl::PostShader::SsrBlur);
    REQUIRE_FALSE(blur.empty());
    CHECK(std::string_view(gl::PostShaderName(gl::PostShader::SsrBlur)) == "psPostSsrBlur");
    CHECK(CountOf(trace, "layout(location = 1) out float fragSpread;") == 1);
    CHECK(CountOf(trace, "const float kSsrBlurMaxSpread = " + num(kSsrBlurMaxSpread) + ";") == 1);
    CHECK(CountOf(trace, "float ssrBlurSpread(float pxH) { return clamp(0.5 * pxH, 0.0, kSsrBlurMaxSpread); }") == 1);
    CHECK(CountOf(trace, "fragSpread = ssrBlurSpread(pxH);") == 2);
    for (const std::string& c : {"const int kSsrBlurMaxTaps = " + std::to_string(kSsrBlurMaxTaps) + ";",
                                 "const float kSsrBlurMaxSpread = " + num(kSsrBlurMaxSpread) + ";",
                                 "const float kSsrBlurMinSpread = " + num(kSsrBlurMinSpread) + ";"})
    {
        CAPTURE(c);
        CHECK(CountOf(blur, c) == 1);
    }
    for (const char* line : {
             "if (s == 0 || r < kSsrBlurMinSpread) {",
             "float n = clamp(ceil(r), 1.0, float(kSsrBlurMaxTaps));",
             "if (float(k) > n) break;",
             "float o = r * float(k) / n;",
             "float x = float(k) / n;",
             "float wk = exp(-2.0 * x * x);",
             "ivec2 q = ivec2(clamp(ij.x + int(floor(float(side) * o + 0.5)), 0, last), ij.y);",
             "if (int(floor(t.a * 0.25)) != s) continue;",
             "float ts = clamp(texelFetch(spreadTex, q, 0).r / r, 0.0, 1.0);",
             "float w = wk * blurDepthWeight(texelFetch(linDepthTex, q, 0).r, z) * ts * ts;",
             "float dz = (tz - z) / (0.03 * z + 0.1);",
         })
    {
        CAPTURE(line);
        CHECK(CountOf(blur, line) == 1);
    }
    CHECK(CountOf(composite, "float dz = (tz - z) / (0.03 * z + 0.1);") == 1);
}

TEST_CASE("Reflection backend: the tags ride on the depth state, off is the state of before",
          "[Graphics][GL33][remaster][ssr]")
{
    const std::string queue = ReadTextFile(Gl33Dir() / "EngineGL33_Queue.cpp");
    const std::string state = ReadTextFile(Gl33Dir() / "EngineGL33_State.cpp");
    const std::string post = ReadTextFile(Gl33Dir() / "EngineGL33_PostFX.cpp");
    const std::string ssr = ReadTextFile(Gl33Dir() / "EngineGL33_PostSsr.cpp");
    const std::string vb = ReadTextFile(Gl33Dir() / "EngineGL33_VertexBuffer.cpp");
    const std::string life = ReadTextFile(Gl33Dir() / "EngineGL33_Lifecycle.cpp");
    REQUIRE_FALSE(ssr.empty());

    // Every draw's tag comes from the descriptor, tagging only while the world
    // writes tags, and a tag change without a descriptor change re-applies the
    // depth state.
    const size_t apply = queue.find("void EngineGL33::ApplyPassState(");
    REQUIRE(apply != std::string::npos);
    const std::string body = queue.substr(apply, queue.find("\n}\n", apply) - apply);
    const size_t tag = body.find("_ssrStencil = Poseidon::remaster::SsrStencilForDraw(");
    const size_t pipe = body.find("ApplyPipeline(d);");
    REQUIRE(tag != std::string::npos);
    CHECK(tag < pipe);
    CHECK(body.find("SsrTagging());") != std::string::npos);
    CHECK(body.find("if (!_ssrStencilValid || _ssrStencilApplied != _ssrStencil)", pipe) != std::string::npos);
    // ApplyDepthMode hands the tag to every depth-stencil bundle.
    for (const char* call : {"ds::Normal(_hasStencilBuffer, tag);", "ds::ReadOnly(_hasStencilBuffer, tag);",
                             "ds::Disabled(_hasStencilBuffer, tag);", "ds::Shadow(_hasStencilBuffer, tag);"})
    {
        CAPTURE(call);
        CHECK(CountOf(state, call) == 1);
    }
    // Tags are written only in the world's remastered passes with the switch on.
    const size_t tagging = ssr.find("bool EngineGL33::SsrTagging() const");
    REQUIRE(tagging != std::string::npos);
    const std::string cond = ssr.substr(tagging, ssr.find("\n}\n", tagging) - tagging);
    for (const char* c : {"_remasterPrograms", "_worldPassOpen", "!_portraitTargetSize",
                          "rm::FeatureOn(_lighting.features, rm::Feature::Ssr)"})
    {
        CAPTURE(c);
        CHECK(cond.find(c) != std::string::npos);
    }
    // The world's end takes the tags away before the resolve and the HUD, and
    // every stencil clear writes every bit first.
    CHECK(post.find("_ssrStencil = {};") < post.find("if (!worldDrawn || !_glContext"));
    CHECK(vb.find("Poseidon::render::depthstencil::StencilFullWriteMask();") <
          vb.find("Poseidon::render::clear::WithMask(mask);"));
    CHECK(life.find("Poseidon::render::depthstencil::StencilFullWriteMask();") <
          life.find("Poseidon::render::clear::WithMask(clearMask);"));
    // The readout tests the bits without writing, then puts the full mask back.
    CHECK(CountOf(ssr, "Poseidon::render::depthstencil::StencilTestEqual(bit, bit);") == 1);
    CHECK(CountOf(ssr, "Poseidon::render::depthstencil::StencilOffFullMask();") == 1);
    // The reflections' output becomes the frame the glow stages and the
    // composite read; the resolved world stays as drawn (TAA's particle mask).
    CHECK(CountOf(ssr, "f.frameColor = s.outTex;") == 1);
    // The trace writes two targets (the result and its spread); the blur runs
    // between the trace and the composite, which reads the blurred trace.
    CHECK(CountOf(ssr, "glDrawBuffers(2, traceTargets);") == 1);
    const size_t traceDraw = ssr.find("DrawTo(s.traceFbo, f.hw, f.hh);");
    const size_t blurDraw = ssr.find("DrawTo(s.blurFbo, f.hw, f.hh);");
    const size_t compositeDraw = ssr.find("DrawTo(s.outFbo, f.w, f.h);");
    REQUIRE(traceDraw != std::string::npos);
    REQUIRE(blurDraw != std::string::npos);
    REQUIRE(compositeDraw != std::string::npos);
    CHECK(traceDraw < blurDraw);
    CHECK(blurDraw < compositeDraw);
    CHECK(CountOf(ssr.substr(blurDraw, compositeDraw - blurDraw), "BindTex(3, s.blurTex);") == 1);
    CHECK(CountOf(ssr, "BindTex(3, s.traceTex);") == 0);
    CHECK(CountOf(post, "f.frameColor = _post.colorTex;") == 1);
    // The air pass (B8) applies over that frame, the bloom and the composite
    // read the air pass's output when it ran and that frame otherwise, and the
    // shafts read that frame.
    CHECK(CountOf(post, "? p.airOutTex : f.frameColor") == 2);
    CHECK(CountOf(post, "BindTex(2, f.frameColor);") == 1);
    CHECK(CountOf(ReadTextFile(Gl33Dir() / "EngineGL33_PostAir.cpp"), "BindTex(0, f.frameColor);") == 1);
    CHECK(CountOf(post, "in.ssrTags = _ssrTagFrame == GetFrameCounter()") == 1);
    // The trace redoes the world's distance fog (a fully fogged pixel is the
    // fog colour and stays it) with the world's own fog, copied when BeginPass
    // builds the world pass's frame state, never with the frame state's fog
    // flag: SetShaderFogEnabled rewrites that per draw, and the last draw
    // (a shadow, a light, an alpha-fog or an unfogged draw) may have left it 0.
    CHECK(CountOf(ssr, "_frameState.fogParams") == 0);
    CHECK(CountOf(ssr, "Set4(tr, \"fogParam\", _ssrWorldFog[0], _ssrWorldFog[1], 0.0f, 0.0f);") == 1);
    const size_t build = queue.find("_frameState = BuildFrameState(");
    const size_t effects = queue.find("_worldEffectsOn = !_portraitTargetSize");
    const size_t copy0 = queue.find("_ssrWorldFog[0] = _frameState.fogParams[0];");
    const size_t copy1 = queue.find("_ssrWorldFog[1] = _frameState.fogParams[1];");
    REQUIRE(build != std::string::npos);
    REQUIRE(effects != std::string::npos);
    REQUIRE(copy0 != std::string::npos);
    REQUIRE(copy1 != std::string::npos);
    CHECK(build < effects);
    CHECK(effects < copy0);
    CHECK(queue.find("UpdateRemasterPrograms();", build) > copy1);
    const std::string trace = gl::PostShaderSource(gl::PostShader::SsrTrace);
    const std::string fogT = "float fogT = clamp(1.0 - (dist - fogParam.x) * fogParam.y, 0.0, 1.0);";
    CHECK(CountOf(trace, fogT) == 1);
    CHECK(CountOf(trace, "fogParam.z") == 0);
    // The world vertex shader's fog factor, the same formula.
    const std::string litShaders = ReadTextFile(Gl33Dir() / "EngineGL33_LightingShaders.cpp");
    CHECK(litShaders.find("float fogFactor = clamp(1.0 - (dist - fogParam.x) * fogParam.y, 0.0, 1.0);") !=
          std::string::npos);

    // The shares reach the gate only while fresh.
    const std::string lighting = ReadTextFile(Gl33Dir() / "EngineGL33_Lighting.cpp");
    CHECK(lighting.find("GetFrameCounter() - _ssr.shareFrame <= static_cast<std::uint32_t>(rm::kSsrShareMaxAge)") !=
          std::string::npos);
    CHECK(lighting.find("in.wetShare = _ssr.groundShare;") != std::string::npos);
}
