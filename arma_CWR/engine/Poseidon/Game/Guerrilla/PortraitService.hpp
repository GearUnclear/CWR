#pragma once
#include <Poseidon/Game/Guerrilla/PortraitStatus.hpp>
#include <Poseidon/Game/Guerrilla/PortraitRenderer.hpp>
#include <Poseidon/Graphics/Textures/TextureBank.hpp>
#include <filesystem>
#include <functional>
#include <map>
#include <deque>

namespace Poseidon
{
class ParamEntry;
}

namespace Poseidon::Guerrilla
{
struct PortraitEntry
{
    PortraitAppearance appearance;
    PortraitStatus status = PortraitStatus::Queued;
    Ref<Texture> texture;
    std::vector<uint8_t> pixels;
    std::string error;
    std::string cacheKey;
};

class PortraitService
{
  public:
    static PortraitService& Instance();
    // graphics-thread API; no worker accesses the graphics device or entries.
    RString Request(const PortraitAppearance& appearance, bool priority = false);
    void PrepareRoster(const std::vector<PortraitAppearance>& roster);
    const PortraitEntry* Find(const RString& id) const;
    Texture* TextureFor(const RString& id);
    void Advance();
    void Cancel();
    void ReleaseGraphics();
    void ClearCache();
    void Teardown();
    unsigned Revision() const { return _revision; }
    size_t Completed() const;
    size_t Total() const { return _entries.size(); }
    bool Pending() const { return !_queue.empty(); }
    size_t RenderCount() const { return _renders; }
    size_t CacheHits() const { return _hits; }
    // A separate cache root enables source-only and isolated acceptance tests.
    explicit PortraitService(std::filesystem::path cacheRoot = {});
    using Generate = std::function<bool(const PortraitAppearance&, std::vector<uint8_t>&, std::string&)>;
    using Resolve = std::function<bool(const PortraitAppearance&, std::string&, std::string&)>;
    void SetTestBackend(Resolve resolve, Generate generate);

  private:
    std::filesystem::path Root() const;
    bool ResolveDependencies(PortraitRenderer& renderer, std::string& metadata, std::string& error);
    bool Read(const std::string& key, const std::string& metadata, std::vector<uint8_t>& pixels) const;
    void Write(const std::string& key, const std::string& metadata, const std::vector<uint8_t>& pixels) const;
    std::map<std::string, PortraitEntry> _entries;
    std::map<std::string, std::string> _dependencyHashes;
    std::deque<std::string> _queue;
    std::filesystem::path _root;
    unsigned _revision = 0;
    size_t _renders = 0, _hits = 0;
    Resolve _resolve;
    Generate _generate;
};

std::string PortraitDigest(const std::string& value);
struct FactionRecord;
std::vector<PortraitAppearance> BuildPortraitRoster(const FactionRecord* resistance, const FactionRecord* occupier,
                                                    const ParamEntry* vehicles, const ParamEntry* faces,
                                                    const std::vector<PortraitAppearance>& recorded);
std::vector<PortraitAppearance> CampaignPortraitRoster();
bool PrepareCampaignPortraits();
} // namespace Poseidon::Guerrilla
