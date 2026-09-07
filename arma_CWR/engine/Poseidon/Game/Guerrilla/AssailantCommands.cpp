#include <Poseidon/Game/Guerrilla/AssailantSystem.hpp>
#include <Poseidon/Game/Commands/GameStateExt.hpp>
#include <Poseidon/Game/Commands/GameStateExtCommon.hpp>
#include <Poseidon/World/Entities/Infantry/Person.hpp>
#include <Poseidon/Foundation/Modules/Modules.hpp>
#include <Poseidon/Foundation/platform.hpp>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
namespace Poseidon::Guerrilla { void EnsureAssailantCommandsLinked() {} }
static GameValue Side(const GameState*) { return GameStringType(AssailantSystem::Instance().SideName()); }
static GameValue Diagnostic(const GameState*, GameValuePar value)
{ return GameStringType(AssailantSystem::Instance().Diagnostic(AssailantSystem::Mode(GameStringType(value)))); }
static GameValue Count(const GameState*, GameValuePar value)
{ return (float)AssailantSystem::Instance().Count(AssailantSystem::Mode(GameStringType(value))); }
static GameValue Classify(const GameState*, GameValuePar value)
{ return GameStringType(AssailantSystem::Name(AssailantSystem::Instance().Classify(dyn_cast<EntityAI>(GetObject(value))))); }
static GameValue Remove(const GameState*, GameValuePar value)
{ return AssailantSystem::Instance().Remove(dyn_cast<EntityAI>(GetObject(value))); }
static GameValue Eligible(const GameState*, GameValuePar value)
{
    const GameArrayType& a = value;
    if (a.Size() != 2 || a[0].GetType() != GameObject || a[1].GetType() != GameString) return false;
    return AssailantSystem::Instance().Eligible(dyn_cast<Person>(GetObject(a[0])), AssailantSystem::Mode(GameStringType(a[1])));
}
static GameValue Register(const GameState*, GameValuePar value)
{
    const GameArrayType& a = value;
    if (a.Size() != 4 || a[0].GetType() != GameObject || a[1].GetType() != GameString ||
        a[2].GetType() != GameObject || a[3].GetType() != GameString) return false;
    return AssailantSystem::Instance().Register(dyn_cast<Person>(GetObject(a[0])), AssailantSystem::Mode(GameStringType(a[1])),
        dyn_cast<EntityAI>(GetObject(a[2])), GameStringType(a[3]));
}
static GameValue Interval(const GameState*, GameValuePar value)
{ return AssailantSystem::Interval((float)value); }
static GameValue Advance(const GameState*, GameValuePar value)
{
    const GameArrayType& a = value;
    if (a.Size() != 3 || a[0].GetType() != GameScalar || a[1].GetType() != GameScalar || a[2].GetType() != GameBool) return 0.0f;
    return AssailantSystem::Advance((float)a[0], (float)a[1], (bool)a[2]);
}
INIT_MODULE(GuerrillaAssailants, 3)
{
    GGameState.NewNularOp(GameNular(GameString, "gmAssailantSide", Side));
    GGameState.NewFunction(GameFunction(GameString, "gmAssailantDiagnostic", Diagnostic, GameString));
    GGameState.NewFunction(GameFunction(GameScalar, "gmAssailantCount", Count, GameString));
    GGameState.NewFunction(GameFunction(GameString, "gmAssailantClass", Classify, GameObject));
    GGameState.NewFunction(GameFunction(GameBool, "gmAssailantRemove", Remove, GameObject));
    GGameState.NewFunction(GameFunction(GameBool, "gmAssailantEligible", Eligible, GameArray));
    GGameState.NewFunction(GameFunction(GameBool, "gmAssailantRegister", Register, GameArray));
    GGameState.NewFunction(GameFunction(GameScalar, "gmAssailantInterval", Interval, GameScalar));
    GGameState.NewFunction(GameFunction(GameScalar, "gmAssailantAdvance", Advance, GameArray));
}
