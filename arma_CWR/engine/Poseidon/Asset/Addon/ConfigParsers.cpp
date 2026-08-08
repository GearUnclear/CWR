#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Core/Application.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
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

// ParamFile::Parse resolves #include directives relative to the current working directory,
// so we chdir into the file's directory, parse the basename, and restore.
// Always parse the basename — passing dir/file.cpp after chdir to dir would double
// the prefix and silently parse nothing.
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

static SRef<ParamFile> s_deferredModConfig;

// True when a mod's bin/resource won the resource enumeration (replaced the base
// menu resource). Read by the main menu to keep the community addon's custom menu
// (and hijack it) instead of flipping to the remaster menu, and by config init to
// restore the base remaster UI resources the mod's resource shadowed.
static bool s_menuOverriddenByMod = false;

bool IsMenuOverriddenByMod()
{
    return s_menuOverriddenByMod;
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
    // The base game's resource-extra.cpp carries the remaster's UI additions
    // (RscOptionsShell, RscDisplayMainRemaster, …) as new classes. The resource
    // enumeration (ParseResource) stops at the first mod that ships a bin/resource
    // — and Res.ParseBin clears Res on entry — so when a community mod replaces the
    // menu resource the base extra is never reached, leaving the new Options screen
    // empty and the Mods entry gone. Merge it on top of whatever the mod loaded so
    // the remaster UI survives any resource override.
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
            // Update copies array values verbatim, keeping their _file back-pointer aimed at the
            // stack-local `extra`; re-point them at Res before `extra` dies, or a later
            // string-expression GetFloat (e.g. a control's position/up/direction) derefs freed
            // stack (stack-use-after-return opening the Mods screen).
            Res.SetFile(&Res);
            LOG_INFO(Config, "MergeBaseResourceExtra: restored base {} over the mod resource", (const char*)extraFile);
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
    bool isMod = (dir.GetLength() > 0);
    if (isMod)
    {
        for (bool upperCase : {false, true})
        {
            RString binDir = MakeBinDir(dir, upperCase);
            SRef<ParamFile> modConfig = new ParamFile;
            if (ParseConfigFromDir(binDir, *modConfig))
            {
                s_deferredModConfig = modConfig;
                LOG_INFO(Config, "  Mod config found in {}, deferring merge", (const char*)binDir);
                break;
            }
        }
        return false;
    }

    for (bool upperCase : {false, true})
    {
        RString binDir = MakeBinDir(dir, upperCase);
        if (ParseConfigFromDir(binDir, Pars))
        {
            if (s_deferredModConfig)
            {
                LOG_INFO(Config, "  Merging deferred mod config into base");
                Pars.Update(*s_deferredModConfig);
                s_deferredModConfig = nullptr;
            }
            return true;
        }
    }

    return false;
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

    // Optional supplemental resources — resource-extra.cpp in the same bin/ directory
    // is merged via Update() so it can add new displays/templates without rebuilding
    // the pre-compiled RESOURCE.BIN. Parse + Update is required because ParamFile::Parse
    // clears entries on entry (paramFile.cpp:3206).
    {
        RString extraFile;
        RString extraFileName;
        if (ResolveFileInDir(binDirUsed, "resource-extra.cpp", extraFile, extraFileName))
        {
            ParamFile extra;
            ParseTextFileFromResolvedPath(extra, extraFile, extraFileName);
            Res.Update(extra);
            // Re-point merged array values at Res before the stack-local `extra` dies — Update
            // leaves their _file aimed at the source. Otherwise a string-expression GetFloat
            // (control position/up/direction) derefs freed stack (stack-use-after-return).
            Res.SetFile(&Res);
            LOG_INFO(Config, "ParseResource: merged {} into Res", (const char*)extraFile);
        }
    }

    // The winning resource (enumeration stops here) came from a mod when dir is
    // non-empty — the base menu resource was replaced.
    s_menuOverriddenByMod = (dir.GetLength() > 0);
    return true;
}

} // namespace Poseidon
