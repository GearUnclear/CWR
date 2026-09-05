#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Asset/Addon/ModDoctor.hpp>
#include <Poseidon/Asset/Formats/P3D/P3DStructures.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include "../../Support/test_fixtures.hpp"

#include <cstring>
#include <fstream>
#include <string>
#include <vector>

using namespace Poseidon::ModDoctor;
using Catch::Approx;

namespace
{

void PushU32(std::vector<char>& out, uint32_t value)
{
    for (int i = 0; i < 4; ++i)
    {
        out.push_back(static_cast<char>((value >> (8 * i)) & 0xffU));
    }
}

//! One header record: asciiz name + packingMethod, originalSize, reserved, time, length.
void PushEntry(std::vector<char>& out, const char* name, uint32_t length)
{
    out.insert(out.end(), name, name + ::strlen(name));
    out.push_back('\0');
    PushU32(out, 0); // uncompressed
    PushU32(out, length);
    PushU32(out, 0);
    PushU32(out, 0);
    PushU32(out, length);
}

//! Smallest pbo that carries a single uncompressed entry.
std::vector<char> MakePbo(const char* entryName, const std::string& body)
{
    std::vector<char> pbo;
    PushEntry(pbo, entryName, static_cast<uint32_t>(body.size()));
    PushEntry(pbo, "", 0); // terminator
    pbo.insert(pbo.end(), body.begin(), body.end());
    return pbo;
}

std::vector<char> ReadFixture(const char* relative)
{
    std::ifstream in(TestFixtures::ResolveFixturePath(relative), std::ios::binary | std::ios::ate);
    REQUIRE(in.good());
    std::streamoff size = in.tellg();
    std::vector<char> bytes(static_cast<size_t>(size));
    in.seekg(0, std::ios::beg);
    in.read(bytes.data(), size);
    return bytes;
}

float ReadFloat(const std::vector<char>& bytes, size_t offset)
{
    float value = 0.0f;
    ::memcpy(&value, bytes.data() + offset, sizeof(value));
    return value;
}

} // namespace

TEST_CASE("ModDoctor: wildcard filter", "[moddoctor][addon]")
{
    REQUIRE(WildcardMatch("*", "LoBoWreck.pbo"));
    REQUIRE(WildcardMatch("*.pbo", "LoBoWreck.pbo"));
    REQUIRE(WildcardMatch("LoBo*.pbo", "loboWRECK.PBO")); // case-insensitive
    REQUIRE(WildcardMatch("LoBoWreck.pbo", "LoBoWreck.pbo"));
    REQUIRE(WildcardMatch("LoBoWrec?.pbo", "LoBoWreck.pbo"));
    REQUIRE_FALSE(WildcardMatch("*.p3d", "LoBoWreck.pbo"));
    REQUIRE_FALSE(WildcardMatch("LoBoPal*.pbo", "LoBoWreck.pbo"));
}

TEST_CASE("ModDoctor: undefined scope keyword", "[moddoctor][addon]")
{
    SECTION("A file without the #define header is patched, padded to length")
    {
        const char* config = "class Wreck\n"
                             "{\n"
                             "\tscope = public;\n"
                             "\tscopeWeapon=protected;\n"
                             "};\n";
        std::vector<Finding> findings = ScanConfigText(config, ::strlen(config), 0, "config.cpp");
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].defect == DefectClass::MissingDefineHeader);
        REQUIRE(findings[0].children.size() == 2);

        const Patch& first = findings[0].children[0].patches.at(0);
        REQUIRE(findings[0].children[0].line == 3);
        REQUIRE(first.original == "\tscope = public;");
        REQUIRE(first.replacement == "\tscope = 2;     ");
        REQUIRE(first.replacement.size() == first.original.size());

        const Patch& second = findings[0].children[1].patches.at(0);
        REQUIRE(second.original == "\tscopeWeapon=protected;");
        REQUIRE(second.replacement == "\tscopeWeapon = 1;      ");
        REQUIRE(second.replacement.size() == second.original.size());
    }

    SECTION("A file that defines the keyword is left alone")
    {
        const char* config = "#define private 0\n"
                             "#define protected 1\n"
                             "#define public 2\n"
                             "class Wreck { scope = public; };\n";
        std::vector<Finding> findings = ScanConfigText(config, ::strlen(config), 0, "config.cpp");
        REQUIRE(findings.empty());
    }

    SECTION("A keyword on a value entry that is not read as a number is left alone")
    {
        const char* config = "displayName = public;\n";
        std::vector<Finding> findings = ScanConfigText(config, ::strlen(config), 0, "config.cpp");
        REQUIRE(findings.empty());
    }
}

TEST_CASE("ModDoctor: malformed float literal", "[moddoctor][addon]")
{
    SECTION("Only the dotted token is rewritten, and to the same length")
    {
        const char* config = "tracerColor[] = {0.8, 0.5, 0.0.1, 0.25};\n";
        std::vector<Finding> findings = ScanConfigText(config, ::strlen(config), 0, "config.cpp");
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].defect == DefectClass::MalformedFloat);
        const Patch& patch = findings[0].patches.at(0);
        REQUIRE(patch.original == "0.0.1");
        // The strtod prefix is 0.0; the fractional zeros only pad it back to
        // length, so the token stays a plain number wherever it stood.
        REQUIRE(patch.replacement == "0.000");
        REQUIRE(patch.offset == static_cast<int64_t>(std::string(config).find("0.0.1")));
    }

    SECTION("A longer literal keeps its whole prefix")
    {
        const char* config = "value = 12.34.56;\n";
        std::vector<Finding> findings = ScanConfigText(config, ::strlen(config), 0, "config.cpp");
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].patches.at(0).replacement == "12.34000");
    }

    SECTION("Quoted strings, comments, paths and plain floats are not touched")
    {
        const char* config = "a = \"0.0.1\";\n"
                             "// version 1.0.1.2\n"
                             "/* 2.0.0 */\n"
                             "model = \\LoBo\\tex.0.0.1.paa;\n"
                             "b = 1.5;\n"
                             "c = 3;\n";
        std::vector<Finding> findings = ScanConfigText(config, ::strlen(config), 0, "config.cpp");
        REQUIRE(findings.empty());
    }
}

TEST_CASE("ModDoctor: patched config text parses as intended", "[moddoctor][addon]")
{
    std::string config = "class Wreck\n"
                         "{\n"
                         "\tscope = public;\n"
                         "\ttracerColor[] = {0.8, 0.5, 0.0.1, 0.25};\n"
                         "};\n";
    std::vector<Finding> findings = ScanConfigText(config.data(), config.size(), 0, "config.cpp");
    REQUIRE(ApplyPatches(config.data(), config.size(), findings) == 2);

    // Under --strict-config the reader does not coerce anything, so the patched
    // bytes have to stand on their own.
    struct StrictLiteralsGuard
    {
        bool saved = Poseidon::GParamFileStrictLiterals;
        ~StrictLiteralsGuard() { Poseidon::GParamFileStrictLiterals = saved; }
    } guard;
    Poseidon::GParamFileStrictLiterals = true;

    ParamFile pf;
    QIStream in(config.data(), static_cast<int>(config.size()));
    pf.Parse(in);

    ParamClass* wreck = pf.FindEntry("Wreck")->GetClassInterface();
    REQUIRE(wreck != nullptr);
    REQUIRE(wreck->FindEntry("scope")->GetInt() == 2);

    ParamEntry* tracer = wreck->FindEntry("tracerColor");
    REQUIRE(tracer != nullptr);
    REQUIRE(tracer->GetSize() == 4);
    REQUIRE((*tracer)[0].GetFloat() == Approx(0.8f));
    REQUIRE((*tracer)[2].IsFloatValue() == true);
    REQUIRE((*tracer)[2].GetFloat() == Approx(0.0f));
    REQUIRE((*tracer)[3].GetFloat() == Approx(0.25f));
}

TEST_CASE("ModDoctor: pbo entry table walk and in-place fix", "[moddoctor][addon]")
{
    const std::string body = "class Wreck\n"
                             "{\n"
                             "\tscope = public;\n"
                             "\ttracerColor[] = {0.8, 0.0.1};\n"
                             "};\n";
    std::vector<char> pbo = MakePbo("config.cpp", body);

    std::vector<PboEntry> entries;
    std::string error;
    REQUIRE(ReadPboEntries(pbo.data(), pbo.size(), entries, error));
    REQUIRE(error.empty());
    REQUIRE(entries.size() == 1);
    REQUIRE(entries[0].name == "config.cpp");
    REQUIRE(entries[0].length == body.size());
    REQUIRE(entries[0].dataOffset == static_cast<int64_t>(pbo.size() - body.size()));
    REQUIRE_FALSE(entries[0].compressed);

    std::vector<Finding> findings = ScanPbo(pbo.data(), pbo.size(), error);
    REQUIRE(error.empty());
    REQUIRE(findings.size() == 2); // the scope cause plus the float site

    const size_t sizeBefore = pbo.size();
    REQUIRE(ApplyPatches(pbo.data(), pbo.size(), findings) == 2);
    REQUIRE(pbo.size() == sizeBefore); // the header table must never move

    // Idempotent: a second scan finds nothing, and re-applying the stale plan
    // changes no bytes.
    std::vector<char> after = pbo;
    std::vector<Finding> again = ScanPbo(pbo.data(), pbo.size(), error);
    REQUIRE(again.empty());
    REQUIRE(ApplyPatches(pbo.data(), pbo.size(), findings) == 0);
    REQUIRE(pbo == after);
}

TEST_CASE("ModDoctor: rejects a malformed pbo header table", "[moddoctor][addon]")
{
    std::vector<PboEntry> entries;
    std::string error;

    std::vector<char> truncated = MakePbo("config.cpp", "x = 1;\n");
    truncated.resize(8);
    REQUIRE_FALSE(ReadPboEntries(truncated.data(), truncated.size(), entries, error));
    REQUIRE_FALSE(error.empty());

    // An entry that claims more data than the file holds.
    std::vector<char> oversized;
    PushEntry(oversized, "config.cpp", 1000000);
    PushEntry(oversized, "", 0);
    oversized.push_back('x');
    REQUIRE_FALSE(ReadPboEntries(oversized.data(), oversized.size(), entries, error));
}

TEST_CASE("ModDoctor: buried model origin", "[moddoctor][addon][p3d]")
{
    std::vector<char> model = ReadFixture("p3d/animated_morph_odol.p3d");
    REQUIRE(model.size() > 100);

    ModelOrigin origin;
    std::string error;
    REQUIRE(ReadModelOrigin(model.data(), model.size(), origin, error));
    REQUIRE(error.empty());

    SECTION("The located trailer agrees with the full ODOL reader")
    {
        QIStream in(model.data(), static_cast<int>(model.size()));
        Poseidon::Asset::Formats::BinaryReader reader(in);
        Poseidon::Asset::Formats::P3D::Model parsed = Poseidon::Asset::Formats::P3D::readModel(reader);

        REQUIRE(origin.minY == Approx(parsed.minMax[0].y));
        REQUIRE(origin.maxY == Approx(parsed.minMax[1].y));
        REQUIRE(origin.boundingCenterY == Approx(parsed.boundingCenter.y));
    }

    SECTION("A model that seats is left alone")
    {
        // The fixture is authored around its origin, so it is not buried.
        REQUIRE(origin.boundingCenterY + origin.maxY > 0.0f);
        REQUIRE(ScanModelEntry(model.data(), model.size(), 0, "fixture.p3d").empty());
    }

    SECTION("Sinking the origin is detected and repaired to the lowest vertex")
    {
        // Push boundingCenter.Y below the mesh top, the exact shape of the two
        // @LoBo M60A1 wrecks: the whole mesh then sits at or under the origin.
        const size_t bcY = static_cast<size_t>(origin.trailerOffset) + kBoundingCenterOffset + 4;
        const float buried = -origin.maxY - 1.0f;
        ::memcpy(model.data() + bcY, &buried, sizeof(buried));

        std::vector<Finding> findings = ScanModelEntry(model.data(), model.size(), 0, "fixture.p3d");
        REQUIRE(findings.size() == 1);
        REQUIRE(findings[0].defect == DefectClass::BuriedModelOrigin);
        REQUIRE(findings[0].patches.at(0).offset == static_cast<int64_t>(bcY));
        REQUIRE(findings[0].patches.at(0).original.size() == 4);
        REQUIRE(findings[0].patches.at(0).replacement.size() == 4);

        const size_t sizeBefore = model.size();
        REQUIRE(ApplyPatches(model.data(), model.size(), findings) == 1);
        REQUIRE(model.size() == sizeBefore);
        REQUIRE(ReadFloat(model, bcY) == Approx(-origin.minY));

        // Repaired: the lowest vertex now rests on the terrain, so a rerun is a
        // no-op.
        REQUIRE(ScanModelEntry(model.data(), model.size(), 0, "fixture.p3d").empty());
    }
}
