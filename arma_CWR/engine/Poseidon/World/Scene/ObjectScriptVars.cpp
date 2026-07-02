#include <Poseidon/World/Scene/ObjectScriptVars.hpp>

#include <Poseidon/IO/Serialization/ParamArchive.hpp>

namespace Poseidon
{

void ObjectScriptVars::Set(const char* name, GameValuePar value)
{
    RString source = name;
    source.Lower();
    const GameVariable& var = _vars[source];
    if (VarBankType::NotNull(var))
    {
        if (value.GetNil())
        {
            _vars.Remove(source);
            return;
        }
        const_cast<GameVariable&>(var)._value = value;
    }
    else if (!value.GetNil())
    {
        _vars.Add(GameVariable(source, value));
    }
}

bool ObjectScriptVars::Get(const char* name, GameValue& ret) const
{
    RString source = name;
    source.Lower();
    const GameVariable& var = _vars[source];
    if (VarBankType::IsNull(var))
    {
        return false;
    }
    ret = var._value;
    return true;
}

LSError ObjectScriptVars::Serialize(ParamArchive& ar)
{
    // GameValue (de)serialization resolves GameData factories through the
    // archive params, which must point at the game evaluator — same contract
    // as the GameState "Variables" block (ExpressExt.cpp).
    void* old = ar.GetParams();
    ar.SetParams(&GGameState);
    LSError err = ar.Serialize("Variables", _vars, 1);
    ar.SetParams(old);
    return err;
}

} // namespace Poseidon
