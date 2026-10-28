// Regression: mission description.ext entries like `sound[] = {"sound/s01r01.ogg", ...}`
// resolved to `sound\sound/s01r01.ogg` (doubled prefix) and failed in FileCache.
// Without the fix, GetSoundName/GetPictureName below produce "sound\sound/..." /
// "data\pic.paa" -> "data\data\pic.paa". Tolerances are exact-string equality.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <initializer_list>
#include <string.h>
#include <Poseidon/Foundation/Strings/RString.hpp>

TEST_CASE("GetDefaultName preserves an existing dir prefix", "[io][paramFileExt]")
{
    SECTION("bare name gets the default dir prepended")
    {
        RString r = GetDefaultName("s01r01", "sound\\", ".wss");
        REQUIRE(strcmp((const char*)r, "sound\\s01r01.wss") == 0);
    }

    SECTION("name with matching forward-slash dir prefix is left alone")
    {
        RString r = GetDefaultName("sound/s01r01.ogg", "sound\\", ".wss");
        REQUIRE(strcmp((const char*)r, "sound/s01r01.ogg") == 0);
    }

    SECTION("name with matching backslash dir prefix is left alone")
    {
        RString r = GetDefaultName("sound\\s01r01.ogg", "sound\\", ".wss");
        REQUIRE(strcmp((const char*)r, "sound\\s01r01.ogg") == 0);
    }

    SECTION("matching prefix is detected case-insensitively")
    {
        RString r = GetDefaultName("SOUND/s01r01.ogg", "sound\\", ".wss");
        REQUIRE(strcmp((const char*)r, "sound/s01r01.ogg") == 0);
    }

    SECTION("leading slash strips and skips the dir prefix")
    {
        RString r = GetDefaultName("\\anim\\walk.rtm", "anim\\", ".rtm");
        REQUIRE(strcmp((const char*)r, "anim\\walk.rtm") == 0);
    }

    SECTION("missing extension is appended")
    {
        RString r = GetDefaultName("walk", "anim\\", ".rtm");
        REQUIRE(strcmp((const char*)r, "anim\\walk.rtm") == 0);
    }

    SECTION("works for the data\\ picture dir too")
    {
        RString r = GetDefaultName("data/pic.paa", "data\\", ".paa");
        REQUIRE(strcmp((const char*)r, "data/pic.paa") == 0);
    }
}

TEST_CASE("GetSoundName accepts mission-relative sound/ paths", "[io][paramFileExt]")
{
    // Real-world case: Missions/01TakeTheCar.ABEL/description.ext writes
    //   sound[] = {"sound/s01r01.ogg", db-10, 1.0};
    // Pre-fix this resolved to "sound\\sound/s01r01.ogg".
    RString r = GetSoundName("sound/s01r01.ogg");
    REQUIRE(strcmp((const char*)r, "sound/s01r01.ogg") == 0);
}

TEST_CASE("SelectMenuInitWorld falls back to demo world when configured init terrain is absent", "[io][paramFileExt]")
{
    SECTION("full package keeps initWorld when its terrain exists")
    {
        RString r = SelectMenuInitWorld("Intro", "Demo", false, true, true);
        REQUIRE(strcmp((const char*)r, "Intro") == 0);
    }

    SECTION("full binary on stripped demo package falls back to demoWorld")
    {
        RString r = SelectMenuInitWorld("Intro", "Demo", false, false, true);
        REQUIRE(strcmp((const char*)r, "Demo") == 0);
    }

    SECTION("demo binary still explicitly prefers demoWorld")
    {
        RString r = SelectMenuInitWorld("Intro", "Demo", true, true, true);
        REQUIRE(strcmp((const char*)r, "Demo") == 0);
    }

    SECTION("missing demoWorld keeps the configured initWorld")
    {
        RString r = SelectMenuInitWorld("Intro", "", false, false, false);
        REQUIRE(strcmp((const char*)r, "Intro") == 0);
    }
}

TEST_CASE("GetColor reads three-entry colours with alpha 1", "[io][paramFileExt][remaster]")
{
    // CfgNonAIVehicles >> StreetLamp: colorDiffuse[]={0.9,0.8,0.6}; colorAmbient[]={0.1,0.1,0.1};
    // Read as white, every street lamp lit at ten times its authored ambient
    // (a grey wash out to 500 m around a town at night) and lost its warm colour.
    ParamFile pf;
    auto array = [&](const char* name, std::initializer_list<float> values) -> const ParamEntry&
    {
        ParamEntry* entry = pf.AddArray(name);
        for (float v : values)
            entry->AddValue(v);
        return *entry;
    };

    const Color diffuse = GetColor(array("colorDiffuse", {0.9f, 0.8f, 0.6f}));
    CHECK(diffuse.R() == Catch::Approx(0.9f));
    CHECK(diffuse.G() == Catch::Approx(0.8f));
    CHECK(diffuse.B() == Catch::Approx(0.6f));
    CHECK(diffuse.A() == Catch::Approx(1.0f));

    const Color ambient = GetColor(array("colorAmbient", {0.1f, 0.1f, 0.1f}));
    CHECK(ambient.R() == Catch::Approx(0.1f));
    CHECK(ambient.B() == Catch::Approx(0.1f));

    const Color rgba = GetColor(array("color", {0.9f, 0.8f, 0.8f, 0.5f}));
    CHECK(rgba.G() == Catch::Approx(0.8f));
    CHECK(rgba.A() == Catch::Approx(0.5f));

    // Anything else still reads as white.
    const Color bad = GetColor(array("bad", {0.2f, 0.3f}));
    CHECK(bad.R() == Catch::Approx(1.0f));
    CHECK(bad.B() == Catch::Approx(1.0f));
}
