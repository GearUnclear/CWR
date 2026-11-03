#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <Poseidon/Graphics/Rendering/Impostor.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <cstdint>
#include <string>

// XP-impostor (render/Impostor.hpp, EngineGL33_Impostor.cpp): the frame
// choice the vertex shader mirrors, the coverage-keeping mip chain, and the
// prelude declarations the impostor program rewrites.

using namespace Poseidon::render::impostor;
using Catch::Matchers::WithinAbs;

namespace
{
Image Solid(int w, int h, std::uint8_t a)
{
    Image img;
    img.w = w;
    img.h = h;
    img.rgba.assign(static_cast<size_t>(w) * h * 4, 0);
    for (size_t i = 0; i < img.rgba.size(); i += 4)
    {
        img.rgba[i] = 40, img.rgba[i + 1] = 120, img.rgba[i + 2] = 30, img.rgba[i + 3] = a;
    }
    return img;
}

int Count(const std::string& s, const std::string& what)
{
    int n = 0;
    for (size_t at = s.find(what); at != std::string::npos; at = s.find(what, at + 1))
        ++n;
    return n;
}
} // namespace

TEST_CASE("impostor frames: azimuth from +z toward +x, blend toward the next frame", "[impostor]")
{
    FramePick p = PickFrames(0.0f, 1.0f, 8);
    CHECK(p.f0 == 0);
    CHECK(p.f1 == 1);
    CHECK_THAT(p.blend, WithinAbs(0.0, 1e-5));

    p = PickFrames(1.0f, 0.0f, 8); // 90 degrees
    CHECK(p.f0 == 2);
    CHECK_THAT(p.blend, WithinAbs(0.0, 1e-4));

    p = PickFrames(-1.0f, 0.0f, 8); // -90 degrees = 270
    CHECK(p.f0 == 6);
    CHECK(p.f1 == 7);

    p = PickFrames(0.38268343f, 0.92387953f, 8); // 22.5 degrees: half way from 0 to 1
    CHECK(p.f0 == 0);
    CHECK_THAT(p.blend, WithinAbs(0.5, 1e-4));

    p = PickFrames(-0.38268343f, 0.92387953f, 8); // -22.5 degrees: half way from 7 to 0
    CHECK(p.f0 == 7);
    CHECK(p.f1 == 0);
    CHECK_THAT(p.blend, WithinAbs(0.5, 1e-4));

    p = PickFrames(0.0f, 0.0f, 8); // no direction: frame 0
    CHECK(p.f0 == 0);
    CHECK(p.blend == 0.0f);
}

TEST_CASE("impostor mips: alpha-weighted colour, empty texels take the fill", "[impostor]")
{
    Image a = Solid(2, 2, 0);
    Image n = Solid(2, 2, 0);
    // One opaque texel of four.
    a.rgba[3] = 255;
    a.rgba[0] = 200;
    const std::uint8_t fill[4] = {1, 2, 3, 0};
    const std::uint8_t fillN[4] = {128, 128, 128, 128};
    Image oa, on;
    Downsample(a, n, oa, on, fill, fillN);
    REQUIRE(oa.w == 1);
    REQUIRE(oa.h == 1);
    CHECK(oa.rgba[0] == 200); // only the opaque texel's colour
    CHECK(oa.rgba[3] == 64);  // alpha averaged
    Image empty = Solid(2, 2, 0);
    Downsample(empty, n, oa, on, fill, fillN);
    CHECK(oa.rgba[0] == 1);
    CHECK(oa.rgba[3] == 0);
    CHECK(on.rgba[0] == 128);
}

TEST_CASE("impostor mips: the chain keeps level 0's leaf coverage", "[impostor]")
{
    // A sparse crown: one texel in three opaque, in a pattern a box filter
    // turns into alpha below the cut everywhere.
    Image a = Solid(kTile, kTile, 0);
    Image n = Solid(kTile, kTile, 128);
    for (int y = 0; y < kTile; ++y)
        for (int x = 0; x < kTile; ++x)
            if ((x + 2 * y) % 3 == 0)
                a.rgba[(static_cast<size_t>(y) * kTile + x) * 4 + 3] = 255;
    const float c0 = Coverage(a, kAlphaRef);
    const std::uint8_t fill[4] = {40, 120, 30, 0};
    const std::uint8_t fillN[4] = {128, 128, 128, 128};
    std::vector<Image> ca, cn;
    BuildChain(a, n, fill, fillN, ca, cn);
    REQUIRE(ca.size() == 8); // 128 .. 1
    CHECK(ca.back().w == 1);
    CHECK(ca.back().h == 1);
    for (size_t l = 1; l + 2 < ca.size(); ++l)
        CHECK_THAT(Coverage(ca[l], kAlphaRef), WithinAbs(c0, 0.08));
    // Without the scale a 2x2 box of this pattern never reaches the cut.
    Image plainA, plainN;
    Downsample(a, n, plainA, plainN, fill, fillN);
    Image plain2A, plain2N;
    Downsample(plainA, plainN, plain2A, plain2N, fill, fillN);
    CHECK(Coverage(plain2A, kAlphaRef) < c0 - 0.1f);
}

TEST_CASE("impostor program: the prelude declarations it rewrites exist once each", "[impostor]")
{
    const std::string common = Poseidon::gl33lighting::RemasterFragmentCommon();
    CHECK(Count(common, "in vec3 vWorldRel;") == 1);
    CHECK(Count(common, "in vec3 vNormalW;") == 1);
    CHECK(Count(common, "in vec4 vCrown;") == 1);
    CHECK(Count(common, "vec4 alphaRef;") == 1);
    CHECK(Count(common, "uniform sampler2D tex0;") == 1);
    CHECK(Count(common, "vec3 shadeRemastered(vec3 albedo, bool terrain, out float ambientShare)") == 1);
    CHECK(Count(common, "void finishFragment(vec4 r0, float ambientShare, bool nightEye)") == 1);
}

TEST_CASE("impostor settings: off unless asked", "[impostor]")
{
    // The test runner sets no POSEIDON_XP_IMPOSTOR.
    const Settings& s = Get();
    CHECK(s.distance >= 50.0f);
    CHECK(sizeof(Instance) == 64);
}
