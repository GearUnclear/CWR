// Script command surface for the Guerrilla StashRegistry.  Registered from
// its own INIT_MODULE at stage 3 so GGameState.Init() (GameStateExt, stage 2)
// has already run - same pattern as GarrisonCacheCommands.cpp.

#include <Poseidon/Game/Guerrilla/StashRegistry.hpp>

#include <Poseidon/AI/VehicleAI.hpp> // VehicleSupply (dyn_cast + keep-when-empty flag)

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
// Referenced from StashRegistry.cpp to keep this TU (only content besides
// this: file-static commands + module registration) in the link.
void EnsureStashRegistryCommandsLinked() {}
} // namespace Poseidon::Guerrilla

// gmStashRegister <obj> - flag the supply keep-when-empty + track it
// (idempotent; silent no-op on non-supply / destroyed objects)
static GameValue GmStashRegister(const GameState* /*state*/, GameValuePar oper1)
{
    Object* obj = GetObject(oper1);
    if (!obj || obj->IsDammageDestroyed())
    {
        return NOTHING;
    }
    VehicleSupply* veh = dyn_cast<VehicleSupply>(obj);
    if (!veh)
    {
        return NOTHING;
    }
    veh->SetKeepCargoWhenEmpty(true);
    StashRegistry::Instance().Register(veh);
    return NOTHING;
}

// gmStashUnregister <obj> - clear the flag + drop the row.  Deliberately does
// NOT force-delete an already-empty holder: it reverts to stock behavior and
// self-deletes on the next removal/clear that leaves it empty (stock holders
// only ever delete at removal time, so this matches engine semantics).
static GameValue GmStashUnregister(const GameState* /*state*/, GameValuePar oper1)
{
    Object* obj = GetObject(oper1);
    if (!obj) // no destroyed-gate: allow cleanup
    {
        return NOTHING;
    }
    VehicleSupply* veh = dyn_cast<VehicleSupply>(obj);
    if (!veh)
    {
        return NOTHING;
    }
    veh->SetKeepCargoWhenEmpty(false);
    StashRegistry::Instance().Unregister(veh);
    return NOTHING;
}

// gmStashCount -> scalar (tracked rows; dead holders drop on the prune tick)
static GameValue GmStashCount(const GameState* /*state*/)
{
    return (float)StashRegistry::Instance().Count();
}

// gmStash <i> -> [object, [x,z,h]] or [] out of range
static GameValue GmStash(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;

    StashRegistry& registry = StashRegistry::Instance();
    int index = toInt((float)oper1);
    if (index < 0 || index >= registry.Count())
    {
        return value;
    }
    array.Resize(2);
    // OBJECT-NULL-compatible when the link died between prune ticks
    array[0] = GameValueExt((Object*)registry.GetObject(index));

    // position in getPos order [easting, northing, elevation]
    Vector3 pos = registry.GetPos(index);
    GameValue posValue = state->CreateGameValue(GameArray);
    GameArrayType& posArr = posValue;
    posArr.Resize(3);
    posArr[0] = pos.X();
    posArr[1] = pos.Z();
    posArr[2] = pos.Y();
    array[1] = posValue;
    return value;
}

INIT_MODULE(GuerrillaStashRegistry, 3)
{
    GGameState.NewFunction(GameFunction(GameNothing, "gmStashRegister", GmStashRegister, GameObject));
    GGameState.NewFunction(GameFunction(GameNothing, "gmStashUnregister", GmStashUnregister, GameObject));
    GGameState.NewNularOp(GameNular(GameScalar, "gmStashCount", GmStashCount));
    GGameState.NewFunction(GameFunction(GameArray, "gmStash", GmStash, GameScalar));
}
