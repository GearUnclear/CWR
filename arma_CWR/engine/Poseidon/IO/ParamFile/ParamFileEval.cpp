
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>

#include <Poseidon/IO/Streams/SerializeBin.hpp>
#include <Evaluator/express.hpp>
#include <Poseidon/Foundation/Common/FltOpts.hpp>
#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

#include <mutex>
#include <set>
#include <string>

namespace Poseidon
{
// A config value only reaches the evaluator when it is not a plain number
// (ParamRawValue::GetInt/GetFloat try ScanInt/ScanFloat first), so a nil result
// means an identifier that resolved to nothing - classically `scope = public;`
// in an OFP-era config missing its `#define public 2` header. The stock engine
// reads that as 0 in total silence (the evaluator's own report goes through
// RptF, which compiles away) and every class in the file quietly turns
// abstract. Keep the value - 0 is what two decades of content expects - but say
// what happened. Once per distinct expression: legacy mods repeat the same
// unresolved identifier across hundreds of classes (@LoBo alone re-reads
// 'VSoft' 130+ times in one mount), and one line per spelling is what a human
// needs to find the broken config.
static void WarnIfConfigValueUnresolved(const GameValue& result, const char* expr)
{
    if (!result.GetNil())
    {
        return;
    }
    // Deliberately leaked so there is no exit-time destructor (same reason the
    // GGameStateEvaluatorFunctions singleton below suppresses that warning).
    static std::mutex* seenLock = new std::mutex();
    static std::set<std::string>* seen = new std::set<std::string>();
    {
        std::lock_guard<std::mutex> guard(*seenLock);
        if (!seen->insert(expr).second)
        {
            return;
        }
    }
    LOG_WARN(Core,
             "Config value '{}' does not evaluate to a number (undefined identifier? OFP-era configs need e.g. "
             "'#define public 2' in the same file); reading it as 0",
             expr);
}
// EvaluatorFunctions implementation backed by the game state. The *Internal
// variants skip the game-state context init/deinit that the plain variants do.
class GameStateEvaluatorFunctions : public EvaluatorFunctions
{
  public:
    GameVarSpace* CreateVariables() override;
    void DeleteVariables(GameVarSpace* vars) override;
    void InitEvaluator(GameVarSpace* vars) override;
    void DoneEvaluator() override;
    GameVarSpace* LoadVariables(SerializeBinStream& f) override;
    void SaveVariables(SerializeBinStream& f, GameVarSpace* vars) override;
    float EvaluateFloat(const char* expr, GameVarSpace* vars) override;
    float EvaluateFloatInternal(const char* expr) override;
    RString EvaluateStringInternal(const char* expr) override;
    void ExecuteInternal(const char* expr) override;
    void VarSetFloatInternal(const char* name, float value, bool readOnly, bool forceLocal) override;

    GameStateEvaluatorFunctions() { ParamFile::SetDefaultEvalFunctions(this); }
};

// Meyers singleton - no global constructor
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
static GameStateEvaluatorFunctions& GGameStateEvaluatorFunctions()
{
    static GameStateEvaluatorFunctions instance;
    return instance;
}
#pragma clang diagnostic pop

void InitParamFileEvaluator()
{
    // Force initialization of singleton
    (void)GGameStateEvaluatorFunctions();
}

GameVarSpace* GameStateEvaluatorFunctions::CreateVariables()
{
    return new GameVarSpace();
}

void GameStateEvaluatorFunctions::DeleteVariables(GameVarSpace* vars)
{
    delete vars;
}

// Must be paired with a DoneEvaluator() call.
void GameStateEvaluatorFunctions::InitEvaluator(GameVarSpace* vars)
{
    GGameState.BeginContext(vars);
}

void GameStateEvaluatorFunctions::DoneEvaluator()
{
    GGameState.EndContext();
}

GameVarSpace* GameStateEvaluatorFunctions::LoadVariables(SerializeBinStream& f)
{
    GameVarSpace* vars = new GameVarSpace();
    int n;
    f.Transfer(n);
    for (int i = 0; i < n; i++)
    {
        RString name;
        int value;
        f.Transfer(name);
        f.Transfer(value);
        GameVariable var(name, (float)value, true);
        vars->_vars.Add(var);
    }
    return vars;
}

void GameStateEvaluatorFunctions::SaveVariables(SerializeBinStream& f, GameVarSpace* vars)
{
    int n = vars ? vars->_vars.NItems() : 0;
    f.Transfer(n);
    if (n > 0)
    {
        for (int i = 0; i < vars->_vars.NTables(); i++)
        {
            AutoArray<GameVariable>& table = vars->_vars.GetTable(i);
            for (int j = 0; j < table.Size(); j++)
            {
                RString name = table[j]._name;
                int value = toInt((float)table[j]._value);
                f.Transfer(name);
                f.Transfer(value);
            }
        }
    }
}

float GameStateEvaluatorFunctions::EvaluateFloat(const char* expr, GameVarSpace* vars)
{
    GGameState.BeginContext(vars);
    GameValue result = GGameState.Evaluate(expr);
    GGameState.EndContext();
    WarnIfConfigValueUnresolved(result, expr);
    return result;
}

float GameStateEvaluatorFunctions::EvaluateFloatInternal(const char* expr)
{
    GameValue result = GGameState.Evaluate(expr);
    WarnIfConfigValueUnresolved(result, expr);
    return result;
}

RString GameStateEvaluatorFunctions::EvaluateStringInternal(const char* expr)
{
    return GGameState.Evaluate(expr).GetText();
}

void GameStateEvaluatorFunctions::ExecuteInternal(const char* expr)
{
    GGameState.Execute(expr);
}

void GameStateEvaluatorFunctions::VarSetFloatInternal(const char* name, float value, bool readOnly, bool forceLocal)
{
    GGameState.VarSet(name, GameValue(value), readOnly, forceLocal);
}

} // namespace Poseidon
