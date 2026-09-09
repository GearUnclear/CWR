// Shape check over the portrait catalogue that ships in guerrilla-mode/core.
//
// The dossier looks a portrait up by name and never validates it: PortraitKeyOf
// builds lower(bodyClass) + "__" + lower(face), Gather probes
// gmcore\portraits\<key>.paa with QIFStreamB::FileExist, and Compose hands the
// path to AddImage.  A file that is present but the wrong shape therefore fails
// silently at draw time, on a machine that is not this one.  So the constraints
// the notepad imposes are asserted here, where they are cheap:
//
//   * power of two and exactly square.  The PAA mip chain is not read from the
//     file, it is RECONSTRUCTED as mips[0]._w >> i (Pactext.cpp:1853-1857), so a
//     non-power-of-two texture is outside what the reader assumes.
//   * 256 on a side.  Under the RscHTML path the engine caps the used mip at
//     1024 (UIControlsExt.cpp:561-562) and MAX_MIPMAPS is 7, so the ceiling is
//     512; 256 sits comfortably under both and the on-screen size comes only
//     from AddImage's w/h anyway, so pixel size buys sharpness and nothing else.
//   * DXT1.  The cards are fully opaque, and PoseidonTools defaults to DXT5, so
//     a portrait that came out DXT5 means somebody dropped the -f DXT1 flag.
//   * a lowercase, key-shaped filename.  AddImage lowercases the path it is
//     given (UIControlsExt.cpp:545), so an uppercase file simply never resolves
//     on a case-sensitive filesystem.
//
// No game data and no GPU: ReadPAAInfo parses the header off disk.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Graphics/Textures/PAADecoder.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <vector>

using namespace Poseidon;

namespace
{

std::filesystem::path PortraitDir()
{
    // TESTS_ROOT_DIR is <repo>/tests and already exists as a compile definition
    // (tests/unit/engine/Poseidon/CMakeLists.txt); adding another one would
    // rebuild the whole target for nothing.
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "guerrilla-mode" / "core" / "portraits";
}

bool IsPowerOfTwo(int v)
{
    return v > 0 && (v & (v - 1)) == 0;
}

// ^[a-z0-9_]+__face[0-9]+\.paa$, hand-rolled so the test does not pull <regex>
// in for one pattern.
bool KeyShaped(const std::string& name)
{
    const std::string suffix = ".paa";
    if (name.size() <= suffix.size() || name.compare(name.size() - suffix.size(), suffix.size(), suffix) != 0)
    {
        return false;
    }
    const std::string stem = name.substr(0, name.size() - suffix.size());
    const std::string sep = "__face";
    const size_t at = stem.rfind(sep);
    if (at == std::string::npos || at == 0)
    {
        return false;
    }
    for (size_t i = 0; i < at; i++)
    {
        const char c = stem[i];
        if (!(std::islower(static_cast<unsigned char>(c)) || std::isdigit(static_cast<unsigned char>(c)) || c == '_'))
        {
            return false;
        }
    }
    const std::string digits = stem.substr(at + sep.size());
    if (digits.empty())
    {
        return false;
    }
    return std::all_of(digits.begin(), digits.end(),
                       [](char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; });
}

std::vector<std::filesystem::path> PortraitFiles()
{
    std::vector<std::filesystem::path> out;
    const std::filesystem::path dir = PortraitDir();
    if (!std::filesystem::is_directory(dir))
    {
        return out;
    }
    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".paa")
        {
            out.push_back(entry.path());
        }
    }
    std::sort(out.begin(), out.end());
    return out;
}

} // namespace

TEST_CASE("portrait catalogue directory ships portraits", "[guerrilla][journal][portrait]")
{
    // The cards are renders of game models and are gitignored, so a clone
    // legitimately holds none and every dossier falls back to the "Photograph
    // unavailable" panel. What this case pins is that the FOLDER is part of the
    // tree, carrying the tracked README and catalogue.json that say how to
    // reshoot; the count is reported so a reader can tell "none shot in this
    // checkout" from "the shoot broke". The per-card format case below runs
    // over whatever is present, so it is vacuous on a bare clone by design.
    const std::filesystem::path dir = PortraitDir();
    INFO("portrait directory: " << dir.string());
    REQUIRE(std::filesystem::is_directory(dir));
    WARN("portrait cards present in this checkout: " << PortraitFiles().size());
}

TEST_CASE("every shipped portrait is a 256x256 opaque DXT1 card", "[guerrilla][journal][portrait]")
{
    for (const auto& path : PortraitFiles())
    {
        const std::string name = path.filename().string();
        INFO("portrait: " << name);

        CHECK(KeyShaped(name));

        PAAInfo info;
        REQUIRE(ReadPAAInfo(path.string(), info));
        CHECK(info.isPaa);
        CHECK(info.width == 256);
        CHECK(info.height == 256);
        CHECK(info.width == info.height);
        CHECK(IsPowerOfTwo(info.width));
        CHECK(IsPowerOfTwo(info.height));
        REQUIRE(info.formatName != nullptr);
        CHECK(std::string(info.formatName) == "DXT1");
        // Repeated halving from 256 gives 8 levels down to 2x2; the reader stops
        // at MAX_MIPMAPS anyway, so anything from 5 up is usable.
        CHECK(info.mipmapCount >= 5);
        CHECK(info.mipmapCount <= 8);
    }
}
