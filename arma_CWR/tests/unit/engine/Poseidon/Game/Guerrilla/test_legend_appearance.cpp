#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Game/Guerrilla/LegendAppearance.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QStream.hpp>
#include <Poseidon/Game/Guerrilla/PortraitService.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <set>
#include <cstring>
#include <string>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

TEST_CASE("portrait and live binding preserve valid saved faces outside the roll pool", "[guerrilla][portrait]")
{
    const char* text = "class CfgFaces { class Face10 {}; class SavedFace {}; };";
    ParamFile config;
    QIStream stream(text, std::strlen(text));
    config.Parse(stream);
    const auto* faces = config.FindEntry("CfgFaces");
    CHECK(std::string(ResolveLegendFace(faces, RString("SavedFace"), false)) == "SavedFace");
    CHECK(std::string(ResolveLegendFace(faces, RString("Missing"), false)) == "Face10");
    CHECK(std::string(ResolveLegendFace(nullptr, RString("SavedFace"), false)) == "SavedFace");
}

TEST_CASE("portrait face fallback respects disabled presence and body gender", "[guerrilla][portrait]")
{
    const char* text = "class CfgFaces { class Face10 {disabled=0;}; class Face18 {woman=1;}; class Face27 {}; };";
    ParamFile config;
    QIStream stream(text, std::strlen(text));
    config.Parse(stream);
    const auto* faces = config.FindEntry("CfgFaces");
    REQUIRE(faces);
    CHECK_FALSE(LegendFaceUsable(faces, "Face10", false));
    CHECK(std::string(ResolveLegendFace(faces, RString("Face10"), false)) == "Face27");
    CHECK(std::string(ResolveLegendFace(faces, RString("Face27"), true)) == "Face18");
    CHECK(std::string(ResolveLegendFace(faces, RString(), true)) == "Face18");
    ParamFile empty;
    CHECK(std::string(ResolveLegendFace(&empty, RString("Face27"), false)) == "Default");
}

TEST_CASE("portrait roster contains selected companions, civilian outfit, bosses and saved appearances",
          "[guerrilla][portrait]")
{
    FactionRecord resistance, occupier;
    resistance.values.Add({"companionClass", "Companion"});
    resistance.values.Add({"companionClassCiv", "CivilianOutfit"});
    resistance.values.Add({"recruitFighter", "UnrelatedRecruit"});
    occupier.values.Add({"officer", "Commander"});
    occupier.tiers.Add("Conscripts");
    occupier.tiers.Add("Elite");
    occupier.tierThresholds.Add(5);
    occupier.tiersSniper.Add("");
    occupier.tiersSniper.Add("NamedSniper");
    const auto roster = BuildPortraitRoster(&resistance, &occupier, nullptr, nullptr,
                                            {{"FallenBody", "SavedModFace"}, {"Companion", "Face10"}});
    std::set<std::string> bodies;
    for (const auto& appearance : roster)
        bodies.emplace(appearance.body);
    CHECK(bodies == std::set<std::string>{"Companion", "CivilianOutfit", "Commander", "NamedSniper", "FallenBody"});
    CHECK(roster.size() == 17);
    CHECK(std::string(roster.back().face) == "SavedModFace");
    CHECK(BuildPortraitRoster(nullptr, nullptr, nullptr, nullptr, {}).empty());
}

TEST_CASE("portrait roster retains an unavailable fallen identity as well as the live fallback",
          "[guerrilla][portrait]")
{
    const char* text = "class CfgFaces { class Face10 {}; };";
    ParamFile config;
    QIStream stream(text, std::strlen(text));
    config.Parse(stream);
    const auto roster = BuildPortraitRoster(nullptr, nullptr, nullptr, config.FindEntry("CfgFaces"),
                                            {{"FallenBody", "RemovedModFace"}});
    REQUIRE(roster.size() == 2);
    CHECK(std::string(roster[0].face) == "RemovedModFace");
    CHECK(std::string(roster[1].face) == "Face10");
}
