#include <catch2/catch_test_macros.hpp>

#include <Poseidon/AI/AI.hpp>
#include <Poseidon/World/Entities/Vehicles/House.hpp>
#include <Poseidon/World/Entities/Vehicles/Vehicle.hpp>
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>

// Guards around the two crashes @LoBo's static scenery props reproduced when they
// were createVehicle'd at runtime:
//
//  * Building::DrawProxies dereferenced WeaponProxy::obj after IsPresent() said the
//    proxy was there. `obj` is an LLink, so it reads back null once its target is
//    released, while `selection` keeps whatever InitShape put there - present and
//    null at the same time, and an 0xC0000005 in the renderer.
//  * Building::Building sized _locks from NPos(), which is read through a
//    static_cast of the type object. A type that was not really a BuildingType gave
//    an arbitrary count and memmove'd that range inside AutoArray::Resize.

using namespace Poseidon;

TEST_CASE("WeaponProxy - a default proxy is absent", "[vehicles][house][proxy]")
{
    WeaponProxy proxy;
    CHECK(proxy.selection == -1);
    CHECK(proxy.IsPresent() == false);
}

TEST_CASE("WeaponProxy - a selection with no object is absent", "[vehicles][house][proxy]")
{
    // Exactly the state a released proxy object leaves behind. `selection >= 0`
    // alone used to answer true here and DrawProxies then dereferenced null.
    WeaponProxy proxy;
    proxy.selection = 0;
    REQUIRE(proxy.obj.GetLink() == nullptr);
    CHECK(proxy.IsPresent() == false);

    proxy.selection = 7;
    CHECK(proxy.IsPresent() == false);
}

TEST_CASE("BuildingType - the proxy table covers every LOD the renderer can ask for", "[vehicles][house][proxy]")
{
    // InitShape fills _proxies per LOD and DrawProxies indexes it with a LOD number,
    // so the bound both sides clamp to has to be the array's own size.
    STATIC_REQUIRE(MAX_LOD_LEVELS > 0);
    // LOD_INVISIBLE is a sentinel, not an index: it must fall outside the table so
    // the range check in DrawProxies rejects it rather than reading past the end.
    STATIC_REQUIRE(LOD_INVISIBLE >= MAX_LOD_LEVELS);
}

TEST_CASE("Building - the interior-position bound is plausible but finite", "[vehicles][house]")
{
    // Sanity, not a tuning knob: big enough for any real interior, small enough that
    // a garbage count read through the wrong type layout cannot drive an allocation.
    STATIC_REQUIRE(MaxHousePositions > 1024);
    STATIC_REQUIRE(MaxHousePositions < 1000000);
}

TEST_CASE("StaticSeatOffsetY - authored seating is kept whenever any of the mesh clears the terrain",
          "[vehicles][house][seat]")
{
    // The healthy LoBoWreck models: origin at or near the tracks, mesh mostly
    // above it. Origin-on-ground is the authored intent - keep it exactly.
    CHECK(StaticSeatOffsetY(0.522f, -0.9f, 0.9f) == 0.522f);   // LoBo_t54wrck
    CHECK(StaticSeatOffsetY(1.379f, -1.2f, 1.2f) == 1.379f);   // LoBo_Shot_Wreck1

    // Deliberately sunk content (rocks, ruins): origin above the mesh bottom, so
    // part of the model ends up underground - but the top still clears the
    // terrain. Authored sinking, not a defect. Keep it.
    CHECK(StaticSeatOffsetY(-0.5f, -1.0f, 1.0f) == -0.5f);
}

TEST_CASE("StaticSeatOffsetY - a mesh entirely at or below its own origin is seated on the terrain",
          "[vehicles][house][seat]")
{
    // LoBo_M60A1_wreck as shipped: boundingCenter.Y -1.9205, mesh -1.47..1.47.
    // Origin-on-ground puts the roof 0.45 m under the sand - invisible at any
    // terrain height. Rescue: lowest vertex on the terrain, i.e. -meshMinY,
    // exactly the seat tools/lobo/fix-lobo-model-origin.ps1 bakes into the p3d.
    CHECK(StaticSeatOffsetY(-1.9205f, -1.47f, 1.47f) == 1.47f);
    // LoBo_M60A1_wreck2: boundingCenter.Y -2.0573, mesh -1.1374..1.1374.
    CHECK(StaticSeatOffsetY(-2.0573f, -1.1374f, 1.1374f) == 1.1374f);
    // Boundary: mesh top exactly on the terrain is still invisible - rescue it.
    CHECK(StaticSeatOffsetY(-1.0f, -1.0f, 1.0f) == 1.0f);
}
