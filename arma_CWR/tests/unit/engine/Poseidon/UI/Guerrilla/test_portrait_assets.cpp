// Source-only print tests: authored pixels, no game data or generated catalogue.
#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Game/Guerrilla/PortraitRecipe.hpp>
#include <Poseidon/Graphics/Textures/Image.hpp>
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp>
#include <array>
#include <chrono>
#include <filesystem>
#include <Random/randomGen.hpp>
#include <memory>
#include <thread>
#include <stdexcept>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

TEST_CASE("portrait asset RNG is nested, exception-safe and thread-local", "[guerrilla][portrait]")
{
    auto* campaign = &GRandGen;
    auto expected = std::make_unique<RandomGenerator>(*campaign);
    auto visual = std::make_unique<RandomGenerator>(1937, 512);
    auto nested = std::make_unique<RandomGenerator>(3, 4);
    RandomGenerator* otherThread = nullptr;
    {
        ScopedRandomGenerator scope(*visual);
        REQUIRE(&GRandGen == visual.get());
        for (int i = 0; i < 20; ++i)
            GRandGen.RandomValue();
        try
        {
            ScopedRandomGenerator inner(*nested);
            REQUIRE(&GRandGen == nested.get());
            throw std::runtime_error("asset load failure");
        }
        catch (const std::runtime_error&)
        {
        }
        REQUIRE(&GRandGen == visual.get());
        std::thread worker([&] { otherThread = &GRandGen; });
        worker.join();
        CHECK(otherThread == campaign);
    }
    REQUIRE(&GRandGen == campaign);
    auto actual = std::make_unique<RandomGenerator>(*campaign);
    for (int i = 0; i < 32; ++i)
        CHECK(actual->RandomValue() == expected->RandomValue());
}

TEST_CASE("portrait print rejects malformed readback", "[guerrilla][journal][portrait]")
{
    CHECK(StylePortrait({}).empty());
    CHECK(StylePortrait(std::vector<uint8_t>(512 * 512 * 4)).empty());
}

TEST_CASE("portrait print has opaque paper, rule and exactly 236 photograph pixels", "[guerrilla][journal][portrait]")
{
    std::vector<uint8_t> rgb(512 * 512 * 3, 128);
    const auto card = StylePortrait(rgb);
    REQUIRE(card.size() == 256 * 256 * 4);
    for (int y = 0; y < 256; ++y)
        for (int x = 0; x < 256; ++x)
        {
            const size_t at = (y * 256 + x) * 4;
            REQUIRE(card[at + 3] == 255);
            const bool inside = x >= 10 && x < 246 && y >= 10 && y < 246;
            const bool edge = x >= 9 && x <= 246 && y >= 9 && y <= 246 && (x == 9 || x == 246 || y == 9 || y == 246);
            const std::array<uint8_t, 3> expected = inside ? std::array<uint8_t, 3>{139, 132, 122}
                                                    : edge ? std::array<uint8_t, 3>{194, 190, 177}
                                                           : std::array<uint8_t, 3>{218, 214, 201};
            for (int c = 0; c < 3; ++c)
                REQUIRE(card[at + c] == expected[c]);
        }
}

TEST_CASE("portrait print preserves orientation and mutes authored colours", "[guerrilla][journal][portrait]")
{
    std::vector<uint8_t> rgb(512 * 512 * 3);
    for (int y = 0; y < 512; ++y)
        for (int x = 0; x < 512; ++x)
            rgb[(y * 512 + x) * 3 + (y < 256 ? 0 : 2)] = 255;
    const auto card = StylePortrait(rgb);
    const size_t top = (40 * 256 + 128) * 4, bottom = (210 * 256 + 128) * 4;
    CHECK(card[top] > card[top + 2]);
    CHECK(card[bottom + 2] > card[bottom]);
    CHECK(card[top + 1] > 18);
    CHECK(card[top] < 245);
    CHECK(StylePortrait(rgb) == card);
}

TEST_CASE("authored portrait print round trips as a 256 square opaque PNG", "[guerrilla][journal][portrait]")
{
    const auto pixels = StylePortrait(std::vector<uint8_t>(512 * 512 * 3, 128));
    struct TempFile
    {
        std::filesystem::path path =
            std::filesystem::temp_directory_path() /
            ("portrait-print-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".png");
        ~TempFile()
        {
            std::error_code ec;
            std::filesystem::remove(path, ec);
        }
    } file;
    REQUIRE(Image::FromRGBA(256, 256, pixels).Save(file.path.string()));
    const auto image = Image::FromFile(file.path.string());
    REQUIRE(image.valid());
    CHECK(image.width() == 256);
    CHECK(image.height() == 256);
    CHECK(image.ToRGBA().data() == pixels);
}

TEST_CASE("private portrait geometry preserves animation originals without sharing writes", "[portrait][guerrilla]")
{
    Shape source;
    source.ReallocTable(1);
    source.SetPos(0) = Vector3(1, 2, 3);
    source.SetNorm(0) = VUp;
    source.SetClip(0, 0);
    source.SaveOriginalPos();
    source.SetPos(0) = Vector3(4, 5, 6);
    Shape copy(source, true);
    REQUIRE(copy.OriginalPosValid());
    CHECK(copy.OrigPos(0).Y() == 2);
    copy.RestoreOriginalPos();
    copy.SetPos(0) = Vector3(9, 9, 9);
    CHECK(source.Pos(0).Y() == 5);
    source.RestoreOriginalPos();
    CHECK(source.Pos(0).Y() == 2);
}

TEST_CASE("portrait light snapshot owns references independently of scratch selection", "[portrait][guerrilla]")
{
    Ref<LightPoint> light = new LightPoint(Color(1, 1, 1), Color(1, 1, 1));
    LightList original;
    original.Add(Ref<Light>(light.GetRef()));
    const int refs = light->RefCounter();
    {
        LightList saved(original);
        CHECK(light->RefCounter() == refs + 1);
        original.Clear();
        REQUIRE(saved.Size() == 1);
        CHECK(saved[0].GetRef() == light.GetRef());
    }
    CHECK(light->RefCounter() == 1);
}
