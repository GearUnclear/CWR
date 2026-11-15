#include <catch2/catch_test_macros.hpp>

#include <Poseidon/World/Scene/LandFitCache.hpp>

#include <string>

// A land-following bush (ClipLandKeep vertices) is fitted to the terrain under
// it. Perf plan W1-9 fits each plain object once per LOD level and draws that
// fitted copy until something the fit reads changes. These tests pin when a
// kept fit may be reused: only for the same level, the same world matrix, the
// same terrain heights and the same damage. A stale reuse would draw a bush
// floating over or sunk into a crater, a moved bush at its old slope, or a
// LOD switch with the other level's vertices.

using Poseidon::LandFit::FitKey;
using Poseidon::LandFit::FitTable;
using Poseidon::LandFit::MakeFitKey;

namespace
{
int levelA = 0;
int levelB = 0;
int ownerX = 0;
int ownerY = 0;
const void* kLevel0 = &levelA;
const void* kLevel1 = &levelB;
const void* kBushX = &ownerX;
const void* kBushY = &ownerY;

// Aside, up, direction, position: an unrotated object at (1000, 50, 2000).
const float kAtRest[12] = {1, 0, 0, 0, 1, 0, 0, 0, 1, 1000, 50, 2000};

FitKey Key(const void* level = kLevel0, int levelIndex = 0, const float* matrix = kAtRest, std::uint64_t terrain = 7,
           float damage = 0.0f)
{
    return MakeFitKey(level, levelIndex, 40, matrix, terrain, damage);
}

using Table = FitTable<std::string>;
} // namespace

TEST_CASE("land fit: a kept fit is found for the same key", "[landfit]")
{
    Table table;
    table.Store(kBushX, Key(), std::string("fit0"), 1);
    auto* e = table.Find(kBushX, Key(), 2);
    REQUIRE(e != nullptr);
    CHECK(e->payload == "fit0");
    CHECK(e->lastUsed == 2);
    CHECK(table.Size() == 1);
}

TEST_CASE("land fit: moving the object invalidates its fit", "[landfit]")
{
    Table table;
    table.Store(kBushX, Key(), std::string("fit0"), 1);

    float moved[12];
    std::memcpy(moved, kAtRest, sizeof(moved));
    moved[9] += 0.001f; // a millimetre east
    CHECK(table.Find(kBushX, Key(kLevel0, 0, moved), 2) == nullptr);

    float turned[12];
    std::memcpy(turned, kAtRest, sizeof(turned));
    turned[0] = 0.0f; // rotated a quarter turn: aside and direction swap
    turned[2] = -1.0f;
    turned[6] = 1.0f;
    turned[8] = 0.0f;
    CHECK(table.Find(kBushX, Key(kLevel0, 0, turned), 2) == nullptr);

    // A fallen tree tilts the up vector.
    float fallen[12];
    std::memcpy(fallen, kAtRest, sizeof(fallen));
    fallen[4] = 0.0f;
    fallen[5] = 1.0f;
    CHECK(table.Find(kBushX, Key(kLevel0, 0, fallen), 2) == nullptr);

    // Rebuilding at the new place replaces the entry for that level.
    table.Store(kBushX, Key(kLevel0, 0, moved), std::string("fit-moved"), 2);
    CHECK(table.Size() == 1);
    CHECK(table.Find(kBushX, Key(), 3) == nullptr);
    REQUIRE(table.Find(kBushX, Key(kLevel0, 0, moved), 3) != nullptr);
}

TEST_CASE("land fit: damage invalidates the fit", "[landfit]")
{
    Table table;
    table.Store(kBushX, Key(), std::string("fit0"), 1);
    CHECK(table.Find(kBushX, Key(kLevel0, 0, kAtRest, 7, 0.25f), 2) == nullptr);
    table.Store(kBushX, Key(kLevel0, 0, kAtRest, 7, 0.25f), std::string("fit-hit"), 2);
    REQUIRE(table.Find(kBushX, Key(kLevel0, 0, kAtRest, 7, 0.25f), 3) != nullptr);
}

TEST_CASE("land fit: a terrain height change invalidates every fit", "[landfit]")
{
    Table table;
    table.Store(kBushX, Key(), std::string("x"), 1);
    table.Store(kBushY, Key(), std::string("y"), 1);
    CHECK(table.Find(kBushX, Key(kLevel0, 0, kAtRest, 8), 2) == nullptr);
    CHECK(table.Find(kBushY, Key(kLevel0, 0, kAtRest, 8), 2) == nullptr);
    // the old version is no longer asked for; the stale entries age out
    CHECK(table.Sweep(1000, 600) == 2);
    CHECK(table.Size() == 0);
}

TEST_CASE("land fit: each LOD level keeps its own entry", "[landfit]")
{
    Table table;
    table.Store(kBushX, Key(kLevel0, 0), std::string("lod0"), 1);
    table.Store(kBushX, Key(kLevel1, 1), std::string("lod1"), 1);
    CHECK(table.Size() == 2);

    auto* near = table.Find(kBushX, Key(kLevel0, 0), 2);
    auto* far = table.Find(kBushX, Key(kLevel1, 1), 2);
    REQUIRE(near != nullptr);
    REQUIRE(far != nullptr);
    CHECK(near->payload == "lod0");
    CHECK(far->payload == "lod1");

    // A key naming level 1's index with level 0's shape is not level 1's fit.
    CHECK(table.Find(kBushX, Key(kLevel0, 1), 2) == nullptr);
    // A level index never stored is a miss, not another level's entry.
    CHECK(table.Find(kBushX, Key(kLevel0, 2), 2) == nullptr);
}

TEST_CASE("land fit: owners never share entries", "[landfit]")
{
    Table table;
    table.Store(kBushX, Key(), std::string("x"), 1);
    CHECK(table.Find(kBushY, Key(), 2) == nullptr);
    table.Forget(kBushX);
    CHECK(table.Find(kBushX, Key(), 2) == nullptr);
    CHECK(table.Size() == 0);
}

TEST_CASE("land fit: a reloaded level with another vertex count is a miss", "[landfit]")
{
    Table table;
    table.Store(kBushX, Key(), std::string("x"), 1);
    const FitKey other = MakeFitKey(kLevel0, 0, 41, kAtRest, 7, 0.0f);
    CHECK(table.Find(kBushX, other, 2) == nullptr);
}

TEST_CASE("land fit: unused entries age out, used ones stay", "[landfit]")
{
    Table table;
    table.Store(kBushX, Key(kLevel0, 0), std::string("x0"), 1);
    table.Store(kBushX, Key(kLevel1, 1), std::string("x1"), 1);
    table.Store(kBushY, Key(), std::string("y"), 1);
    REQUIRE(table.Find(kBushX, Key(kLevel0, 0), 500) != nullptr);
    CHECK(table.Sweep(700, 600) == 2);
    CHECK(table.Size() == 1);
    CHECK(table.Find(kBushX, Key(kLevel0, 0), 701) != nullptr);
    CHECK(table.Find(kBushX, Key(kLevel1, 1), 701) == nullptr);
    CHECK(table.Find(kBushY, Key(), 701) == nullptr);
}

TEST_CASE("land fit: the terrain version mixes every input", "[landfit]")
{
    using Poseidon::LandFit::MixVersion;
    const std::uint64_t seed = 1469598103934665603ull;
    CHECK(MixVersion(seed, 1) != MixVersion(seed, 2));
    CHECK(MixVersion(MixVersion(seed, 1), 2) != MixVersion(MixVersion(seed, 1), 3));
}
