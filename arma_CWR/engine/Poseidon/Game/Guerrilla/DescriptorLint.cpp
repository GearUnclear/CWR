#include <Poseidon/Game/Guerrilla/DescriptorLint.hpp>

#include <Poseidon/Foundation/Framework/DebugLog.hpp>

#include <Poseidon/IO/ParamFile/ParamFile.hpp>

#include <math.h>
#include <stdarg.h>
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

// ---------------------------------------------------------------------------
// per-template base audit (issue #76)
// ---------------------------------------------------------------------------

namespace
{

std::string FmtS(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    ::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return std::string(buf);
}

bool TypeIs(const RString& type, const char* want)
{
    return stricmp((const char*)type, want) == 0;
}

std::string SeedWhy(const SeedBasesSetting& s)
{
    if (s.explicitKey)
    {
        return "set by the template";
    }
    const int authored = s.authoredAirfields + s.authoredSeaports;
    if (authored == 0)
    {
        return "default: the template authors no AIRFIELD or SEAPORT";
    }
    return FmtS("default: the template authors %d AIRFIELD / %d SEAPORT", s.authoredAirfields, s.authoredSeaports);
}

} // namespace

void ReadTemplateZones(const ParamEntry* cfgGuerrillaZones, std::vector<DetectAuthoredZone>& out)
{
    const ParamEntry* zones = cfgGuerrillaZones ? cfgGuerrillaZones->FindEntry("Zones") : nullptr;
    if (!zones)
    {
        return;
    }
    for (int i = 0; i < zones->GetEntryCount(); i++)
    {
        const ParamEntry& e = zones->GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        DetectAuthoredZone z;
        z.name = e.ReadValue("name", RString(e.GetName()));
        z.type = e.ReadValue("type", RString("OUTPOST"));
        const ParamEntry* pos = e.FindEntry("position");
        if (pos && pos->IsArray() && pos->GetSize() >= 2)
        {
            z.x = (*pos)[0];
            z.z = (*pos)[1];
        }
        out.push_back(z);
    }
}

SeedBasesSetting ResolveSeedBases(const ParamEntry* cfgGuerrillaZones, const std::vector<DetectAuthoredZone>& authored)
{
    SeedBasesSetting s;
    for (const DetectAuthoredZone& z : authored)
    {
        if (TypeIs(z.type, "AIRFIELD"))
        {
            s.authoredAirfields++;
        }
        else if (TypeIs(z.type, "SEAPORT"))
        {
            s.authoredSeaports++;
        }
    }
    if (cfgGuerrillaZones && cfgGuerrillaZones->FindEntry("seedBases"))
    {
        s.explicitKey = true;
        s.seed = cfgGuerrillaZones->ReadValue("seedBases", 0.0f) != 0.0f;
    }
    else
    {
        s.seed = s.authoredAirfields + s.authoredSeaports == 0;
    }
    return s;
}

bool ReadPlayRegion(const ParamEntry* cfgGuerrillaZones, float& minX, float& minZ, float& maxX, float& maxZ)
{
    const ParamEntry* region = cfgGuerrillaZones ? cfgGuerrillaZones->FindEntry("playRegion") : nullptr;
    if (!region || !region->IsArray() || region->GetSize() < 4)
    {
        return false;
    }
    const float v[4] = {(*region)[0], (*region)[1], (*region)[2], (*region)[3]};
    if (v[0] >= v[2] || v[1] >= v[3])
    {
        return false;
    }
    minX = v[0];
    minZ = v[1];
    maxX = v[2];
    maxZ = v[3];
    return true;
}

bool ResolvePlayRegion(const ParamEntry* cfgGuerrillaZones, const std::vector<DetectAuthoredZone>& authored,
                       float& minX, float& minZ, float& maxX, float& maxZ, bool* isDefault, float pad)
{
    if (ReadPlayRegion(cfgGuerrillaZones, minX, minZ, maxX, maxZ))
    {
        if (isDefault)
        {
            *isDefault = false;
        }
        return true;
    }
    if (authored.empty())
    {
        return false;
    }
    float x0 = authored[0].x, x1 = authored[0].x, z0 = authored[0].z, z1 = authored[0].z;
    for (const DetectAuthoredZone& z : authored)
    {
        x0 = z.x < x0 ? z.x : x0;
        x1 = z.x > x1 ? z.x : x1;
        z0 = z.z < z0 ? z.z : z0;
        z1 = z.z > z1 ? z.z : z1;
    }
    minX = x0 - pad;
    minZ = z0 - pad;
    maxX = x1 + pad;
    maxZ = z1 + pad;
    if (isDefault)
    {
        *isDefault = true;
    }
    return true;
}

std::vector<BaseLintRow> LintTemplateBases(const BaseDetectResult& detected,
                                           const std::vector<DetectAuthoredZone>& authored,
                                           const SeedBasesSetting& seedBases)
{
    std::vector<BaseLintRow> rows;
    for (const BaseCandidate& c : detected.candidates)
    {
        BaseLintRow r;
        r.type = (const char*)c.type;
        r.name = (const char*)c.name;
        r.x = c.x;
        r.z = c.z;
        r.confidence = c.confidence;
        r.status = BaseStatusName(c.status);
        const std::string head =
            FmtS("%s \"%s\" at %.0f %.0f conf=%.2f", r.type.c_str(), r.name.c_str(), c.x, c.z, c.confidence);
        if (c.status == BaseStatus::Covered)
        {
            r.verdict = "COVERED";
            r.coveredBy = (const char*)c.coveredBy;
            for (const DetectAuthoredZone& a : authored)
            {
                if (stricmp((const char*)a.name, (const char*)c.coveredBy) == 0)
                {
                    r.coveredByType = (const char*)a.type;
                    r.coverDist = sqrtf((a.x - c.x) * (a.x - c.x) + (a.z - c.z) * (a.z - c.z));
                    break;
                }
            }
            r.line = "COVERED " + head + FmtS(" by \"%s\"", r.coveredBy.c_str());
            if (r.coverDist >= 0)
            {
                r.line += FmtS(" (%s, %.0f m)", r.coveredByType.c_str(), r.coverDist);
            }
        }
        else if (c.status == BaseStatus::OutsideRegion || c.status == BaseStatus::LowConfidence)
        {
            r.verdict = "SKIPPED";
            r.line = "SKIPPED " + head + " (" + r.status + "): never seeded";
        }
        else if (TypeIs(c.type, "OUTPOST"))
        {
            r.verdict = "MISSING";
            r.line = "MISSING " + head + ": offline candidate, never seeded (the outpost ladder is the author's)";
        }
        else
        {
            r.verdict = "MISSING";
            r.seeded = seedBases.seed;
            r.line = "MISSING " + head +
                     (r.seeded ? ": the runtime seeds it (seedBases=1, " : ": NOT seeded (seedBases=0, ") +
                     SeedWhy(seedBases) + ")";
        }
        rows.push_back(r);
    }
    return rows;
}

std::string FormatSeedBasesRow(const char* label, const SeedBasesSetting& seedBases)
{
    return FmtS("BASES %s seedBases=%d (%s)", label ? label : "", seedBases.seed ? 1 : 0, SeedWhy(seedBases).c_str());
}

} // namespace Poseidon::Guerrilla
