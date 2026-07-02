#pragma once

#include <Evaluator/express.hpp>

namespace Poseidon
{

// Per-object script variable bank backing the setVariable/getVariable script
// commands. Allocated lazily by Object::ScriptVars(true) on first setVariable,
// so objects that never carry variables pay only a null pointer. Serialized
// with the owning Object from save version 14 (see Object::Serialize).
class ObjectScriptVars
{
    VarBankType _vars;

  public:
    //! set (or overwrite) a variable; a nil value deletes it
    void Set(const char* name, GameValuePar value);
    //! fetch a variable; returns false when undefined
    bool Get(const char* name, GameValue& ret) const;

    bool IsEmpty() const { return _vars.NItems() == 0; }

    LSError Serialize(ParamArchive& ar);
    static ObjectScriptVars* CreateObject(ParamArchive& ar) { return new ObjectScriptVars(); }
};

} // namespace Poseidon
