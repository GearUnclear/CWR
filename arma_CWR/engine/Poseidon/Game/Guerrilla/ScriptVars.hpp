#pragma once

// Null-safe readers for the script-owned globals the native Guerrilla services
// observe (GM_COMP_*, gm*).  One shared copy of the idiom
// GuerrillaJournalPages.cpp:72-116 and ZoneRegistry's own side-selection reader
// each grew privately.
//
// GameState::VarGet lowercases the name itself, so the lowercase spelling every
// existing caller uses ("gmwarlevel", "gm_comp_names") is convention, not a
// requirement; keep it.  An absent global returns the shared GameDataNil
// (GameVoid) singleton (Evaluator/express.cpp:2481, GameState::VarGet), so every
// reader gates on GetType(), never on a null check.
//
// NEVER hold a GameValue across frames: the whole bank is rebuilt by the
// savegame path (WorldImpl.cpp GGameState serialization) and can be dropped by
// VarDelete.  Read, copy out, forget.
//
// Header-only on purpose: a .cpp placed directly under Game/Guerrilla/ has to be
// classified in Showcase.Abel/human/coverage.json or the human-suite contract
// test goes red, and none of this needs a translation unit.

#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

#include <Evaluator/express.hpp> // GameState / GameValue (GGameState, VarGet)

namespace Poseidon
{

class Object;

namespace Guerrilla
{
namespace ScriptVars
{

// A scalar global, `fallback` when absent or of another type.  `found`, when
// given, reports whether the global was there AND a scalar.
inline float Scalar(const char* name, float fallback = 0, bool* found = nullptr)
{
    if (found)
    {
        *found = false;
    }
    if (!name)
    {
        return fallback;
    }
    GameValue value = GGameState.VarGet(name);
    if (value.GetType() != GameScalar)
    {
        return fallback;
    }
    if (found)
    {
        *found = true;
    }
    return (float)value;
}

// A boolean global; false when absent or of another type.
inline bool Bool(const char* name)
{
    if (!name)
    {
        return false;
    }
    GameValue value = GGameState.VarGet(name);
    return value.GetType() == GameBool && (GameBoolType)value;
}

// A string global; "" when absent or of another type.
inline RString String(const char* name)
{
    if (!name)
    {
        return RString();
    }
    GameValue value = GGameState.VarGet(name);
    if (value.GetType() != GameString)
    {
        return RString();
    }
    return RString((GameStringType)value);
}

// `out` is cleared first; false when absent or not a GameArray.  Elements of the
// wrong type read back as ""/0/false and still occupy their slot, so the
// caller's index correspondence across the GM_COMP_* arrays holds.
inline bool StringArray(const char* name, AutoArray<RString>& out)
{
    out.Clear();
    if (!name)
    {
        return false;
    }
    GameValue value = GGameState.VarGet(name);
    if (value.GetType() != GameArray)
    {
        return false;
    }
    const GameArrayType& array = value;
    out.Realloc(array.Size());
    for (int i = 0; i < array.Size(); i++)
    {
        out.Add(array[i].GetType() == GameString ? RString((GameStringType)array[i]) : RString());
    }
    return true;
}

inline bool ScalarArray(const char* name, AutoArray<float>& out)
{
    out.Clear();
    if (!name)
    {
        return false;
    }
    GameValue value = GGameState.VarGet(name);
    if (value.GetType() != GameArray)
    {
        return false;
    }
    const GameArrayType& array = value;
    out.Realloc(array.Size());
    for (int i = 0; i < array.Size(); i++)
    {
        out.Add(array[i].GetType() == GameScalar ? (float)array[i] : 0.0f);
    }
    return true;
}

inline bool BoolArray(const char* name, AutoArray<bool>& out)
{
    out.Clear();
    if (!name)
    {
        return false;
    }
    GameValue value = GGameState.VarGet(name);
    if (value.GetType() != GameArray)
    {
        return false;
    }
    const GameArrayType& array = value;
    out.Realloc(array.Size());
    for (int i = 0; i < array.Size(); i++)
    {
        out.Add(array[i].GetType() == GameBool ? (bool)(GameBoolType)array[i] : false);
    }
    return true;
}

// Element count of an array global; 0 when absent or not an array.
inline int ArraySize(const char* name)
{
    if (!name)
    {
        return 0;
    }
    GameValue value = GGameState.VarGet(name);
    if (value.GetType() != GameArray)
    {
        return 0;
    }
    const GameArrayType& array = value;
    return array.Size();
}

// One element of an OBJECT array; null when absent / not an array / out of range
// / not a GameObject / objNull.  DECLARED here, DEFINED in LegendRegistry.cpp
// (it needs Game/Commands/GameStateExt.hpp plus World/Scene/Object.hpp, neither
// of which this header wants to drag in) with the GameStateExtObj.cpp idiom
//   static_cast<GameDataObject*>(v.GetData())->GetObject()
// after checking v.GetType() == GameObject.
Object* ObjectAt(const char* name, int index);

} // namespace ScriptVars
} // namespace Guerrilla
} // namespace Poseidon
