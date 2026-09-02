#include <Poseidon/Game/Guerrilla/RosterProbe.hpp>

#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // GetShapeName

#include <ctype.h>
#include <string.h>
#include <string>

namespace Poseidon::Guerrilla
{

namespace
{

// The script commands whose argument is a file path.
const char* const kScriptCommands[] = {"execVM", "preprocessFileLineNumbers", "preprocessFile", "loadFile", "exec"};

bool IsIdentChar(char c)
{
    return isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

// Collect the double-quoted argument of every path-taking script command in one
// EH body.
void ScanScriptBody(const char* text, std::vector<RString>& out)
{
    if (!text)
    {
        return;
    }
    const size_t len = ::strlen(text);
    for (size_t i = 0; i < len; ++i)
    {
        for (const char* cmd : kScriptCommands)
        {
            size_t cl = ::strlen(cmd);
            if (i + cl > len || strnicmp(text + i, cmd, static_cast<int>(cl)) != 0)
            {
                continue;
            }
            // whole word: a `preprocessFileLineNumbers` must not match as `preprocessFile`
            if (i > 0 && IsIdentChar(text[i - 1]))
            {
                continue;
            }
            if (i + cl < len && IsIdentChar(text[i + cl]))
            {
                continue;
            }
            size_t j = i + cl;
            while (j < len && (text[j] == ' ' || text[j] == '\t'))
            {
                ++j;
            }
            if (j >= len || text[j] != '"')
            {
                break; // a brace-delimited or computed argument: not statically a path
            }
            size_t start = ++j;
            while (j < len && text[j] != '"')
            {
                ++j;
            }
            if (j <= start)
            {
                break;
            }
            out.push_back(RString(std::string(text + start, j - start).c_str()));
            i = j;
            break;
        }
    }
}

const ParamClass* AsClass(const ParamEntry* entry)
{
    return (entry && entry->IsClass()) ? entry->GetClassInterface() : nullptr;
}

} // namespace

bool RosterWildcardMatch(const char* pattern, const char* text)
{
    const char* star = nullptr;
    const char* mark = nullptr;
    while (*text)
    {
        char p = static_cast<char>(tolower(static_cast<unsigned char>(*pattern)));
        char t = static_cast<char>(tolower(static_cast<unsigned char>(*text)));
        if (p == '?' || p == t)
        {
            ++pattern;
            ++text;
        }
        else if (*pattern == '*')
        {
            star = pattern++;
            mark = text;
        }
        else if (star)
        {
            pattern = star + 1;
            text = ++mark;
        }
        else
        {
            return false;
        }
    }
    while (*pattern == '*')
    {
        ++pattern;
    }
    return *pattern == 0;
}

std::vector<RString> CollectScriptReferences(const ParamEntry& cls)
{
    std::vector<RString> out;
    const ParamClass* c = AsClass(&cls);
    if (!c)
    {
        return out;
    }
    const ParamEntry* handlers = c->FindEntry("EventHandlers");
    const ParamClass* hc = AsClass(handlers);
    if (!hc)
    {
        return out;
    }
    for (int i = 0; i < hc->GetEntryCount(); i++)
    {
        const ParamEntry& e = hc->GetEntry(i);
        if (e.IsClass() || e.IsArray())
        {
            continue;
        }
        ScanScriptBody(e.GetValue(), out);
    }
    return out;
}

bool IsProbeCandidate(const ParamEntry* vehiclesCfg, const ParamEntry& entry)
{
    if (!vehiclesCfg || !entry.IsClass())
    {
        return false;
    }
    const ParamClass* cls = entry.GetClassInterface();
    if (!cls)
    {
        return false;
    }
    // Man and LandVehicle both derive from Land in the stock hierarchy, so Land
    // alone would do; naming all three keeps the gate working on a package that
    // flattens or renames part of the chain.
    static const char* const kBases[] = {"Man", "Land", "LandVehicle"};
    bool derived = false;
    for (const char* baseName : kBases)
    {
        const ParamClass* base = AsClass(vehiclesCfg->FindEntry(baseName));
        if (!base || base == cls)
        {
            continue;
        }
        if (cls->IsDerivedFrom(*base))
        {
            derived = true;
            break;
        }
    }
    if (!derived)
    {
        return false;
    }
    // scope >= 2 is "public": the value the editor and createVehicle require.
    // scope 1 is script-only and scope 0 abstract, and neither is a shipped body.
    return entry.ReadValue("scope", 0) >= 2;
}

RosterProbeResult ProbeRosterClass(const ParamEntry& cls, const std::function<bool(RString)>& fileExists)
{
    RosterProbeResult r;
    r.className = cls.GetName();
    r.owner = cls.GetOwner();
    r.model = cls.ReadValue("model", RString());
    r.scriptRefs = CollectScriptReferences(cls);

    if (r.model.GetLength() == 0)
    {
        r.reason = "the class authors no model";
        return r;
    }
    RString shape = GetShapeName(r.model);
    if (!fileExists || !fileExists(shape))
    {
        r.reason = RString("model shape '") + shape + RString("' is not in the loaded data package");
        return r;
    }
    for (const RString& ref : r.scriptRefs)
    {
        // Config paths are authored with a leading backslash (`\Mod\x.sqs`); the
        // bank filesystem is rooted, so strip it before asking.
        const char* p = ref;
        while (*p == '\\' || *p == '/')
        {
            ++p;
        }
        if (!fileExists(RString(p)))
        {
            r.reason = RString("EventHandlers script '") + ref + RString("' is not in the loaded data package");
            return r;
        }
    }
    r.ok = true;
    return r;
}

std::vector<RosterProbeResult> ProbeRoster(const ParamEntry* vehiclesCfg, const RosterProbeOptions& options,
                                           const std::function<bool(RString)>& fileExists)
{
    std::vector<RosterProbeResult> out;
    if (!vehiclesCfg)
    {
        return out;
    }
    for (int i = 0; i < vehiclesCfg->GetEntryCount(); i++)
    {
        const ParamEntry& e = vehiclesCfg->GetEntry(i);
        if (!IsProbeCandidate(vehiclesCfg, e))
        {
            continue;
        }
        RString owner = e.GetOwner();
        bool match = false;
        if (options.owners.empty() && options.classNames.empty())
        {
            // No selection at all: every class some addon shipped. Base-game
            // classes (empty owner) are not this mod's to answer for.
            match = owner.GetLength() > 0;
        }
        for (int k = 0; k < static_cast<int>(options.owners.size()) && !match; k++)
        {
            match = stricmp(owner, options.owners[k]) == 0;
        }
        for (int k = 0; k < static_cast<int>(options.classNames.size()) && !match; k++)
        {
            match = stricmp(e.GetName(), options.classNames[k]) == 0;
        }
        if (!match)
        {
            continue;
        }
        if (options.filter.GetLength() > 0 && !RosterWildcardMatch(options.filter, e.GetName()))
        {
            continue;
        }
        out.push_back(ProbeRosterClass(e, fileExists));
    }
    return out;
}

} // namespace Poseidon::Guerrilla
