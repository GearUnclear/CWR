#include <catch2/catch_test_macros.hpp>

#include <Poseidon/AI/AI.hpp>
#include <Poseidon/World/Entities/Vehicles/House.hpp>
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
