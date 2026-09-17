#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Game/Guerrilla/PortraitService.hpp>
#include <Poseidon/Graphics/Textures/Image.hpp>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include <stdexcept>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{
struct Cache
{
    std::filesystem::path root =
        std::filesystem::temp_directory_path() /
        ("portrait-service-" + std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    ~Cache()
    {
        std::error_code ec;
        std::filesystem::remove_all(root, ec);
    }
};
void Backend(PortraitService& service, std::string dependency = "asset-v1", bool success = true)
{
    service.SetTestBackend(
        [dependency](const PortraitAppearance&, std::string& key, std::string&)
        {
            key += dependency;
            return true;
        },
        [success](const PortraitAppearance&, std::vector<uint8_t>& rgb, std::string& error)
        {
            if (!success)
            {
                error = "unsupported synthetic appearance";
                return false;
            }
            rgb.assign(512 * 512 * 3, 128);
            return true;
        });
}
const PortraitAppearance appearance{RString("Body"), RString("Face10")};
} // namespace

TEST_CASE("portrait identity uses SHA256 and unambiguous case-folded appearance fields", "[portrait][guerrilla]")
{
    CHECK(PortraitDigest("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    Cache cache;
    PortraitService service(cache.root);
    const auto id = service.Request(appearance);
    CHECK(service.Request({"BODY", "face10"}) == id);
    CHECK(service.Request({"Bo", "dyFace10"}) != id);
    CHECK(service.Total() == 2);
}

TEST_CASE("portrait cold and warm launches deduplicate without renders or rewrites", "[portrait][guerrilla]")
{
    Cache cache;
    PortraitService cold(cache.root);
    Backend(cold);
    const auto id = cold.Request(appearance);
    for (int i = 0; i < 10; ++i)
        cold.Request(appearance, true);
    CHECK(cold.Total() == 1);
    CHECK(cold.Find(id)->status == PortraitStatus::Queued);
    cold.Advance();
    REQUIRE(cold.Find(id)->status == PortraitStatus::Ready);
    CHECK(cold.RenderCount() == 1);
    CHECK(cold.Revision() == 1);
    const auto file = cache.root / cold.Find(id)->cacheKey / "portrait.png";
    const auto modified = std::filesystem::last_write_time(file);
    PortraitService warm(cache.root);
    Backend(warm);
    warm.Request(appearance);
    warm.Advance();
    CHECK(warm.RenderCount() == 0);
    CHECK(warm.CacheHits() == 1);
    CHECK(warm.Find(id)->pixels == cold.Find(id)->pixels);
    CHECK(std::filesystem::last_write_time(file) == modified);
}

TEST_CASE("portrait dependency changes and corrupt PNGs regenerate", "[portrait][guerrilla]")
{
    Cache cache;
    PortraitService first(cache.root);
    Backend(first);
    const auto id = first.Request(appearance);
    first.Advance();
    const auto oldKey = first.Find(id)->cacheKey;
    std::ofstream(cache.root / oldKey / "portrait.png", std::ios::trunc) << "interrupted";
    PortraitService repaired(cache.root);
    Backend(repaired);
    repaired.Request(appearance);
    repaired.Advance();
    REQUIRE(repaired.Find(id)->status == PortraitStatus::Ready);
    CHECK(repaired.RenderCount() == 1);
    PortraitService changed(cache.root);
    Backend(changed, "overridden-face-v2");
    changed.Request(appearance);
    changed.Advance();
    CHECK(changed.RenderCount() == 1);
    CHECK(changed.Find(id)->cacheKey != oldKey);
}

TEST_CASE("portrait failure completes once and cancellation retains completed entries", "[portrait][guerrilla]")
{
    Cache cache;
    PortraitService service(cache.root);
    Backend(service, "v1", false);
    const auto id = service.Request(appearance);
    service.Advance();
    CHECK(service.Find(id)->status == PortraitStatus::Unavailable);
    CHECK(service.Completed() == 1);
    service.Request(appearance);
    service.Advance();
    CHECK(service.RenderCount() == 1);
    service.Request({"Next", "Face18"});
    service.Cancel();
    CHECK_FALSE(service.Pending());
    CHECK(service.Find(id)->status == PortraitStatus::Unavailable);
}

TEST_CASE("portrait cache rejects wrong dimensions and nonopaque decoded images", "[portrait][guerrilla]")
{
    Cache cache;
    PortraitService first(cache.root);
    Backend(first);
    const auto id = first.Request(appearance);
    first.Advance();
    const auto file = (cache.root / first.Find(id)->cacheKey / "portrait.png").string();
    SECTION("wrong dimensions")
    {
        REQUIRE(Image::FromRGBA(1, 1, {128, 128, 128, 255}).Save(file));
    }
    SECTION("transparent decoded image")
    {
        REQUIRE(Image::FromRGBA(256, 256, std::vector<uint8_t>(256 * 256 * 4, 128)).Save(file));
    }
    SECTION("truncated image with intact header")
    {
        std::filesystem::resize_file(file, 40);
    }
    PortraitService next(cache.root);
    Backend(next);
    next.Request(appearance);
    next.Advance();
    CHECK(next.RenderCount() == 1);
    CHECK(next.CacheHits() == 0);
    CHECK(next.Find(id)->status == PortraitStatus::Ready);
}

TEST_CASE("clearing portraits drops memory and disk entries and permits regeneration", "[portrait][guerrilla]")
{
    Cache cache;
    PortraitService service(cache.root);
    Backend(service);
    const auto id = service.Request(appearance);
    service.Advance();
    const auto revision = service.Revision();
    REQUIRE(std::filesystem::exists(cache.root));
    service.ClearCache();
    CHECK_FALSE(std::filesystem::exists(cache.root));
    CHECK(service.Total() == 0);
    CHECK(service.Revision() > revision);
    CHECK(service.Find(id) == nullptr);
    CHECK(service.Request(appearance) == id);
    service.Advance();
    CHECK(service.RenderCount() == 1);
    CHECK(service.Find(id)->status == PortraitStatus::Ready);
}

TEST_CASE("portrait cache write failure retains session pixels", "[portrait][guerrilla]")
{
    Cache cache;
    std::ofstream(cache.root) << "not a directory";
    PortraitService service(cache.root);
    Backend(service);
    const auto id = service.Request(appearance);
    service.Advance();
    CHECK(service.Find(id)->status == PortraitStatus::Ready);
    CHECK(service.Find(id)->pixels.size() == 256 * 256 * 4);
    service.ReleaseGraphics();
    CHECK(service.Find(id)->pixels.size() == 256 * 256 * 4);
}

TEST_CASE("portrait concurrent publication exposes complete entries", "[portrait][guerrilla]")
{
    Cache cache;
    PortraitService a(cache.root), b(cache.root);
    Backend(a);
    Backend(b);
    a.Request(appearance);
    b.Request(appearance);
    std::thread first([&] { a.Advance(); });
    std::thread second([&] { b.Advance(); });
    first.join();
    second.join();
    PortraitService reader(cache.root);
    Backend(reader);
    const auto id = reader.Request(appearance);
    reader.Advance();
    CHECK(reader.CacheHits() == 1);
    CHECK(reader.RenderCount() == 0);
    CHECK(reader.Find(id)->status == PortraitStatus::Ready);
}

TEST_CASE("portrait queue prioritizes open dossier and recovers after teardown", "[portrait][guerrilla]")
{
    Cache cache;
    PortraitService service(cache.root);
    Backend(service);
    const auto first = service.Request(appearance);
    const auto open = service.Request({"LaterBody", "Face18"}, true);
    service.Advance();
    CHECK(service.Find(open)->status == PortraitStatus::Ready);
    CHECK(service.Find(first)->status == PortraitStatus::Queued);
    const auto pixels = service.Find(open)->pixels;
    service.ReleaseGraphics();
    CHECK(service.Find(open)->pixels == pixels);
    service.Cancel();
    CHECK(service.Total() == 1);
    service.Teardown();
    CHECK(service.Total() == 0);
    CHECK(service.RenderCount() == 0);
    CHECK(service.CacheHits() == 0);
}
TEST_CASE("portrait exceptions terminate one request and leave queue usable", "[portrait][guerrilla]")
{
    Cache cache;
    PortraitService service(cache.root);
    service.SetTestBackend([](const PortraitAppearance&, std::string&, std::string&) -> bool
                           { throw std::runtime_error("missing asset"); }, {});
    const auto failed = service.Request(appearance);
    service.Advance();
    CHECK(service.Find(failed)->status == PortraitStatus::Unavailable);
    CHECK(service.Completed() == 1);
    Backend(service);
    const auto next = service.Request({"Next", "Face18"});
    service.Advance();
    CHECK(service.Find(next)->status == PortraitStatus::Ready);
}
