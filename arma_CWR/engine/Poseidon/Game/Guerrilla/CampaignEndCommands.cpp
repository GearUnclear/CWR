// Script command surface for the Guerrilla campaign end state (issue #79
// B3).  Registered from its own INIT_MODULE at stage 3 so GGameState.Init()
// (GameStateExt, stage 2) has already run - same pattern as
// JournalCommands.cpp.
//
//   gmCampaignWon               -> bool, the saved win latch
//   gmCampaignEnd               -> [won, endPending, endIssued, endClock,
//                                   held, total, wonDay, lastTaken]
//                                  held / total count the campaign's enemy
//                                  military zones (the roster); total is 0
//                                  until the roster is taken on the first
//                                  active tick
//   gmCampaignEndDelay <secs>   sets the seconds between the latch and the
//                                  end request; a pending clock restarts
//                                  from the new value (tests hold the end
//                                  with a large value)

#include <Poseidon/Game/Guerrilla/CampaignEnd.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
// Referenced from CampaignEnd.cpp to keep this TU (only content besides
// this: file-static commands + module registration) in the link.
void EnsureCampaignEndCommandsLinked() {}
} // namespace Poseidon::Guerrilla

static GameValue GmCampaignWon(const GameState* /*state*/)
{
    return CampaignEnd::Instance().Won();
}

static GameValue GmCampaignEnd(const GameState* state)
{
    const CampaignEnd& end = CampaignEnd::Instance();
    const ZoneRegistry& registry = ZoneRegistry::Instance();
    GameValue value = state->CreateGameValue(GameArray);
    GameArrayType& array = value;
    array.Resize(8);
    array[0] = GameValue(end.Won());
    array[1] = GameValue(end.EndPending());
    array[2] = GameValue(end.EndIssued());
    array[3] = end.EndClock();
    array[4] = (float)end.RosterHeld(registry);
    array[5] = (float)end.RosterSize();
    array[6] = (float)end.WonDay();
    array[7] = GameStringType(end.LastTaken());
    return value;
}

static GameValue GmCampaignEndDelay(const GameState* /*state*/, GameValuePar oper1)
{
    CampaignEnd::Instance().SetEndDelay((float)oper1);
    return NOTHING;
}

INIT_MODULE(GuerrillaCampaignEnd, 3)
{
    GGameState.NewNularOp(GameNular(GameBool, "gmCampaignWon", GmCampaignWon));
    GGameState.NewNularOp(GameNular(GameArray, "gmCampaignEnd", GmCampaignEnd));
    GGameState.NewFunction(GameFunction(GameNothing, "gmCampaignEndDelay", GmCampaignEndDelay, GameScalar));
}
