#pragma once

#include <string>
#include <vector>

// Mount a game data package (plus optional mod folders) inside a CLI tool, so
// the merged global `Pars` is the same config PoseidonGame would see for the
// same -mod line.
//
// This is the minimum of Globals::Init that a config-reading tool needs: the
// working-directory change, the mod path, the configuration pass (base
// bin/config + the deferred mod configs + bin/config-extra.cpp), and the
// dta/addons bank load with the addon-config parse. Everything else Globals::Init
// does - the file server, profiles, input, voices, shapes, the world - is
// deliberately left out: none of it is needed to read a config, and all of it
// costs startup time or pulls in subsystems a tool has no business starting.
//
// One mount at a time per process. Unmount() restores the working directory and
// clears the banks and Pars, so a tool may mount several packages in sequence.
namespace PoseidonTools
{

class PackageMount
{
  public:
    PackageMount() = default;
    ~PackageMount();

    PackageMount(const PackageMount&) = delete;
    PackageMount& operator=(const PackageMount&) = delete;

    // `dataDir` is the package root (the directory holding bin/, dta/, addons/).
    // `modDirs` are mod folders in mount order, first listed = lowest priority,
    // matching the -mod command line. Both may be relative; they are resolved to
    // absolute paths before the chdir, because a mod path is read after it.
    // Returns false with `error` set; a failed Mount leaves nothing mounted.
    //
    // A package with only bin/config.cpp and no pbos at all is valid: the bank
    // load simply finds nothing and Pars carries the text config.
    bool Mount(const std::string& dataDir, const std::vector<std::string>& modDirs, std::string& error);

    // Idempotent; also run by the destructor.
    void Unmount();

    bool IsMounted() const { return _mounted; }
    const std::string& DataDir() const { return _dataDir; }

  private:
    bool _mounted = false;
    std::string _dataDir;
    std::string _previousDir;
};

} // namespace PoseidonTools
