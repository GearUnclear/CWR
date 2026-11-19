// The fire plume's puffs never step the frame (fix 02 follow-up): a puff
// fades out by its depth along the view before Object::DrawDecal drops it,
// and the plume's shadow on the sun is eased, not rebuilt in jumps.
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
std::string ReadSource(const std::filesystem::path& p)
{
    std::ifstream f(p);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::filesystem::path EngineDir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "engine" / "Poseidon";
}
} // namespace

TEST_CASE("fire plume puffs fade out before a decal is dropped at the eye", "[firespread]")
{
    const std::string object = ReadSource(EngineDir() / "World" / "Scene" / "Object.cpp");
    const std::string smokes = ReadSource(EngineDir() / "Graphics" / "Rendering" / "Effects" / "Smokes.cpp");
    REQUIRE_FALSE(object.empty());
    REQUIRE_FALSE(smokes.empty());
    // Object::DrawDecal drops a decal whose middle is within ten near planes
    // of the camera; the puff's fade is measured from that same plane, by its
    // depth along the view (a big puff passing beside the eye filled the
    // screen at a large distance and then went in one frame).
    CHECK(object.find("float nearest = camera.Near() * 10;") != std::string::npos);
    CHECK(smokes.find("constexpr float kPuffDecalCutNear = 10.0f;") != std::string::npos);
    CHECK(smokes.find("const float cut = kPuffDecalCutNear * cam.Near();") != std::string::npos);
    CHECK(smokes.find("a *= PlumeSmooth(cut + kPuffEyeNear * size, cut + kPuffEyeFar * size, depth);") !=
          std::string::npos);
    // not by its distance any more
    CHECK(smokes.find("GetCamera()->Position().Distance(Vector3(px, y, pz))") == std::string::npos);
}

TEST_CASE("the fire plume's shadow is eased, a blob per stretch, from the puffs' own alpha", "[firespread]")
{
    const std::string world = ReadSource(EngineDir() / "World" / "Simulation" / "FireSpreadWorld.cpp");
    REQUIRE_FALSE(world.empty());
    CHECK(world.find("kPlumeShadowEaseSec") != std::string::npos);
    CHECK(world.find("PlumeShadowBlob& o = _plumeShadowTarget.blobs[b];") != std::string::npos);
    // the shadow takes the puffs' alpha before the fade round the eye, so it
    // never moves with the camera
    CHECK(world.find("p->BodyOpacity()") != std::string::npos);
    CHECK(world.find("p->Opacity() < 0.01f") == std::string::npos);
}
