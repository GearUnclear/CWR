#include <Poseidon/Game/Guerrilla/DescriptorLint.hpp>

#include <Poseidon/Foundation/Framework/DebugLog.hpp>

#include <stdio.h>
#include <string.h>

namespace Poseidon::Guerrilla
{

namespace
{

RString IndexedKey(const char* base, int index)
{
    char buf[96];
    ::snprintf(buf, sizeof(buf), "%s[%d]", base, index);
    return RString(buf);
}

void Add(std::vector<LintFinding>& out, RString key, RString value, RString substitute, LintOutcome outcome)
{
    LintFinding f;
    f.key = key;
    f.value = value;
    f.substitute = substitute;
    f.outcome = outcome;
    out.push_back(f);
}

// tiers / civTiers / the role ladders: the pass keeps the array length and
// rewrites entries in place, or clears the whole array when nothing resolved.
void DiffFixedArray(std::vector<LintFinding>& out, const char* name, const AutoArray<RString>& raw,
                    const AutoArray<RString>& resolved)
{
    for (int i = 0; i < raw.Size(); i++)
    {
        if (raw[i].GetLength() == 0)
        {
            continue; // an authored blank is a deliberate "this rung fields no such role"
        }
        RString key = IndexedKey(name, i);
        if (i >= resolved.Size())
        {
            Add(out, key, raw[i], RString(), LintOutcome::Dropped);
            continue;
        }
        if (stricmp(raw[i], resolved[i]) == 0)
        {
            Add(out, key, raw[i], raw[i], LintOutcome::Ok);
        }
        else if (resolved[i].GetLength() == 0)
        {
            Add(out, key, raw[i], RString(), LintOutcome::Dropped);
        }
        else
        {
            Add(out, key, raw[i], resolved[i], LintOutcome::Substituted);
        }
    }
}

// vehicles / civVehicles: the pass drops unresolvable hulls and compacts, so the
// resolved array is an ordered subsequence of the authored one.
void DiffCompactedArray(std::vector<LintFinding>& out, const char* name, const AutoArray<RString>& raw,
                        const AutoArray<RString>& resolved)
{
    int j = 0;
    for (int i = 0; i < raw.Size(); i++)
    {
        RString key = IndexedKey(name, i);
        if (j < resolved.Size() && stricmp(raw[i], resolved[j]) == 0)
        {
            Add(out, key, raw[i], raw[i], LintOutcome::Ok);
            ++j;
        }
        else
        {
            Add(out, key, raw[i], RString(), LintOutcome::Dropped);
        }
    }
}

} // namespace

const char* ToString(LintOutcome outcome)
{
    switch (outcome)
    {
        case LintOutcome::Ok:
            return "ok";
        case LintOutcome::Substituted:
            return "substituted";
        case LintOutcome::Dropped:
            return "DROPPED";
    }
    return "?";
}

std::vector<LintFinding> DiffFactionRecord(const FactionRecord& raw, const FactionRecord& resolved)
{
    std::vector<LintFinding> out;

    DiffFixedArray(out, "tiers", raw.tiers, resolved.tiers);
    DiffFixedArray(out, "civTiers", raw.civTiers, resolved.civTiers);
    DiffFixedArray(out, "tiersMG", raw.tiersMG, resolved.tiersMG);
    DiffFixedArray(out, "tiersAT", raw.tiersAT, resolved.tiersAT);
    DiffFixedArray(out, "tiersMedic", raw.tiersMedic, resolved.tiersMedic);
    DiffFixedArray(out, "tiersSniper", raw.tiersSniper, resolved.tiersSniper);
    DiffCompactedArray(out, "vehicles", raw.vehicles, resolved.vehicles);
    DiffCompactedArray(out, "civVehicles", raw.civVehicles, resolved.civVehicles);

    // values[] keeps its size and order; only the value strings move.
    for (int i = 0; i < raw.values.Size(); i++)
    {
        const FactionRecord::NamedValue& rv = raw.values[i];
        if (rv.value.GetLength() == 0)
        {
            continue;
        }
        // Match by key rather than index: a caller that built the two records
        // from different loads should still line them up on the key.
        const FactionRecord::NamedValue* res = nullptr;
        if (i < resolved.values.Size() && stricmp(resolved.values[i].key, rv.key) == 0)
        {
            res = &resolved.values[i];
        }
        else
        {
            for (int k = 0; k < resolved.values.Size() && !res; k++)
            {
                if (stricmp(resolved.values[k].key, rv.key) == 0)
                {
                    res = &resolved.values[k];
                }
            }
        }
        if (!res)
        {
            Add(out, rv.key, rv.value, RString(), LintOutcome::Dropped);
        }
        else if (stricmp(res->value, rv.value) == 0)
        {
            Add(out, rv.key, rv.value, rv.value, LintOutcome::Ok);
        }
        else if (res->value.GetLength() == 0)
        {
            Add(out, rv.key, rv.value, RString(), LintOutcome::Dropped);
        }
        else
        {
            Add(out, rv.key, rv.value, res->value, LintOutcome::Substituted);
        }
    }

    return out;
}

bool FactionIsSterile(const FactionRecord& resolved)
{
    for (int i = 0; i < resolved.tiers.Size(); i++)
    {
        if (resolved.tiers[i].GetLength() > 0)
        {
            return false;
        }
    }
    return true;
}

bool RecordingClassProbe::Exists(const char* bank, const char* className) const
{
    bool exists = _inner.Exists(bank, className);
    Query q;
    q.bank = bank ? bank : "";
    q.className = className ? className : "";
    q.exists = exists;
    _queries.push_back(q);
    return exists;
}

bool RecordingClassProbe::Spawnable(const char* className) const
{
    bool ok = _inner.Spawnable(className);
    Query q;
    q.bank = "CfgVehicles";
    q.className = className ? className : "";
    q.exists = ok;
    _queries.push_back(q);
    return ok;
}

std::vector<RString> RecordingClassProbe::Misses() const
{
    std::vector<RString> out;
    for (const Query& q : _queries)
    {
        if (q.exists || q.className.GetLength() == 0)
        {
            continue;
        }
        RString name = q.bank + RString("/") + q.className;
        bool seen = false;
        for (const RString& s : out)
        {
            if (stricmp(s, name) == 0)
            {
                seen = true;
                break;
            }
        }
        if (!seen)
        {
            out.push_back(name);
        }
    }
    return out;
}

} // namespace Poseidon::Guerrilla
