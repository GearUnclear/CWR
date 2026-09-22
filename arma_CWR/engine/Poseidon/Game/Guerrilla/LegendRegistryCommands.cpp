// Script command surface for the Guerrilla Legend registry.  Registered from
// its own INIT_MODULE at stage 3 so GGameState.Init() (GameStateExt, stage 2)
// has already run - same pattern as JournalCommands.cpp.
//
//   gmLegendBind [i, obj]        stamp the registry identity onto a freshly
//                                created companion body; creates the row when
//                                the poll has not seen that companion yet
//   gmLegendName <i>             -> the companion's display name; falls back to
//                                   GM_COMP_NAMES select i, so it is never ""
//                                   while the roster has the index
//   gmLegendFace <i>             -> the companion's face token ("" when unknown)
//   gmLegendId   <i>             -> the companion's stable row id ("" unknown)
//   gmLegendCount                -> scalar, ROWS (companions then enemy Legends)
//   gmLegendInfo <row>           -> [id, displayName, kind, role, zone, alive,
//                                    legend, awardMask]; [] out of range
//   gmLegendHistory              -> [version, seed, opening1, opening2]
//   gmLegendHistoryEvent <k>     -> [title, text, place]; [] out of range
//   gmLegendPos <row>            -> the enemy Legend's stand in SCRIPT order
//                                   [easting, northing, elevation], the same
//                                   shape getPos and gmZone element 8 use; []
//                                   out of range (a row that never found a
//                                   stand reads [0,0,0])
//   gmLegendBody <row>           -> the named body; objNull when never spawned
//                                   or deleted.  A DEAD body still comes back:
//                                   ask alive/damage separately, never treat a
//                                   null link as a death
//   gmLegendVehicle <row>        -> the tank commander's hull; objNull once it
//                                   is destroyed or when he never had one
//   gmLegendDefeated <row>       -> bool, the persisted latch
//   gmLegendMarker <row>         -> [name, type, color, text] read back out of
//                                   markersMap; [] when the row has no marker.
//                                   It exists because there is no stock
//                                   markerText reader (GameStateExt.cpp has
//                                   markerPos / markerType / markerColor only).
//
// INDEX CONVENTION: the three name/face/id readers take a COMPANION index (the
// GM_COMP_* index the scripts already carry); gmLegendInfo takes a ROW index,
// because that is the only way to reach the enemy Legend rows.  Rows are kept
// companions-first, so gmLegendInfo 0 is the first companion for the life of the
// campaign.

#include <Poseidon/Game/Guerrilla/LegendRegistry.hpp>

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/AI/AICore.hpp>                 // markersMap
#include <Poseidon/AI/Path/ArcadeWaypoint.hpp>    // ArcadeMarkerInfo
#include <Poseidon/Foundation/Common/FltOpts.hpp> // toInt
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
// Referenced from LegendRegistry.cpp to keep this TU (only content besides
// this: file-static commands + module registration) in the link.
void EnsureLegendRegistryCommandsLinked() {}
} // namespace Poseidon::Guerrilla

// gmLegendBind [compIndex, object] -> bool
// Spliced into GM_fnCompSpawn right after the loadout re-apply, so the body
// wears its Legend name from the frame it exists rather than from the next
// 1 Hz poll.  Must run AFTER createUnit returns: the init string executes
// before the engine stamps the pool identity.
static GameValue GmLegendBind(const GameState* state, GameValuePar oper1)
{
    const GameArrayType& array = oper1;
    if (array.Size() != 2)
    {
        state->SetError(EvalGen, "gmLegendBind: [index, object]");
        return false;
    }
    if (!CheckType(state, array[0], GameScalar) || !CheckType(state, array[1], GameObject))
    {
        return false;
    }
    const int index = toInt((float)array[0]);
    Object* body = GetObject(array[1]);
    if (!body)
    {
        return false;
    }
    return LegendRegistry::Instance().Bind(index, body);
}

// gmLegendName <compIndex> -> string
static GameValue GmLegendName(const GameState* /*state*/, GameValuePar oper1)
{
    return GameStringType(LegendRegistry::Instance().CompanionDisplayName(toInt((float)oper1)));
}

// gmLegendFace <compIndex> -> string
static GameValue GmLegendFace(const GameState* /*state*/, GameValuePar oper1)
{
    const LegendRegistry& registry = LegendRegistry::Instance();
    const int row = registry.FindByCompIndex(toInt((float)oper1));
    return GameStringType(row >= 0 ? registry.Row(row).face : RString());
}

// gmLegendId <compIndex> -> string
static GameValue GmLegendId(const GameState* /*state*/, GameValuePar oper1)
{
    const LegendRegistry& registry = LegendRegistry::Instance();
    const int row = registry.FindByCompIndex(toInt((float)oper1));
    return GameStringType(row >= 0 ? registry.Row(row).id : RString());
}

// gmLegendCount -> scalar
static GameValue GmLegendCount(const GameState* /*state*/)
{
    return (float)LegendRegistry::Instance().RowCount();
}

// gmLegendInfo <rowIndex> -> [id, displayName, kind, role, zone, alive, legend, awardMask]
static GameValue GmLegendInfo(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    const LegendRegistry& registry = LegendRegistry::Instance();
    const int index = toInt((float)oper1);
    if (index < 0 || index >= registry.RowCount())
    {
        return value;
    }
    const LegendRow& row = registry.Row(index);
    RString role = row.role;
    if (row.kind == LKCompanion)
    {
        const int rank =
            row.rankSeen < 0
                ? 0
                : (row.rankSeen >= LegendRegistry::NRankLadder ? LegendRegistry::NRankLadder - 1 : row.rankSeen);
        role = RString(LegendRegistry::kRankLadder[rank]);
    }
    array.Resize(8);
    array[0] = GameStringType(row.id);
    array[1] = GameStringType(registry.DisplayName(row));
    array[2] = (float)row.kind;
    array[3] = GameStringType(role);
    array[4] = GameStringType(row.kind == LKCompanion ? row.lastZone : row.zoneName);
    array[5] = row.alive;
    array[6] = row.legend;
    array[7] = (float)row.awardMask;
    return value;
}

// gmLegendHistory -> [version, seed, opening1, opening2]
static GameValue GmLegendHistory(const GameState* state)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    const HistoryRecord& history = LegendRegistry::Instance().History();
    array.Resize(4);
    array[0] = (float)history.version;
    array[1] = (float)history.seed;
    array[2] = GameStringType(history.openingPage1);
    array[3] = GameStringType(history.openingPage2);
    return value;
}

// gmLegendHistoryEvent <k> -> [title, text, place]
static GameValue GmLegendHistoryEvent(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    const HistoryRecord& history = LegendRegistry::Instance().History();
    const int index = toInt((float)oper1);
    if (!history.Present() || index < 0 || index >= kHistoryEvents)
    {
        return value;
    }
    array.Resize(3);
    array[0] = GameStringType(history.eventTitle[index]);
    array[1] = GameStringType(history.eventText[index]);
    array[2] = GameStringType(history.eventPlace[index]);
    return value;
}

// gmLegendPos <rowIndex> -> [easting, northing, elevation]
static GameValue GmLegendPos(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    const LegendRegistry& registry = LegendRegistry::Instance();
    const int index = toInt((float)oper1);
    if (index < 0 || index >= registry.RowCount())
    {
        return value;
    }
    // SCRIPT order, [easting, northing, elevation], not engine Vector3 order:
    // getPos returns X then Z then Y (ObjGetPos, GameStateExtGrp.cpp) and
    // gmZone's element 8 does the same, so a stand read here can be handed
    // straight to setPos or measured against a zone centre.
    const Vector3 pos = registry.RowPos(index);
    array.Resize(3);
    array[0] = pos.X();
    array[1] = pos.Z();
    array[2] = pos.Y();
    return value;
}

// gmLegendBody <rowIndex> -> object
static GameValue GmLegendBody(const GameState* /*state*/, GameValuePar oper1)
{
    return GameValueExt(LegendRegistry::Instance().RowBody(toInt((float)oper1)));
}

// gmLegendVehicle <rowIndex> -> object
static GameValue GmLegendVehicle(const GameState* /*state*/, GameValuePar oper1)
{
    return GameValueExt(LegendRegistry::Instance().RowVehicle(toInt((float)oper1)));
}

// gmLegendDefeated <rowIndex> -> bool
static GameValue GmLegendDefeated(const GameState* /*state*/, GameValuePar oper1)
{
    const LegendRegistry& registry = LegendRegistry::Instance();
    const int index = toInt((float)oper1);
    if (index < 0 || index >= registry.RowCount())
    {
        return false;
    }
    return registry.Row(index).defeated;
}

// gmLegendMarker <rowIndex> -> [name, type, color, text]
static GameValue GmLegendMarker(const GameState* state, GameValuePar oper1)
{
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    const LegendRegistry& registry = LegendRegistry::Instance();
    const int index = toInt((float)oper1);
    if (index < 0 || index >= registry.RowCount())
    {
        return value;
    }
    const RString name = registry.Row(index).markerName;
    if (name.GetLength() == 0)
    {
        return value;
    }
    for (int m = 0; m < markersMap.Size(); m++)
    {
        const ArcadeMarkerInfo& info = markersMap[m];
        if (stricmp(info.name, name) != 0)
        {
            continue;
        }
        array.Resize(4);
        array[0] = GameStringType(info.name);
        array[1] = GameStringType(info.type);
        array[2] = GameStringType(info.colorName);
        array[3] = GameStringType(info.text);
        return value;
    }
    return value;
}

INIT_MODULE(GuerrillaLegends, 3)
{
    GGameState.NewFunction(GameFunction(GameBool, "gmLegendBind", GmLegendBind, GameArray));
    GGameState.NewFunction(GameFunction(GameString, "gmLegendName", GmLegendName, GameScalar));
    GGameState.NewFunction(GameFunction(GameString, "gmLegendFace", GmLegendFace, GameScalar));
    GGameState.NewFunction(GameFunction(GameString, "gmLegendId", GmLegendId, GameScalar));
    GGameState.NewNularOp(GameNular(GameScalar, "gmLegendCount", GmLegendCount));
    GGameState.NewFunction(GameFunction(GameArray, "gmLegendInfo", GmLegendInfo, GameScalar));
    GGameState.NewNularOp(GameNular(GameArray, "gmLegendHistory", GmLegendHistory));
    GGameState.NewFunction(GameFunction(GameArray, "gmLegendHistoryEvent", GmLegendHistoryEvent, GameScalar));
    GGameState.NewFunction(GameFunction(GameArray, "gmLegendPos", GmLegendPos, GameScalar));
    GGameState.NewFunction(GameFunction(GameObject, "gmLegendBody", GmLegendBody, GameScalar));
    GGameState.NewFunction(GameFunction(GameObject, "gmLegendVehicle", GmLegendVehicle, GameScalar));
    GGameState.NewFunction(GameFunction(GameBool, "gmLegendDefeated", GmLegendDefeated, GameScalar));
    GGameState.NewFunction(GameFunction(GameArray, "gmLegendMarker", GmLegendMarker, GameScalar));
}
