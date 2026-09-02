#include <Poseidon/Game/Guerrilla/FactionSources.hpp>

#include <Poseidon/IO/ParamFileExt.hpp> // Pars / ExtParsMission

#include <Poseidon/Foundation/platform.hpp> // stricmp

namespace Poseidon::Guerrilla
{

namespace
{
constexpr const char* kTableName = "CfgGuerrillaFactions";
}

void FactionSources::Clear()
{
    _factions = nullptr;
    _records.clear();
    _merged.Clear();
}

const FactionSources::Record* FactionSources::FindRecord(const char* className) const
{
    if (!className)
    {
        return nullptr;
    }
    for (const Record& r : _records)
    {
        if (stricmp(r.className, className) == 0)
        {
            return &r;
        }
    }
    return nullptr;
}

// Deep-copy one faction class into dst, flattening its base chain (a base
// that the SOURCE table resolves - `class Pack : Base` inside one config).
// ParamClass::Update merges a class's OWN entries, so the chain is applied
// root-first and the class itself last, which makes the derived keys win
// exactly as inheritance would have resolved them.
void FactionSources::CopyClass(ParamClass& dst, const ParamEntry& srcClass, const ParamEntry& srcTable)
{
    std::vector<const ParamClass*> chain;
    const ParamEntry* cur = &srcClass;
    // hop budget is the cycle guard; a config cannot legitimately nest deeper
    for (int hop = 0; cur && cur->IsClass() && hop < 16; hop++)
    {
        const ParamClass* cls = static_cast<const ParamClass*>(cur);
        chain.push_back(cls);
        const char* baseName = cls->GetBaseName();
        if (!baseName || !*baseName)
        {
            break;
        }
        cur = srcTable.FindEntry(baseName);
    }
    ParamClass* copy = dst.AddClass(srcClass.GetName(), true);
    for (int i = (int)chain.size() - 1; i >= 0; i--)
    {
        copy->Update(*chain[i]);
    }
    copy->SetOwner(srcClass.GetOwner(), true);
}

void FactionSources::Build(const ParamEntry* globalCfg, const ParamEntry* islandCfg)
{
    Clear();
    if (!globalCfg && !islandCfg)
    {
        return;
    }
    ParamClass* table = _merged.AddClass(kTableName, true);

    // island first: its classes are the authored primary roster
    if (islandCfg)
    {
        for (int i = 0; i < islandCfg->GetEntryCount(); i++)
        {
            const ParamEntry& e = islandCfg->GetEntry(i);
            if (!e.IsClass() || table->FindEntryNoInheritance(e.GetName()))
            {
                continue; // a duplicate inside one block keeps its first declaration
            }
            CopyClass(*table, e, *islandCfg);
            Record r;
            r.className = e.GetName();
            r.origin = Origin::Island;
            r.overrodeGlobal = globalCfg && globalCfg->FindEntry(e.GetName()) != nullptr;
            r.owner = RString(e.GetOwner());
            _records.push_back(r);
        }
    }
    // then every global class the island did not redeclare
    if (globalCfg)
    {
        for (int i = 0; i < globalCfg->GetEntryCount(); i++)
        {
            const ParamEntry& e = globalCfg->GetEntry(i);
            if (!e.IsClass() || table->FindEntryNoInheritance(e.GetName()))
            {
                continue; // island wins on collision
            }
            CopyClass(*table, e, *globalCfg);
            Record r;
            r.className = e.GetName();
            r.origin = Origin::Global;
            r.owner = RString(e.GetOwner());
            _records.push_back(r);
        }
    }
    // Update copies array values that keep a _file back-pointer into the
    // SOURCE ParamFile (the same hazard ConfigParsers guards after a mod
    // merge) - re-point everything at the owned copy before a source dies.
    _merged.SetFile(&_merged);
    _factions = _merged.FindEntry(kTableName);
}

void BuildFactionSourcesFromEngine(FactionSources& out)
{
    out.Build(Pars.FindEntry(kTableName), ExtParsMission.FindEntry(kTableName));
}

} // namespace Poseidon::Guerrilla
