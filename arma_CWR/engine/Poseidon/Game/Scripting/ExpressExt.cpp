#include <Evaluator/express.hpp>

#include <Poseidon/Core/BuildInfo.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/Core/Application.hpp>
#include <Poseidon/Foundation/Platform/AppConfig.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/UI/Locale/StringtableExt.hpp>
#include <Poseidon/Foundation/Framework/AppFrame.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

#include <cstring>
#include <cstdio>

using Poseidon::LocalizeString;

namespace Poseidon
{
} // namespace Poseidon

class ParamArchiveFunctions : public ArchiveFunctions
{
  public:
    LSError Serialize(ParamArchive& ar, const RStringB& name, bool& value, int minVersion) override
    {
        return ar.Serialize(name, value, minVersion);
    }
    LSError Serialize(ParamArchive& ar, const RStringB& name, bool& value, int minVersion, bool defValue) override
    {
        return ar.Serialize(name, value, minVersion, defValue);
    }
    LSError Serialize(ParamArchive& ar, const RStringB& name, GameType& value, int minVersion) override
    {
        return ar.Serialize(name, value, minVersion);
    }
    LSError Serialize(ParamArchive& ar, const RStringB& name, float& value, int minVersion) override
    {
        return ar.Serialize(name, value, minVersion);
    }
    LSError Serialize(ParamArchive& ar, const RStringB& name, RString& value, int minVersion) override
    {
        return ar.Serialize(name, value, minVersion);
    }
    LSError Serialize(ParamArchive& ar, const RStringB& name, AutoArray<GameValue>& value, int minVersion) override
    {
        return ar.Serialize(name, value, minVersion);
    }
    LSError Serialize(ParamArchive& ar, const RStringB& name, Ref<GameData>& value, int minVersion) override
    {
        return ar.Serialize(name, value, minVersion);
    }
    LSError Serialize(ParamArchive& ar, GameState* value) override;
    GameData* CreateGameData(ParamArchive& ar, GameType type) override;
    bool IsSaving(ParamArchive& ar) override { return ar.IsSaving(); }

    // Registration happens via InitScriptingDefaults(), called from
    // Poseidon::InitDefaults() at app startup.
} GParamArchiveFunctions;

// The archive's ItemN order is the SAVE bank's hash traversal order. Loading
// can grow/rehash that bank, and world restoration can insert more globals
// between passes. PassSecond must resolve each saved name, not traverse the
// current buckets and pair unrelated GameData types with the same ItemN.
static LSError RestoreVariableReferences(ParamArchive& ar, GameState* state)
{
    ParamArchive variables;
    if (!ar.OpenSubclass("Variables", variables))
    {
        ar.OnError(LSNoEntry, "Variables");
        return LSNoEntry;
    }
    ParamArchive header = variables;
    header.FirstPass(); // Read metadata only; scalar reads skip PassSecond.
    int count = 0;
    PARAM_CHECK(header.Serialize("items", count, 1))
    for (int i = 0; i < count; ++i)
    {
        char itemName[32];
        std::snprintf(itemName, sizeof(itemName), "Item%d", i);
        ParamArchive item;
        if (!variables.OpenSubclass(itemName, item, true))
        {
            variables.OnError(LSNoEntry, itemName);
            return LSNoEntry;
        }
        ParamArchive key = item;
        key.FirstPass();
        RString name;
        PARAM_CHECK(key.Serialize("name", name, 1))
        GameVariable& variable = state->GetVariables().Set(name);
        if (VarBankType::IsNull(variable))
        {
            item.OnError(LSNoEntry, name);
            return LSNoEntry;
        }
        PARAM_CHECK(variable.Serialize(item))
        variables.CloseSubclass(item);
    }
    ar.CloseSubclass(variables);
    return LSOK;
}

LSError ParamArchiveFunctions::Serialize(ParamArchive& ar, GameState* value)
{
    void* old = ar.GetParams();
    ar.SetParams(value);
    const LSError result = ar.IsLoading() && ar.GetPass() == ParamArchive::PassSecond
                               ? RestoreVariableReferences(ar, value)
                               : ar.Serialize("Variables", value->GetVariables(), 1);
    ar.SetParams(old);
    return result;
}

GameData* ParamArchiveFunctions::CreateGameData(ParamArchive& ar, GameType type)
{
    GameState* context = reinterpret_cast<GameState*>(ar.GetParams());
    return context->CreateGameData(type);
}

class GameStateStringtableInfoFunctions : public GameStateInfoFunctions
{
  public:
    const char* GetErrorString(EvalError error) const override;
    RString GetTypeName(GameType type) const override;
    void DisplayErrorMessage(const char* position, const char* error) const override;

    void DisplayDebugMessage(const char* content, int timeMs) const override;

    // Registration via InitScriptingDefaults().
} GGameStateStringtableInfoFunctions;

const char* GameStateStringtableInfoFunctions::GetErrorString(EvalError error) const
{
    return LocalizeString(IDS_EVAL_GEN + error - EvalGen);
}

static void CatType(RString& a, RString b)
{
    if (a.GetLength() <= 0)
    {
        a = b;
    }
    else if (b.GetLength() <= 0)
    {
        ;
    }
    else
    {
        a = a + RString(",") + b;
    }
}

RString GameStateStringtableInfoFunctions::GetTypeName(GameType type) const
{
    if (type == GameVoid)
    {
        return LocalizeString(IDS_EVAL_TYPEANY);
    }
    RString ret = "";
    if (type & GameScalar)
    {
        CatType(ret, LocalizeString(IDS_EVAL_TYPESCALAR));
    }
    if (type & GameArray)
    {
        CatType(ret, LocalizeString(IDS_EVAL_TYPEARRAY));
    }
    if (type & GameBool)
    {
        CatType(ret, LocalizeString(IDS_EVAL_TYPEBOOL));
    }
    if (type & GameString)
    {
        CatType(ret, LocalizeString(IDS_EVAL_TYPESTRING));
    }
    if (type & GameNothing)
    {
        CatType(ret, LocalizeString(IDS_EVAL_TYPENOTHING));
    }
    if (type & GameCode)
    {
        CatType(ret, "code");
    }
    if (type & GameIf)
    {
        CatType(ret, "if");
    }
    if (type & GameWhile)
    {
        CatType(ret, "while");
    }
    return ret;
}

void GameStateStringtableInfoFunctions::DisplayErrorMessage(const char* position, const char* error) const
{
    const bool showInGame = std::strcmp(Poseidon::BuildInfo::BuildType, "RelWithDebInfo") == 0 &&
                            Poseidon::Foundation::AppConfig::Instance().DevMode();
    extern bool AutoTest;
    if (AutoTest)
    {
        // Test mode: surface every SQF/SQS error as a hard failure so the
        // test reports it instead of silently timing out.  Logging at
        // ERROR + requesting a graceful close lets GameApplication run
        // its shutdown sequence (incl. test-mission cleanup, so we don't
        // pollute packages/<pkg>/Missions/).
        LOG_ERROR(Script, "Script error at '{}': {} (test mode — aborting)", position, error);
        if (Poseidon::GApp)
        {
            if (Poseidon::GApp->m_exitCode == 0)
                Poseidon::GApp->m_exitCode = 2;
            Poseidon::GApp->m_closeRequest = true;
        }
    }
    else if (Poseidon::Foundation::AppConfig::Instance().Strict())
    {
        // --strict: a script error is fatal. Logging at ERROR latches the strict
        // trip; GameApplication's main-loop poll turns it into a clean non-zero exit.
        LOG_ERROR(Script, "Script error at '{}': {} (strict — aborting)", position, error);
    }
    else
    {
        LOG_WARN(Script, "Script error at '{}': {}", position, error);
    }
    if (showInGame)
        GlobalShowMessage(10000, "'%s': Error %s", position, error);
}

void GameStateStringtableInfoFunctions::DisplayDebugMessage(const char* content, int timeMs) const
{
    GlobalShowMessage(timeMs, "%s", content);
}

// Explicit registration — call once from program startup (typically
// via Poseidon::InitDefaults()).  Idempotent and safe to call
// multiple times.
void InitScriptingDefaults()
{
    GameState::SetDefaultArchiveFunctions(&GParamArchiveFunctions);
    GameState::SetDefaultInfoFunctions(&GGameStateStringtableInfoFunctions);
}
