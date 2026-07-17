#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/StashRegistry.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>

#include <filesystem>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

TEST_CASE("StashRegistry - empty registry queries are safe", "[game][guerrilla]")
{
    StashRegistry registry;
    CHECK(registry.Count() == 0);
    CHECK(registry.GetObject(0) == nullptr);
    CHECK(registry.GetObject(-1) == nullptr);
    CHECK(registry.GetPos(-1) == VZero);
    CHECK(registry.GetPos(0) == VZero);
    CHECK_FALSE(registry.Register(nullptr));
    CHECK_FALSE(registry.Unregister(nullptr));
    registry.Simulate(10.0f); // must not crash on an empty row set
    CHECK(registry.Count() == 0);
}

TEST_CASE("StashRegistry - rows with dead holders are dropped on load", "[game][guerrilla][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "stash-registry-roundtrip.bin";

    {
        StashRegistry registry;
        // rows carry a null object link (no live world in a unit test) -
        // exactly what a holder that did not survive the load looks like
        registry.AddRowForTest(Vector3(100.0f, 10.0f, 200.0f));
        registry.AddRowForTest(Vector3(4200.0f, 25.0f, 8800.0f));
        REQUIRE(registry.Count() == 2);
        CHECK(registry.GetPos(1).X() == Approx(4200.0f));

        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(registry.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
        // saving must not disturb the live rows
        CHECK(registry.Count() == 2);
    }

    StashRegistry loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }

    // null refs mean the holders are genuinely gone (they ride the world's
    // building serializer) - the contract is DROP, not resurrect
    CHECK(loaded.Count() == 0);

    std::filesystem::remove(archivePath);
}

TEST_CASE("StashRegistry - Simulate prunes dead links and is throttled", "[game][guerrilla]")
{
    StashRegistry registry;
    registry.AddRowForTest(Vector3(100.0f, 10.0f, 200.0f));
    REQUIRE(registry.Count() == 1);

    // a full tick interval elapses -> the dead (null-link) row is pruned
    registry.Simulate(StashRegistry::TickInterval + 0.1f);
    CHECK(registry.Count() == 0);

    // below the throttle nothing happens, even with a dead row present
    registry.AddRowForTest(Vector3(300.0f, 5.0f, 400.0f));
    registry.Simulate(0.1f);
    CHECK(registry.Count() == 1);
}
