#include <catch2/catch_test_macros.hpp>

#include <Poseidon/AI/AI.hpp>
#include <Poseidon/World/Entities/Vehicles/Vehicle.hpp>
#include <Poseidon/World/Scene/ObjectClasses.hpp>

#include <type_traits>

// The switchLight and lightIsOn script commands find their lamp with
// dyn_cast<StreetLamp>. StreetLamp used to declare no casting id of its own, so
// StreetLamp::_classId named Entity::_classId and the cast accepted every Entity:
// a house, a soldier or a car came back as a "lamp". SwitchLight then wrote lamp
// state into that object's memory and SimulateSwitch read a bulb hit point
// through the wrong type layout: an access violation in EntityHit::GetHit, or a
// corrupted soldier that crashed in EntityAI::GetHitCont on its next draw.

using namespace Poseidon;

TEST_CASE("StreetLamp - declares its own casting id", "[scene][streetlamp][casting]")
{
    // Without USE_CASTING the class inherits its parent's IsClassId, so the member
    // pointer type names the parent and the id object is the parent's.
    CHECK(std::is_same_v<decltype(&StreetLamp::IsClassId), bool (StreetLamp::*)(const int*) const>);
    CHECK(&StreetLamp::_classId != &Entity::_classId);
}

TEST_CASE("StreetLamp - dyn_cast does not take a plain entity for a lamp", "[scene][streetlamp][casting]")
{
    Ref<EntityType> type = new EntityType(nullptr);
    Ref<Entity> entity = new Entity(nullptr, type, -1);
    Object* obj = entity;

    CHECK(dyn_cast<Entity>(obj) == entity.GetRef());
    CHECK(dyn_cast<StreetLamp>(obj) == nullptr);
}
