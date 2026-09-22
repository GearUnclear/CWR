#include <Poseidon/Core/ModInstall.hpp>
#include <Poseidon/Foundation/Common/Sha256.hpp>

#include <Poseidon/Core/ModCollection.hpp>

#include <cjson/cJSON.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <utility>
#include <vector>

namespace Poseidon
{
namespace
{

} // namespace

bool VerifyModArtifact(const std::string& path, int64_t expectedBytes, const std::string& expectedSha256,
                       std::string* error)
{
    const auto fail = [&](const std::string& message)
    {
        if (error != nullptr)
            *error = message;
        return false;
    };
    FILE* file = std::fopen(path.c_str(), "rb");
    if (file == nullptr)
        return fail("cannot open downloaded package");
    Sha256 hash;
    std::array<uint8_t, 64 * 1024> buffer{};
    int64_t size = 0;
    size_t read = 0;
    while ((read = std::fread(buffer.data(), 1, buffer.size(), file)) > 0)
    {
        hash.Update(buffer.data(), read);
        size += static_cast<int64_t>(read);
    }
    const bool readError = std::ferror(file) != 0;
    std::fclose(file);
    if (readError)
        return fail("cannot read downloaded package");
    if (expectedBytes > 0 && size != expectedBytes)
        return fail("downloaded package size mismatch");
    if (expectedSha256.empty())
        return true;

    static constexpr char hex[] = "0123456789abcdef";
    std::string actual;
    for (uint8_t byte : hash.Finish())
    {
        actual.push_back(hex[byte >> 4]);
        actual.push_back(hex[byte & 0x0f]);
    }
    std::string expected = expectedSha256;
    std::transform(expected.begin(), expected.end(), expected.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return actual == expected ? true : fail("downloaded package SHA-256 mismatch");
}

StagedModInstall MakeStagedModInstall(const std::string& destinationDir, const std::string& modId)
{
    static std::atomic<uint64_t> sequence{0};
    const std::string suffix = std::to_string(sequence.fetch_add(1));
    return {modId, destinationDir + ".staging-" + suffix, destinationDir, destinationDir + ".backup-" + suffix};
}

void RestoreStagedModInstalls(std::vector<StagedModInstall>& installs)
{
    namespace fs = std::filesystem;
    std::error_code ec;
    for (auto it = installs.rbegin(); it != installs.rend(); ++it)
    {
        if (it->stagingMoved)
        {
            ec.clear();
            fs::remove_all(it->destinationDir, ec);
            it->stagingMoved = false;
        }
        if (it->previousMoved)
        {
            ec.clear();
            fs::rename(it->backupDir, it->destinationDir, ec);
            if (!ec)
                it->previousMoved = false;
        }
    }
}

bool SwapStagedModInstalls(std::vector<StagedModInstall>& installs, std::string* error)
{
    namespace fs = std::filesystem;
    const auto fail = [&](const std::string& message)
    {
        RestoreStagedModInstalls(installs);
        if (error != nullptr)
            *error = message;
        return false;
    };

    std::error_code ec;
    for (StagedModInstall& install : installs)
    {
        ec.clear();
        if (!fs::is_directory(install.stagingDir, ec))
            return fail("staged mod directory is missing: " + install.stagingDir);
        if (fs::exists(install.destinationDir, ec))
        {
            ec.clear();
            fs::rename(install.destinationDir, install.backupDir, ec);
            if (ec)
                return fail("cannot back up installed mod: " + ec.message());
            install.previousMoved = true;
        }
        ec.clear();
        fs::rename(install.stagingDir, install.destinationDir, ec);
        if (ec)
            return fail("cannot activate staged mod: " + ec.message());
        install.stagingMoved = true;
    }
    return true;
}

void CommitStagedModInstalls(std::vector<StagedModInstall>& installs)
{
    namespace fs = std::filesystem;
    std::error_code ec;
    for (StagedModInstall& install : installs)
    {
        if (install.previousMoved)
        {
            ec.clear();
            fs::remove_all(install.backupDir, ec);
        }
        ec.clear();
        fs::remove(install.stagingDir + ".pbo.zst", ec);
    }
    installs.clear();
}

void DiscardStagedModInstalls(std::vector<StagedModInstall>& installs)
{
    namespace fs = std::filesystem;
    std::error_code ec;
    RestoreStagedModInstalls(installs);
    for (const StagedModInstall& install : installs)
    {
        ec.clear();
        fs::remove_all(install.stagingDir, ec);
        ec.clear();
        fs::remove(install.stagingDir + ".pbo.zst", ec);
    }
    installs.clear();
}

static std::filesystem::path PreservedInstallDir(const std::string& destinationDir)
{
    const std::filesystem::path destination(destinationDir);
    return destination.parent_path() / ".downloads" / destination.filename();
}

bool PreserveStagedModInstalls(std::vector<StagedModInstall>& installs, std::string* error)
{
    namespace fs = std::filesystem;
    for (StagedModInstall& install : installs)
    {
        const fs::path preserved = PreservedInstallDir(install.destinationDir);
        std::error_code ec;
        fs::create_directories(preserved.parent_path(), ec);
        if (ec)
        {
            if (error != nullptr)
                *error = "cannot create downloaded mod cache: " + ec.message();
            return false;
        }
        fs::remove_all(preserved, ec);
        ec.clear();
        fs::rename(install.stagingDir, preserved, ec);
        if (ec)
        {
            if (error != nullptr)
                *error = "cannot preserve downloaded mod: " + ec.message();
            return false;
        }
        ec.clear();
        fs::remove(install.stagingDir + ".pbo.zst", ec);
    }
    installs.clear();
    return true;
}

bool FindPreservedStagedModInstall(const std::string& destinationDir, const std::string& modId, int64_t packageRevision,
                                   const std::string& sha256, StagedModInstall& install)
{
    namespace fs = std::filesystem;
    const fs::path preserved = PreservedInstallDir(destinationDir);
    std::error_code ec;
    if (!fs::is_directory(preserved, ec))
        return false;
    const std::vector<ScannedMod> mods = ScanLocalMods(preserved.parent_path().string());
    const auto found = std::find_if(mods.begin(), mods.end(), [&](const ScannedMod& mod)
                                    { return mod.modId == modId && fs::path(mod.folderName) == preserved.filename(); });
    if (found == mods.end() || found->packageRevision != packageRevision ||
        (!sha256.empty() && found->sha256 != sha256))
        return false;
    install = MakeStagedModInstall(destinationDir, modId);
    install.stagingDir = preserved.string();
    return true;
}

std::string ModInstallDir(const std::string& modsRoot, const std::string& modId)
{
    return modsRoot + "/@" + modId;
}

std::string ModInstallDir(const std::string& modsRoot, const std::string& modId, const std::string& folderName)
{
    if (!folderName.empty())
        return modsRoot + "/" + folderName;
    return ModInstallDir(modsRoot, modId);
}

std::string FindInstalledModDir(const std::string& modsRoot, const std::string& modId)
{
    ModCollection mods;
    for (Mod& mod : ScanModsRoot(modsRoot, ModSource::Workshop))
        mods.Add(std::move(mod));
    const Mod* found = mods.Find(modId);
    return found != nullptr ? found->path : std::string();
}

std::string ReadInstalledModVersion(const std::string& modsRoot, const std::string& modId)
{
    namespace fs = std::filesystem;
    const std::string installDir = FindInstalledModDir(modsRoot, modId);
    if (installDir.empty())
        return {};
    const fs::path metadata = fs::path(installDir) / "mod.json";
    std::error_code ec;
    if (!fs::exists(metadata, ec))
    {
        return {};
    }

    std::ifstream in(metadata, std::ios::binary);
    if (!in)
    {
        return {};
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    const std::string text = buffer.str();

    cJSON* root = cJSON_Parse(text.c_str());
    std::string version;
    if (root != nullptr)
    {
        const cJSON* item = cJSON_GetObjectItemCaseSensitive(root, "version");
        if (cJSON_IsString(item) && item->valuestring != nullptr)
        {
            version = item->valuestring;
        }
        cJSON_Delete(root);
    }
    return version;
}

int64_t ReadInstalledPackageRevision(const std::string& modsRoot, const std::string& modId)
{
    namespace fs = std::filesystem;
    const std::string installDir = FindInstalledModDir(modsRoot, modId);
    if (installDir.empty())
        return 0;
    std::ifstream in(fs::path(installDir) / "mod.json", std::ios::binary);
    if (!in)
        return 1;
    std::ostringstream buffer;
    buffer << in.rdbuf();
    cJSON* root = cJSON_Parse(buffer.str().c_str());
    int64_t revision = 1;
    if (root != nullptr)
    {
        const cJSON* item = cJSON_GetObjectItemCaseSensitive(root, "packageRevision");
        if (cJSON_IsNumber(item) && item->valuedouble >= 1)
            revision = static_cast<int64_t>(item->valuedouble);
        cJSON_Delete(root);
    }
    return revision;
}

std::string ReadInstalledArtifactHash(const std::string& modsRoot, const std::string& modId)
{
    namespace fs = std::filesystem;
    const std::string installDir = FindInstalledModDir(modsRoot, modId);
    if (installDir.empty())
        return {};
    std::ifstream in(fs::path(installDir) / "mod.json", std::ios::binary);
    if (!in)
        return {};
    std::ostringstream buffer;
    buffer << in.rdbuf();
    cJSON* root = cJSON_Parse(buffer.str().c_str());
    std::string hash;
    if (root != nullptr)
    {
        const cJSON* item = cJSON_GetObjectItemCaseSensitive(root, "sha256");
        if (cJSON_IsString(item) && item->valuestring != nullptr)
            hash = item->valuestring;
        cJSON_Delete(root);
    }
    return hash;
}

ModInstallStatus GetModInstallStatus(const std::string& modsRoot, const std::string& modId,
                                     const std::string& catalogVersion)
{
    namespace fs = std::filesystem;
    std::error_code ec;
    if (FindInstalledModDir(modsRoot, modId).empty())
    {
        return ModInstallStatus::NotInstalled;
    }

    const std::string installed = ReadInstalledModVersion(modsRoot, modId);
    // Installed with no readable version → can't prove it's stale, so don't nag.
    if (installed.empty() || installed == catalogVersion)
    {
        return ModInstallStatus::Installed;
    }
    return ModInstallStatus::UpdateAvailable;
}

ModInstallStatus GetModInstallStatus(const std::string& modsRoot, const std::string& modId,
                                     const std::string& catalogVersion, int64_t catalogRevision,
                                     const std::string& catalogSha256)
{
    (void)catalogVersion;
    if (FindInstalledModDir(modsRoot, modId).empty())
        return ModInstallStatus::NotInstalled;
    const int64_t installedRevision = ReadInstalledPackageRevision(modsRoot, modId);
    if (installedRevision < catalogRevision)
        return ModInstallStatus::UpdateAvailable;
    if (installedRevision > catalogRevision)
        return ModInstallStatus::InstalledAhead;
    const std::string installedHash = ReadInstalledArtifactHash(modsRoot, modId);
    if (!installedHash.empty() && !catalogSha256.empty() && installedHash != catalogSha256)
        return ModInstallStatus::UpdateAvailable;
    return ModInstallStatus::Installed;
}

std::vector<ScannedMod> ScanLocalMods(const std::string& modsRoot)
{
    // The catalog view of a local scan: ScanModsRoot does the work (and sorts by
    // display name); ScannedMod is the slimmer install-status projection of Mod.
    std::vector<ScannedMod> mods;
    for (const Mod& m : ScanModsRoot(modsRoot, ModSource::Local))
        mods.push_back({m.catalogId.empty() ? m.id : m.catalogId, m.id, m.name, m.version, m.packageRevision, m.sha256,
                        m.sizeBytes});
    return mods;
}

bool WriteInstalledModManifest(const std::string& installDir, const std::string& modId, const std::string& name,
                               const std::string& version, const std::string& folderName,
                               const std::string& downloadUrl, int64_t sizeBytes, std::string* error,
                               int64_t packageRevision, const std::string& sha256)
{
    namespace fs = std::filesystem;
    const auto fail = [&](const std::string& message)
    {
        if (error != nullptr)
            *error = message;
        return false;
    };

    fs::create_directories(installDir);
    cJSON* root = cJSON_CreateObject();
    if (root == nullptr)
        return fail("cannot allocate mod manifest");
    cJSON_AddStringToObject(root, "modId", modId.c_str());
    cJSON_AddStringToObject(root, "name", name.c_str());
    cJSON_AddStringToObject(root, "version", version.c_str());
    cJSON_AddNumberToObject(root, "packageRevision", static_cast<double>(std::max<int64_t>(1, packageRevision)));
    if (!sha256.empty())
        cJSON_AddStringToObject(root, "sha256", sha256.c_str());
    if (!folderName.empty())
        cJSON_AddStringToObject(root, "folderName", folderName.c_str());
    if (!downloadUrl.empty())
        cJSON_AddStringToObject(root, "downloadUrl", downloadUrl.c_str());
    if (sizeBytes > 0)
        cJSON_AddNumberToObject(root, "sizeBytes", static_cast<double>(sizeBytes));

    char* text = cJSON_Print(root);
    cJSON_Delete(root);
    if (text == nullptr)
        return fail("cannot serialize mod manifest");

    std::ofstream out(fs::path(installDir) / "mod.json", std::ios::binary);
    if (!out)
    {
        cJSON_free(text);
        return fail("cannot write mod manifest");
    }
    out << text;
    cJSON_free(text);
    return true;
}
} // namespace Poseidon
