// Script command surface for the Guerrilla undercover layer.  Registered
// from its own INIT_MODULE at stage 3 so GGameState.Init() (GameStateExt,
// stage 2) has already run - same pattern as ZoneRegistryCommands.cpp.

#include <Poseidon/Game/Guerrilla/Undercover.hpp>

#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace Poseidon::Guerrilla
{
// Referenced from Undercover.cpp to keep this TU (only content besides
// this: file-static commands + module registration) in the link.
void EnsureUndercoverCommandsLinked() {}
} // namespace Poseidon::Guerrilla

// gmUndercoverStatus -> 0 clean | 1 suspected (an occupier group holds a
// known TSideUnknown record of the subject) | 2 compromised (a live
// compromised record of the subject or his vehicle exists)
static GameValue GmUndercoverStatus(const GameState* /*state*/)
{
    return (float)UndercoverSystem::Instance().Status();
}

// gmUndercoverWitnesses -> count of occupier groups holding a compromised
// record of the subject (or the subject's current vehicle)
static GameValue GmUndercoverWitnesses(const GameState* /*state*/)
{
    return (float)UndercoverSystem::Instance().WitnessCount();
}

// gmUndercoverForget - clears every compromised record plus the campaign
// first-compromise latch (test aid + future disguise-swap hook)
static GameValue GmUndercoverForget(const GameState* /*state*/)
{
    UndercoverSystem::Instance().ForgetAll();
    return NOTHING;
}

INIT_MODULE(GuerrillaUndercover, 3)
{
    GGameState.NewNularOp(GameNular(GameScalar, "gmUndercoverStatus", GmUndercoverStatus));
    GGameState.NewNularOp(GameNular(GameScalar, "gmUndercoverWitnesses", GmUndercoverWitnesses));
    GGameState.NewNularOp(GameNular(GameNothing, "gmUndercoverForget", GmUndercoverForget));
}
