// Script command surface for the Guerrilla GarrisonCache.  Registered from
// its own INIT_MODULE at stage 3 so GGameState.Init() (GameStateExt, stage 2)
// has already run - same pattern as ZoneRegistryCommands.cpp.

#include <Poseidon/Game/Guerrilla/GarrisonCache.hpp>

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
// Referenced from GarrisonCache.cpp to keep this TU (only content besides
// this: file-static commands + module registration) in the link.
void EnsureGarrisonCacheCommandsLinked() {}
} // namespace Poseidon::Guerrilla

// gmGarrisonSpawned <zoneIndex> -> bool (false out of range / despawned)
static GameValue GmGarrisonSpawned(const GameState* /*state*/, GameValuePar oper1)
{
    int index = toInt((float)oper1);
    return GarrisonCache::Instance().IsSpawned(index);
}

// gmGarrisonLive <zoneIndex> -> scalar (alive bodies; 0 when despawned)
static GameValue GmGarrisonLive(const GameState* /*state*/, GameValuePar oper1)
{
    int index = toInt((float)oper1);
    return (float)GarrisonCache::Instance().LiveCount(index);
}

// gmGarrisonGroups <zoneIndex> -> array of GROUP values (dead links skipped)
static GameValue GmGarrisonGroups(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;

    GarrisonCache& cache = GarrisonCache::Instance();
    int index = toInt((float)oper1);
    int n = cache.NGroups(index);
    for (int i = 0; i < n; i++)
    {
        AIGroup* grp = cache.GetGroup(index, i);
        if (!grp)
        {
            continue;
        }
        array.Add(GameValueExt(grp));
    }
    return value;
}

// gmGarrisonOnEvent ["garrisonSpawned"|"garrisonDespawned", handler]
// handler may be a STRING or a CODE value (GameDataCode's GetString returns
// the source, so the plain string coercion covers both)
static GameValue GmGarrisonOnEvent(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 2))
    {
        return NOTHING;
    }
    if (!CheckType(state, array[0], GameString))
    {
        return NOTHING;
    }

    GameStringType eventName = array[0];
    int type = GarrisonCache::EventTypeFromName(eventName);
    if (type < 0)
    {
        return NOTHING;
    }
    GameStringType handler = array[1];
    GarrisonCache::Instance().SetEventHandler((GarrisonEventType)type, handler);
    return NOTHING;
}

// gmGarrisonForceDespawn <zoneIndex> - immediate survivor write-back
static GameValue GmGarrisonForceDespawn(const GameState* /*state*/, GameValuePar oper1)
{
    int index = toInt((float)oper1);
    GarrisonCache::Instance().ForceDespawn(index);
    return NOTHING;
}

INIT_MODULE(GuerrillaGarrisonCache, 3)
{
    GGameState.NewFunction(GameFunction(GameBool, "gmGarrisonSpawned", GmGarrisonSpawned, GameScalar));
    GGameState.NewFunction(GameFunction(GameScalar, "gmGarrisonLive", GmGarrisonLive, GameScalar));
    GGameState.NewFunction(GameFunction(GameArray, "gmGarrisonGroups", GmGarrisonGroups, GameScalar));
    GGameState.NewFunction(GameFunction(GameNothing, "gmGarrisonOnEvent", GmGarrisonOnEvent, GameArray));
    GGameState.NewFunction(GameFunction(GameNothing, "gmGarrisonForceDespawn", GmGarrisonForceDespawn, GameScalar));
}
