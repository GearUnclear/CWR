#include <Poseidon/Game/Guerrilla/PortraitService.hpp>
#include <Poseidon/Game/Guerrilla/PortraitRecipe.hpp>
#include <Poseidon/Foundation/Common/GamePaths.hpp>
#include <Poseidon/Foundation/Common/Sha256.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Textures/Image.hpp>
#include <Poseidon/Graphics/Textures/LooseTextures.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <fstream>
#include <atomic>
#include <chrono>
#include <algorithm>

namespace Poseidon::Guerrilla
{
namespace
{
std::string Hex(const std::array<uint8_t, 32>& bytes)
{
    std::string result;
    for (const auto byte : bytes)
    {
        result += "0123456789abcdef"[byte >> 4];
        result += "0123456789abcdef"[byte & 15];
    }
    return result;
}
void Field(std::string& dest, const std::string& value)
{
    dest += std::to_string(value.size()) + ":" + value;
}
std::string Identity(const PortraitAppearance& appearance)
{
    RString body = appearance.body, face = appearance.face;
    body.Lower();
    face.Lower();
    std::string value;
    Field(value, (const char*)body);
    Field(value, (const char*)face);
    return value;
}
} // namespace

std::string PortraitDigest(const std::string& value)
{
    Sha256 sha;
    sha.Update(reinterpret_cast<const uint8_t*>(value.data()), value.size());
    return Hex(sha.Finish());
}
PortraitService::PortraitService(std::filesystem::path cacheRoot) : _root(std::move(cacheRoot)) {}
PortraitService& PortraitService::Instance()
{
    static PortraitService service;
    return service;
}
std::filesystem::path PortraitService::Root() const
{
    return _root.empty() ? std::filesystem::path(Foundation::GamePaths::Instance().CacheDir()) / "portraits" : _root;
}
void PortraitService::SetTestBackend(Resolve resolve, Generate generate)
{
    _resolve = std::move(resolve);
    _generate = std::move(generate);
}
RString PortraitService::Request(const PortraitAppearance& appearance, bool priority)
{
    const std::string id = PortraitDigest(Identity(appearance));
    auto [it, inserted] = _entries.try_emplace(id);
    if (inserted)
    {
        it->second.appearance = appearance;
        _queue.push_back(id);
    }
    if (priority && it->second.status == PortraitStatus::Queued)
    {
        _queue.erase(std::remove(_queue.begin(), _queue.end(), id), _queue.end());
        _queue.push_front(id);
    }
    return RString(id.c_str());
}
void PortraitService::PrepareRoster(const std::vector<PortraitAppearance>& roster)
{
    for (const auto& appearance : roster)
        Request(appearance);
}
const PortraitEntry* PortraitService::Find(const RString& id) const
{
    auto it = _entries.find((const char*)id);
    return it == _entries.end() ? nullptr : &it->second;
}
Texture* PortraitService::TextureFor(const RString& id)
{
    auto it = _entries.find((const char*)id);
    if (it == _entries.end() || it->second.status != PortraitStatus::Ready || !GEngine)
        return nullptr;
    auto& entry = it->second;
    if (!entry.texture)
        entry.texture = GEngine->TextBank()->CreateDynamic(256, 256, entry.pixels.data(),
                                                           static_cast<uint32_t>(entry.pixels.size()), true);
    if (entry.texture)
        entry.texture->SetName(RStringB((std::string("portrait:") + (const char*)id).c_str()));
    return entry.texture;
}
bool PortraitService::ResolveDependencies(PortraitRenderer& renderer, std::string& metadata, std::string& error)
{
    Field(metadata, renderer.Configuration());
    for (const auto& dependency : renderer.Dependencies())
    {
        const RString actual = Graphics::ResolveLooseTexturePath(dependency.c_str());
        const std::string path = (const char*)actual;
        auto found = _dependencyHashes.find(path);
        if (found == _dependencyHashes.end())
        {
            QIFStreamB file;
            file.AutoOpen(actual);
            if (file.fail())
            {
                error = "missing dependency: " + path;
                return false;
            }
            Sha256 sha;
            std::array<uint8_t, 65536> buffer;
            while (file.rest() > 0)
            {
                const int count = std::min<int>(file.rest(), static_cast<int>(buffer.size()));
                file.read(buffer.data(), count);
                if (file.fail())
                {
                    error = "unreadable dependency: " + path;
                    return false;
                }
                sha.Update(buffer.data(), count);
            }
            found = _dependencyHashes.emplace(path, Hex(sha.Finish())).first;
        }
        Field(metadata, dependency);
        Field(metadata, path);
        Field(metadata, found->second);
    }
    return true;
}
bool PortraitService::Read(const std::string& key, const std::string& metadata, std::vector<uint8_t>& pixels) const
{
    const auto path = Root() / key;
    std::error_code ec;
    if (std::filesystem::file_size(path / "dependencies", ec) != metadata.size() || ec)
        return false;
    std::ifstream record(path / "dependencies", std::ios::binary);
    const std::string stored((std::istreambuf_iterator<char>(record)), {});
    if (stored != metadata)
        return false;
    const auto bytes = std::filesystem::file_size(path / "portrait.png", ec);
    if (ec || bytes < 33 || bytes > 1024 * 1024)
        return false;
    // Reject hostile dimensions before the decoder allocates memory.
    std::ifstream png(path / "portrait.png", std::ios::binary);
    std::array<unsigned char, 24> header{};
    png.read(reinterpret_cast<char*>(header.data()), header.size());
    const std::array<unsigned char, 8> magic = {137, 80, 78, 71, 13, 10, 26, 10};
    if (!std::equal(magic.begin(), magic.end(), header.begin()))
        return false;
    const auto dimension = [&](int p)
    {
        return (uint32_t(header[p]) << 24) | (uint32_t(header[p + 1]) << 16) | (uint32_t(header[p + 2]) << 8) |
               header[p + 3];
    };
    if (dimension(16) != 256 || dimension(20) != 256)
        return false;
    const auto image = Image::FromFile((path / "portrait.png").string()).ToRGBA();
    if (!image.valid() || image.width() != 256 || image.height() != 256 || image.dataSize() != 256 * 256 * 4)
        return false;
    for (size_t i = 3; i < image.dataSize(); i += 4)
        if (image.data()[i] != 255)
            return false;
    pixels = image.data();
    return true;
}
void PortraitService::Write(const std::string& key, const std::string& metadata,
                            const std::vector<uint8_t>& pixels) const
{
    std::error_code ec;
    std::filesystem::create_directories(Root(), ec);
    if (ec)
        return;
    static std::atomic<uint64_t> serial{0};
    std::filesystem::path temp;
    bool created = false;
    for (int i = 0; i < 20 && !created; ++i)
    {
        temp =
            Root() / (".tmp-" + std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()) +
                      "-" + std::to_string(serial++));
        created = std::filesystem::create_directory(temp, ec);
    }
    if (!created)
        return;
    struct Cleanup
    {
        std::filesystem::path path;
        ~Cleanup()
        {
            std::error_code error;
            std::filesystem::remove_all(path, error);
        }
    } cleanup{temp};
    if (!Image::FromRGBA(256, 256, pixels).Save((temp / "portrait.png").string()))
        return;
    std::ofstream record(temp / "dependencies", std::ios::binary);
    record.write(metadata.data(), metadata.size());
    record.close();
    if (!record)
        return;
    std::vector<uint8_t> existing;
    if (Read(key, metadata, existing))
        return;
    // Publish one complete directory. Move corrupt old entries aside first;
    // concurrent readers either see a complete entry or treat it as a miss.
    const auto dest = Root() / key;
    const auto retired = temp / "retired";
    std::filesystem::rename(dest, retired, ec);
    ec.clear();
    std::filesystem::rename(temp, dest, ec);
    if (!ec)
    {
        cleanup.path = dest / "retired";
    }
}
void PortraitService::Advance()
{
    if (_queue.empty())
        return;
    const std::string id = _queue.front();
    _queue.pop_front();
    auto& entry = _entries.at(id);
    entry.status = PortraitStatus::Generating;
    try
    {
        PortraitRenderer renderer;
        std::string metadata;
        Field(metadata, PortraitRecipeVersion);
        Field(metadata, Identity(entry.appearance));
        const bool resolved = _resolve ? _resolve(entry.appearance, metadata, entry.error)
                                       : renderer.Prepare(entry.appearance, entry.error) &&
                                             ResolveDependencies(renderer, metadata, entry.error);
        if (resolved)
        {
            entry.cacheKey = PortraitDigest(metadata);
            if (Read(entry.cacheKey, metadata, entry.pixels))
                ++_hits;
            else
            {
                std::vector<uint8_t> rgb;
                ++_renders;
                const bool captured =
                    _generate ? _generate(entry.appearance, rgb, entry.error) : renderer.Capture(rgb, entry.error);
                if (captured)
                {
                    entry.pixels = StylePortrait(rgb);
                    if (entry.pixels.empty())
                        entry.error = "invalid portrait readback";
                }
                if (!entry.pixels.empty())
                    Write(entry.cacheKey, metadata, entry.pixels);
            }
        }
    }
    catch (const std::exception& e)
    {
        entry.error = e.what();
    }
    entry.status = entry.pixels.empty() ? PortraitStatus::Unavailable : PortraitStatus::Ready;
    if (entry.status == PortraitStatus::Unavailable)
        LOG_WARN(Core, "Portrait unavailable for {}/{}: {}", (const char*)entry.appearance.body,
                 (const char*)entry.appearance.face, entry.error);
    ++_revision;
}
size_t PortraitService::Completed() const
{
    return std::count_if(
        _entries.begin(), _entries.end(), [](const auto& item)
        { return item.second.status == PortraitStatus::Ready || item.second.status == PortraitStatus::Unavailable; });
}
void PortraitService::Cancel()
{
    for (const auto& id : _queue)
        _entries.erase(id);
    _queue.clear();
    ++_revision;
}
void PortraitService::ReleaseGraphics()
{
    for (auto& [id, entry] : _entries)
        entry.texture = nullptr;
    ++_revision;
}
void PortraitService::Teardown()
{
    _queue.clear();
    _entries.clear();
    _dependencyHashes.clear();
    _renders = _hits = 0;
    ++_revision;
}
void PortraitService::ClearCache()
{
    Teardown();
    std::error_code ec;
    std::filesystem::remove_all(Root(), ec);
    if (ec)
        LOG_WARN(Core, "Unable to clear dossier portrait cache: {}", ec.message());
}
} // namespace Poseidon::Guerrilla
