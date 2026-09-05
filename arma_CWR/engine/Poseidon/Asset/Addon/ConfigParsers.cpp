#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Core/Application.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/Asset/Addon/AddonSystem.hpp>
#include <Poseidon/UI/Locale/Stringtable/Stringtable.hpp>
#include <Poseidon/IO/Filesystem/DirScanner.hpp>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <initializer_list>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <Poseidon/Foundation/Containers/Array.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Types/Pointers.hpp>
#include <Poseidon/Foundation/platform.hpp>
#ifdef _WIN32
#include <windows.h>

#endif

namespace Poseidon
{
extern bool g_stringtableSystemAvailable;

static RString MakeBinDir(RStringB dir, bool upperCase)
{
    if (dir.GetLength() == 0)
        return upperCase ? RString("BIN") : RString("bin");
    return dir + RString(upperCase ? "/BIN" : "/bin");
}

static bool ResolveFileInDir(RStringB dir, const char* name, RString& fullPath, RString& resolvedName)
{
    RString candidate = dir + RString("/") + RString(name);
    if (QIFStreamB::FileExist(candidate))
    {
        fullPath = candidate;
        resolvedName = name;
        return true;
    }

    DirScanner scan;
    if (!scan.First(dir, nullptr))
        return false;

    do
    {
        const char* entry = scan.GetName();
        if (strcmpi(entry, name) == 0)
        {
            resolvedName = entry;
            fullPath = dir + RString("/") + resolvedName;
            return true;
        }
    } while (scan.Next());

    return false;
}

// #include resolves against the cwd, so chdir into the file's dir and parse the basename;
// the full path after chdir doubles the prefix and parses nothing.
static bool ParseTextFileFromResolvedPath(ParamFile& target, RStringB fullPath, RStringB parseName)
{
    LSError err = target.Parse(fullPath);
    if (err == LSOK)
        return true;

    const char* fullPathStr = fullPath;
    const char* slash = strrchr(fullPathStr, '/');
    if (!slash)
        return false;

    char resolvedDir[512];
    snprintf(resolvedDir, sizeof(resolvedDir), "%.*s", (int)(slash - fullPathStr), fullPathStr);
    char buffer[512];
    ::GetCurrentDirectory(512, buffer);
    SetCurrentDirectory(resolvedDir);
    err = target.Parse(parseName);
    SetCurrentDirectory(buffer);
    return err == LSOK;
}

// Every mod directory that shipped a bin/config.cpp / bin/config.bin, in the order
// ModSystem::EnumDirectories visited them - which is REVERSE mount order, last-listed
// mod first. This used to be a single SRef, so with two or more config-bearing mods
// mounted each new one silently overwrote the previous and only ONE was ever merged
// into Pars; worse, because the enumeration runs backwards, the survivor was the
// EARLIEST-listed (lowest priority) mod, the exact opposite of mod-override semantics.
// Symptom: a preload mod mounted last ("@LoBo;@lobofixup;@udshowcase") contributed
// nothing at all, and its CfgAddons/PreloadAddons entries never reached
// World::ActivateAddons - runtime spawns of its classes then hit "Access denied ...
// owner addon is not activated".
static AutoArray<SRef<ParamFile>> s_deferredModConfigs;

// True when a mod's bin/resource won the resource enumeration (replaced the base
// menu resource). Read by the main menu to keep the community addon's custom menu
// (and hijack it) instead of flipping to the remaster menu, and by config init to
// restore the base remaster UI resources the mod's resource shadowed.
static bool s_menuOverriddenByMod = false;

// A mod's bin/config replaced the base master config; read to restore the base config-extra
// (CfgLanguages) it shadowed. Under the overhaul's deferred-merge model (see ParseConfig) the
// base config always wins the enumeration, so nothing shadows config-extra and this stays
// false; the flag and the restore path are kept for the upstream callers that gate on them.
static bool s_configOverriddenByMod = false;

bool IsMenuOverriddenByMod()
{
    return s_menuOverriddenByMod;
}

bool IsConfigOverriddenByMod()
{
    return s_configOverriddenByMod;
}

// The main menu's resource closure: every class the UD main menu resolves through.
// RscDisplayMainRemaster (defined in the package's bin/resource-extra.cpp) inherits
// RscDisplayMain, so the mod's RscDisplayMain — and every style root its controls
// derive from — is what actually decides the menu's geometry and chrome. Restoring
// only RscDisplayMain is not enough: its controls are `X: RscText` / `X: RscActiveMenu`
// and its backdrop is `Background1: RscBackgroundStripeTop`, so a mod that restyles
// the style roots still drifts our menu. This list is the transitive closure of
// RscDisplayMain's base chain + its controls' base chains, and nothing else:
//
//   RscDisplayMain              : RscDisplayBackgroundStripesDark : RscDisplayBackgroundStripes
//     Background1/2/3           : RscBackgroundStripeTop/Bottom/Dark : RscText
//     Line1/Line2/Version/Date  : RscText
//     CWA/FP1..3                : RscPicture     (the Demo package's resource-extra
//                                                  overrides the CWA picture control)
//     Player/Multiplayer/…/Quit : RscActiveMenu  : RscActiveText
//
// All ten are access=3 (PAReadOnlyVerified) in the vanilla resource — Bohemia marked
// them as not-moddable, and the only way a mod changes them at all is the wholesale
// bin/resource replacement this restore closes. Ten classes out of ~800 top-level
// entries: a mod's own displays (briefing, mission, MP screens, its custom dialogs)
// are untouched, so this is a targeted restore, not a wholesale one.
static const char* const kBaseMenuClasses[] = {
    "RscText",
    "RscPicture",
    "RscActiveText",
    "RscActiveMenu",
    "RscBackgroundStripeTop",
    "RscBackgroundStripeBottom",
    "RscBackgroundStripeDark",
    "RscDisplayBackgroundStripes",
    "RscDisplayBackgroundStripesDark",
    "RscDisplayMain",
};

static bool IsBaseMenuClass(const RStringB& name)
{
    for (const char* known : kBaseMenuClasses)
    {
        if (strcmpi(name, known) == 0)
            return true;
    }
    return false;
}

void RestoreBaseMenuResource()
{
    // A mod's bin/resource wins the ParseResource enumeration outright (ParamFile::Parse
    // clears Res on entry), so with a mod mounted the vanilla menu resource is never read
    // at all and the UD menu ends up standing on the mod's furniture: our
    // RscDisplayMainRemaster is restored by MergeBaseResourceExtra, but its inheritance
    // base RscDisplayMain is the mod's, with a different control set and geometry. Under
    // @LoBo that meant no Line1 divider (the class does not exist there), MULTIPLAYER at
    // the top-right, OPTIONS at the bottom-left and PLAYER/QUIT/Version relocated to the
    // screen edges. Re-read the base resource and put the menu closure back.
    //
    // ORDER: this must run BEFORE MergeBaseResourceExtra. resource-extra.cpp is the UD
    // layer — it inherits RscDisplayMain and (in the Demo package) overrides nested
    // controls of it. Vanilla furniture has to be in place first so those overrides
    // inherit from and win over it; restoring afterwards would wipe them.
    for (bool upperCase : {false, true})
    {
        RString binDir = MakeBinDir("", upperCase);
        RString file;
        RString fileName;

        ParamFile baseRes;
        bool parsed = false;
        if (ResolveFileInDir(binDir, "resource.cpp", file, fileName))
            parsed = ParseTextFileFromResolvedPath(baseRes, file, fileName);
        else if (ResolveFileInDir(binDir, "resource.bin", file, fileName))
            parsed = baseRes.ParseBin(file);
        if (!parsed)
            continue;

        // Prune the base copy down to the menu closure so the Update below can touch
        // nothing else — every surviving class's base is itself inside the closure, so
        // no base pointer is left dangling. Collect first, delete after: Delete()
        // reindexes the entry array.
        AutoArray<RStringB> drop;
        for (int i = 0; i < baseRes.GetEntryCount(); i++)
        {
            const RStringB& name = baseRes.GetEntry(i).GetName();
            if (!IsBaseMenuClass(name))
                drop.Add(name);
        }
        for (int i = 0; i < drop.Size(); i++)
            baseRes.Delete(drop[i]);

        // Prepare each destination class in Res:
        //  * strip its entries — Update() MERGES, and the mod's leftovers would survive
        //    and shadow the vanilla ones (@LoBo's RscDisplayMain carries SWI1..SWI4 and
        //    its own nested Background3 that hides the inherited dark stripe). Strip the
        //    children, not the class object itself: other classes hold raw _base pointers
        //    to it (ParamClass::_base is an InitPtr, not a Ref), so deleting the top-level
        //    entry would dangle them.
        //  * lift the access lock — these are all PAReadOnlyVerified and
        //    ParamClass::Update() refuses outright at >= PAReadOnly.
        for (const char* name : kBaseMenuClasses)
        {
            ParamEntry* dstEntry = Res.FindEntry(name);
            ParamClass* dstCls = dstEntry ? dstEntry->GetClassInterface() : nullptr;
            if (!dstCls)
                continue;
            while (dstCls->GetEntryCount() > 0)
            {
                const RStringB child = dstCls->GetEntry(0).GetName(); // copy: Delete frees the entry
                dstCls->Delete(child);
            }
            dstCls->SetAccessMode(PADefault);
        }

        Res.Update(baseRes);
        // Update copies array values verbatim, keeping their _file back-pointer aimed at
        // the stack-local `baseRes`; re-point them at Res before it dies, or a later
        // string-expression GetFloat (a control's position/up/direction) derefs freed
        // stack. Same hazard as MergeBaseResourceExtra below.
        Res.SetFile(&Res);

        // Re-apply the vanilla access mode. Two reasons: (1) it restores the exact
        // vanilla protection state, which the MP config-integrity walk checksums
        // (Network/NetworkServerIntegrity.cpp covers Res as well as Pars); (2)
        // MergeBaseResourceExtra runs next and resource-extra.cpp declares an empty
        // `class RscDisplayMain {};` placeholder — with no base. ParamClass::Update()
        // nulls the destination's base for a baseless source class whenever the
        // destination is writable, so an unlocked RscDisplayMain would silently lose its
        // RscDisplayBackgroundStripesDark base and with it the whole menu backdrop.
        // Vanilla is protected from that by the very lock we just lifted.
        for (const char* name : kBaseMenuClasses)
        {
            const ParamEntry* srcEntry = baseRes.FindEntry(name);
            ParamEntry* dstEntry = Res.FindEntry(name);
            if (srcEntry && dstEntry)
                dstEntry->SetAccessModeForAll(srcEntry->GetAccessMode());
        }

        LOG_INFO(Config, "RestoreBaseMenuResource: restored the base menu closure from {} over the mod resource",
                 (const char*)file);
        return;
    }

    LOG_WARN(Config, "RestoreBaseMenuResource: no base bin/resource found; the mod's main menu layout stands");
}

void MergeBaseResourceExtra()
{
    // ParseResource stops at the first mod's bin/resource and clears Res, so the base
    // resource-extra.cpp (remaster Options/Mods UI) is never reached; merge it back on top.
    for (bool upperCase : {false, true})
    {
        RString binDir = MakeBinDir("", upperCase);
        RString extraFile;
        RString extraFileName;
        if (ResolveFileInDir(binDir, "resource-extra.cpp", extraFile, extraFileName))
        {
            ParamFile extra;
            ParseTextFileFromResolvedPath(extra, extraFile, extraFileName);
            Res.Update(extra);
            // Update leaves array values' _file aimed at the stack-local `extra`; re-point at Res
            // before it dies, else a later string-expression GetFloat derefs freed stack.
            Res.SetFile(&Res);
            LOG_INFO(Config, "MergeBaseResourceExtra: restored base {} over the mod resource", (const char*)extraFile);
            return;
        }
    }
}

void MergeBaseConfigExtra()
{
    // Twin of MergeBaseResourceExtra: a config-replacing mod clears Pars and shadows the base
    // config-extra.cpp (CfgLanguages); re-apply it on top.
    for (bool upperCase : {false, true})
    {
        RString binDir = MakeBinDir("", upperCase);
        RString extraFile;
        RString extraFileName;
        if (ResolveFileInDir(binDir, "config-extra.cpp", extraFile, extraFileName))
        {
            ParamFile extra;
            ParseTextFileFromResolvedPath(extra, extraFile, extraFileName);
            Pars.Update(extra);
            // Re-point at Pars before the stack-local `extra` dies (stack-use-after-return), as
            // in the resource-extra path.
            Pars.SetFile(&Pars);
            LOG_INFO(Config, "MergeBaseConfigExtra: restored base {} over the mod config", (const char*)extraFile);
            return;
        }
    }
}

static bool ParseConfigFromDir(const RStringB& dir, ParamFile& target)
{
    RString file;
    RString fileName;
    if (ResolveFileInDir(dir, "config.cpp", file, fileName))
        return ParseTextFileFromResolvedPath(target, file, fileName);
    if (ResolveFileInDir(dir, "config.bin", file, fileName))
    {
        target.ParseBin(file);
        return true;
    }
    return false;
}

bool ParseStringtable(RStringB dir, void* context)
{
    for (bool upperCase : {false, true})
    {
        RString binDir = MakeBinDir(dir, upperCase);
        RString file;
        RString fileName;
        if (ResolveFileInDir(binDir, "stringtable.csv", file, fileName))
        {
            LoadStringtable("Global", file, 0, false);
            g_stringtableSystemAvailable = true;
            return true;
        }
    }
    return false;
}

bool ParseConfig(RStringB dir, void* context)
{
    // Reset so a re-init leaves the flag accurate. Upstream 3.05 made a mod's bin/config
    // REPLACE the base outright (enumeration stops at the first mod that returns true);
    // the overhaul keeps the deferred-merge model instead - a mod's config is layered ON
    // TOP of the base so a stack like "@LoBo;@lobofixup;@udshowcase" contributes all of
    // its CfgAddons/PreloadAddons entries and the vanilla roster stays available for
    // Guerrilla Mode's island/faction swap. Because the base always wins the enumeration
    // here, nothing ever shadows the base config-extra and the flag stays false; the
    // upstream IsConfigOverriddenByMod()/MergeBaseConfigExtra() restore path is kept as a
    // no-op safety net for callers (Configuration.cpp, InitBridge.cpp).
    s_configOverriddenByMod = false;

    bool isMod = (dir.GetLength() > 0);
    if (isMod)
    {
        for (bool upperCase : {false, true})
        {
            RString binDir = MakeBinDir(dir, upperCase);
            SRef<ParamFile> modConfig = new ParamFile;
            if (ParseConfigFromDir(binDir, *modConfig))
            {
                s_deferredModConfigs.Add(modConfig);
                LOG_INFO(Config, "  Mod config found in {}, deferring merge", (const char*)binDir);
                break;
            }
        }
        return false;
    }

    RString binDirUsed;
    for (bool upperCase : {false, true})
    {
        RString binDir = MakeBinDir(dir, upperCase);
        if (ParseConfigFromDir(binDir, Pars))
        {
            binDirUsed = binDir;
            break;
        }
    }
    if (binDirUsed.GetLength() == 0)
    {
        // No base config: nothing to merge into. Drop what was collected anyway so a
        // later in-process re-mount starts from an empty deferred list.
        s_deferredModConfigs.Clear();
        return false;
    }

    // Merge back-to-front: the collection order is reverse mount order, so
    // walking it backwards applies the earliest-listed mod first and lets each
    // later-listed mod override it. Same precedence a player expects from
    // -mod=a;b;c, and the same direction ParamClass::Update already gives
    // within one file.
    for (int i = s_deferredModConfigs.Size() - 1; i >= 0; i--)
    {
        LOG_INFO(Config, "  Merging deferred mod config {} of {} into base", s_deferredModConfigs.Size() - i,
                 s_deferredModConfigs.Size());
        AddonSystem::MergeIntoBaseConfig(*s_deferredModConfigs[i]);
    }
    // Update copies array values verbatim and they keep a _file back-pointer
    // into the source ParamFile, which the Clear() below frees. Re-point them
    // at Pars first - the same hazard MergeBaseResourceExtra guards for Res.
    if (s_deferredModConfigs.Size() > 0)
    {
        Pars.SetFile(&Pars);
    }
    s_deferredModConfigs.Clear();

    // config-extra.cpp carries the remaster's CfgLanguages as new classes, merged via Update() so
    // they apply without rebuilding CONFIG.BIN. Applied last, after the mod layer, so the remaster
    // language set survives a mod that happens to carry its own CfgLanguages - the same
    // restore-on-top intent as MergeBaseConfigExtra.
    RString extraFile;
    RString extraFileName;
    if (ResolveFileInDir(binDirUsed, "config-extra.cpp", extraFile, extraFileName))
    {
        ParamFile extra;
        ParseTextFileFromResolvedPath(extra, extraFile, extraFileName);
        Pars.Update(extra);
        Pars.SetFile(&Pars);
        LOG_INFO(Config, "ParseConfig: merged {} into Pars", (const char*)extraFile);
    }

    return true;
}

bool ParseRemaster(RStringB dir, void* context)
{
    for (bool upperCase : {false, true})
    {
        RString binDir = MakeBinDir(dir, upperCase);
        RString file;
        RString fileName;
        if (ResolveFileInDir(binDir, "remaster.cpp", file, fileName))
            return ParseTextFileFromResolvedPath(Remaster, file, fileName);
        if (ResolveFileInDir(binDir, "remaster.bin", file, fileName))
        {
            Remaster.ParseBin(file);
            return true;
        }
    }
    return false;
}

bool ParseResource(RStringB dir, void* context)
{
    bool ok = false;
    RString binDirUsed;
    for (bool upperCase : {false, true})
    {
        RString binDir = MakeBinDir(dir, upperCase);
        RString file;
        RString fileName;
        if (ResolveFileInDir(binDir, "resource.cpp", file, fileName))
        {
            ParseTextFileFromResolvedPath(Res, file, fileName);
            ok = true;
            binDirUsed = binDir;
            break;
        }
        if (ResolveFileInDir(binDir, "resource.bin", file, fileName))
        {
            Res.ParseBin(file);
            ok = true;
            binDirUsed = binDir;
            break;
        }
    }

    if (!ok)
        return false;

    // resource-extra.cpp adds displays/templates without rebuilding RESOURCE.BIN; Update() (not
    // Parse, which clears entries) merges it on top.
    {
        RString extraFile;
        RString extraFileName;
        if (ResolveFileInDir(binDirUsed, "resource-extra.cpp", extraFile, extraFileName))
        {
            ParamFile extra;
            ParseTextFileFromResolvedPath(extra, extraFile, extraFileName);
            Res.Update(extra);
            // Re-point at Res before the stack-local `extra` dies, else a later string-expression
            // GetFloat derefs freed stack (stack-use-after-return).
            Res.SetFile(&Res);
            LOG_INFO(Config, "ParseResource: merged {} into Res", (const char*)extraFile);
        }
    }

    // Non-empty dir means a mod's resource won (enumeration stopped here).
    s_menuOverriddenByMod = (dir.GetLength() > 0);
    return true;
}

} // namespace Poseidon
