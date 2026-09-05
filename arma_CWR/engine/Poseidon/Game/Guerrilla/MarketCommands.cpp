// Script command surface for the Guerrilla market (dealers + stock).
// Registered from its own INIT_MODULE at stage 3 so GGameState.Init()
// (GameStateExt, stage 2) has already run - same pattern as
// StashRegistryCommands.cpp.
//
//   gmMarketActive                  -> bool (CfgGuerrillaMarket present + zones active)
//   gmMarketValue "<key>"           -> scalar (dealerShare | dealerRespawnSeconds | hqMoveCost)
//   gmDealerCount                   -> scalar
//   gmDealer <i>                    -> [zoneName, "WEAPON"|"VEHICLE", [x,y,z], npc, [lotx,loty,lotz]]
//                                      ([] out of range; npc objNull while dead/unspawned)
//   gmDealerNearest [kind, [x,y,z]] -> index of the nearest dealer of that kind, -1 none
//   gmDealerStock "WEAPON"          -> [[class, displayName, price, magazine, mags], ...]
//   gmDealerStock "VEHICLE"         -> [[class, displayName, price], ...]
// Positions in getPos order [easting, northing, elevation].

#include <Poseidon/Game/Guerrilla/Market.hpp>

#include <Poseidon/AI/EntityAI.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp> // GLOB_LAND (height above ground)

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
// Referenced from Market.cpp to keep this TU (only content besides this:
// file-static commands + module registration) in the link.
void EnsureMarketCommandsLinked() {}
} // namespace Poseidon::Guerrilla

// position in getPos order [easting, northing, height above ground] - the
// getPos/setPos convention, so a dealer's lot feeds straight into
// createVehicle (an absolute Y would spawn the hull in the air)
static GameValue PosValue(const GameState* state, Vector3Par pos)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    float height = pos.Y();
    if (GLandscape)
    {
        float dx, dz;
        height -= GLOB_LAND->SurfaceYAboveWater(pos.X(), pos.Z(), &dx, &dz);
    }
    array.Resize(3);
    array[0] = pos.X();
    array[1] = pos.Z();
    array[2] = height;
    return value;
}

// gmMarketActive -> bool
static GameValue GmMarketActive(const GameState* /*state*/)
{
    return Market::Instance().IsActive();
}

// gmMarketValue "<key>" -> scalar
static GameValue GmMarketValue(const GameState* /*state*/, GameValuePar oper1)
{
    GameStringType key = oper1;
    return Market::Instance().Value(key);
}

// gmDealerCount -> scalar
static GameValue GmDealerCount(const GameState* /*state*/)
{
    return (float)Market::Instance().DealerCount();
}

// gmDealer <i> -> [zoneName, kind, [x,y,z], npc, [lotx,loty,lotz]] or []
static GameValue GmDealer(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    const DealerRecord* row = Market::Instance().Dealer(toInt((float)oper1));
    if (!row)
    {
        return value;
    }
    array.Resize(5);
    array[0] = GameStringType(row->zoneName);
    array[1] = GameStringType(Market::KindName(row->kind));
    array[2] = PosValue(state, row->pos);
    // OBJECT-NULL-compatible while the dealer is dead or not yet spawned
    array[3] = GameValueExt((Object*)row->npc.GetLink());
    array[4] = PosValue(state, row->lotPos);
    return value;
}

// gmDealerNearest [kind, [x,y,z]] -> index or -1
static GameValue GmDealerNearest(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (!CheckSize(state, array, 2))
    {
        return -1.0f;
    }
    if (!CheckType(state, array[0], GameString) || !CheckType(state, array[1], GameArray))
    {
        return -1.0f;
    }
    GameStringType kindName = array[0];
    int kind = Market::KindFromName(kindName);
    if (kind < 0)
    {
        state->SetError(EvalGen, "gmDealerNearest: unknown dealer kind");
        return -1.0f;
    }
    Vector3 pos;
    if (!GetPos(state, pos, array[1]))
    {
        return -1.0f;
    }
    return (float)Market::Instance().NearestDealer(kind, pos);
}

// gmDealerStock "WEAPON"|"VEHICLE" -> rows
static GameValue GmDealerStock(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    GameStringType kindName = oper1;
    int kind = Market::KindFromName(kindName);
    const Market& market = Market::Instance();
    if (kind == DKWeapon)
    {
        array.Resize(market.NWeapons());
        for (int i = 0; i < market.NWeapons(); i++)
        {
            const MarketWeaponRow& w = market.Weapon(i);
            GameValue rowValue = state->CreateGameValue(GameArray);
            GameArrayType& row = rowValue;
            row.Resize(5);
            row[0] = GameStringType(w.weapon);
            row[1] = GameStringType(w.displayName);
            row[2] = w.price;
            row[3] = GameStringType(w.magazine);
            row[4] = (float)w.mags;
            array[i] = rowValue;
        }
    }
    else if (kind == DKVehicle)
    {
        array.Resize(market.NVehicles());
        for (int i = 0; i < market.NVehicles(); i++)
        {
            const MarketVehicleRow& v = market.Vehicle(i);
            GameValue rowValue = state->CreateGameValue(GameArray);
            GameArrayType& row = rowValue;
            row.Resize(3);
            row[0] = GameStringType(v.vehicle);
            row[1] = GameStringType(v.displayName);
            row[2] = v.price;
            array[i] = rowValue;
        }
    }
    else
    {
        state->SetError(EvalGen, "gmDealerStock: unknown dealer kind");
    }
    return value;
}

INIT_MODULE(GuerrillaMarket, 3)
{
    GGameState.NewNularOp(GameNular(GameBool, "gmMarketActive", GmMarketActive));
    GGameState.NewFunction(GameFunction(GameScalar, "gmMarketValue", GmMarketValue, GameString));
    GGameState.NewNularOp(GameNular(GameScalar, "gmDealerCount", GmDealerCount));
    GGameState.NewFunction(GameFunction(GameArray, "gmDealer", GmDealer, GameScalar));
    GGameState.NewFunction(GameFunction(GameScalar, "gmDealerNearest", GmDealerNearest, GameArray));
    GGameState.NewFunction(GameFunction(GameArray, "gmDealerStock", GmDealerStock, GameString));
}
