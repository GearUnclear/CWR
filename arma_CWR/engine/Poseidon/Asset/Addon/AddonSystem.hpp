#pragma once

#include <Poseidon/Asset/Addon/AddonInfo.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Foundation/Containers/HashMap.hpp>
#include <Poseidon/Foundation/Containers/Array.hpp>

namespace Poseidon
{

class AddonSystem
{
  public:
    static void            RegisterAddon(RString name, RString prefix);
    static const AddonInfo* FindAddonInfo(RString name);
    static void            ClearRegistry();

    static void LoadAddon(const char* addon);
    static void UnloadAddon(const char* addon);
    static void UnlockAllAddons();
    static void MarkAllAddonsLockable();

    static bool   ParseAddonConfig(const RString& prefixPath);
    static void   ParseAllAddonConfigs();
    static void   ClearAddonConfigs();

    /// Merge a mod or addon config into the global `Pars`. Use this instead of a bare
    /// `Pars.Update()` for anything sourced from a mod: it unlocks the addon registry
    /// (`CfgAddons`) for the duration of the merge, which a plain Update cannot write.
    static void MergeIntoBaseConfig(const ParamFile& source);

    /// The destination-explicit form of MergeIntoBaseConfig, so the addon-registry
    /// unlock can be exercised without touching the global config.
    static void MergeConfigInto(ParamFile& target, const ParamFile& source);

    static RString          CheckAddonName(const ParamEntry& addon);
    static bool             CheckVersion(const RString& prefix, const ParamEntry& addon);
    static RString          GetAddonName(const ParamFile& config);
    static const ParamFile* FindAddonConfig(RStringB name);
    static bool             IsPreloadedAddon(RStringB name);

  private:
    typedef MapStringToClass<AddonInfo, AutoArray<AddonInfo>> AddonInfoMap;
    static AddonInfoMap                 s_addonRegistry;
    static AutoArray<SRef<ParamFile>>   s_addonConfigs;

    static void UnlockAddonCallback(const AddonInfo& addon, AddonInfoMap* map, void* context);
    static void MarkAddonLockableCallback(const AddonInfo& addon, AddonInfoMap* map, void* context);
};

} // namespace Poseidon
