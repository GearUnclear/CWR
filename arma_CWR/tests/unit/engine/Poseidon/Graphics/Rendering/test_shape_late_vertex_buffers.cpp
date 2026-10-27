#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Core/Config/EngineConfig.hpp>
#include <Poseidon/Core/Types.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Dummy/EngineDummy.hpp>
#include <Poseidon/Graphics/Rendering/Draw/SpecLods.hpp>
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>
#include <Poseidon/Foundation/Types/Pointers.hpp>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

// Shapes.OptimizeAll() gives every shape a mission loads its vertex buffers.
// A shape first loaded later (createVehicle of a class the mission did not
// place: nearly every Guerrilla Mode vehicle) never got one, so Shape::Draw
// sent it down the CPU T&L path: classic per-vertex light baked into the
// vertices, no remastered per-pixel light, no shadow maps. ShapeBank now
// queues such shapes and OptimizeLate (once per drawn frame) gives them the
// same buffers OptimizeAll would have, except the crew views (vehicle
// interiors), which keep the CPU path. A recording engine stands in for the
// GL backend, so this runs without a GPU.

using namespace Poseidon;

namespace
{

class FakeVertexBuffer : public VertexBuffer
{
  public:
    void Update(const Shape& /*src*/, bool /*dynamic*/) override {}
};

// Hands out a buffer for every CreateVertexBuffer call, as the GL backend does,
// and remembers the type asked for per shape level.
class VertexBufferEngine : public EngineDummy
{
  public:
    std::map<const Shape*, VBType> types;

    VertexBuffer* CreateVertexBuffer(const Shape& src, VBType type) override
    {
        types[&src] = type;
        return new FakeVertexBuffer;
    }
};

struct GEngineGuard
{
    Engine* prev;
    explicit GEngineGuard(Engine* tmp) : prev(GEngine) { GEngine = tmp; }
    ~GEngineGuard() { GEngine = prev; }
};

// Hardware T&L on, the SSE quad path off, as on a GL client.
struct EngineConfigGuard
{
    bool hwtl = ENGINE_CONFIG.enableHWTL;
    bool piii = ENGINE_CONFIG.enablePIII;
    EngineConfigGuard()
    {
        ENGINE_CONFIG.enableHWTL = true;
        ENGINE_CONFIG.enablePIII = false;
    }
    ~EngineConfigGuard()
    {
        ENGINE_CONFIG.enableHWTL = hwtl;
        ENGINE_CONFIG.enablePIII = piii;
    }
};

std::string FixtureShape(const char* name)
{
    const std::filesystem::path root = std::filesystem::path(TESTS_ROOT_DIR).parent_path();
    return (root / "tests" / "fixtures" / "p3d" / name).generic_string();
}

// Which levels of the shape hold a buffer, and of which type (-1 = none).
std::vector<int> BufferLayout(const LODShape& shape, const VertexBufferEngine& engine)
{
    std::vector<int> layout;
    for (int l = 0; l < shape.NLevels(); l++)
    {
        const Shape* level = shape.Level(l);
        int entry = -1;
        if (level && level->GetVertexBuffer())
        {
            auto it = engine.types.find(level);
            entry = it != engine.types.end() ? static_cast<int>(it->second) : 99;
        }
        layout.push_back(entry);
    }
    return layout;
}

int BufferedLevels(const LODShape& shape)
{
    int n = 0;
    for (int l = 0; l < shape.NLevels(); l++)
    {
        if (shape.Level(l) && shape.Level(l)->GetVertexBuffer())
        {
            n++;
        }
    }
    return n;
}

} // namespace

TEST_CASE("A shape loaded after OptimizeAll gets the vertex buffers of a mission-loaded one",
          "[Graphics][remaster][Shape][VertexBuffer]")
{
    VertexBufferEngine engine;
    GEngineGuard engineGuard(&engine);
    EngineConfigGuard configGuard;

    Shapes.OptimizeAll(); // the mission has loaded

    // createVehicle of a class the mission did not place (this model has no
    // crew views, so every level follows OptimizeAll)
    Ref<LODShapeWithShadow> late = Shapes.New(FixtureShape("multi_lod_vehicle.p3d").c_str(), false, true);
    REQUIRE(late);
    REQUIRE(late->NLevels() > 0);
    // Nothing yet: the loader is still setting the shape up.
    CHECK(BufferedLevels(*late) == 0);

    Shapes.OptimizeLate(); // the next frame, before anything draws it
    const std::vector<int> lateLayout = BufferLayout(*late, engine);
    CHECK(BufferedLevels(*late) > 0);

    // Queued once: the frame after builds nothing more.
    const size_t built = engine.types.size();
    Shapes.OptimizeLate();
    CHECK(engine.types.size() == built);

    // The same levels and buffer types OptimizeAll gives it when the mission
    // itself loads the class (OptimizeAll releases and rebuilds every buffer).
    engine.types.clear();
    Shapes.OptimizeAll();
    CHECK(BufferLayout(*late, engine) == lateLayout);
}

TEST_CASE("Late vertex buffers follow the flags the loader sets after ShapeBank::New",
          "[Graphics][remaster][Shape][VertexBuffer]")
{
    VertexBufferEngine engine;
    GEngineGuard engineGuard(&engine);
    EngineConfigGuard configGuard;

    Shapes.OptimizeAll();
    Ref<LODShapeWithShadow> late = Shapes.New(FixtureShape("flat_quad.p3d").c_str(), false, true);
    REQUIRE(late);
    REQUIRE(late->NLevels() > 0);
    REQUIRE(late->LevelOpaque(0)->NFaces() > 0);

    SECTION("a light shape stays on its own lighting path")
    {
        late->OrSpecial(IsLight);
        Shapes.OptimizeLate();
        CHECK(BufferedLevels(*late) == 0);
    }

    SECTION("a sky shape stays on its own lighting path")
    {
        // what Landscape does to the sky dome right after loading it
        Shape* level = late->LevelOpaque(0);
        for (int i = 0; i < level->NPos(); i++)
        {
            level->SetClip(i, ClipLightSky | ClipFogDisable);
        }
        level->CalculateHints();
        late->CalculateHints();
        Shapes.OptimizeLate();
        CHECK(BufferedLevels(*late) == 0);
    }

    SECTION("a plain prop gets its buffer")
    {
        Shapes.OptimizeLate();
        CHECK(BufferedLevels(*late) > 0);
    }

    SECTION("a crew view (the gunner's interior) keeps the CPU path")
    {
        late->AddShape(new Shape(*late->LevelOpaque(0)), VIEW_GUNNER);
        const int gunner = late->FindSpecLevel(VIEW_GUNNER);
        REQUIRE(gunner > 0);
        Shapes.OptimizeLate();
        CHECK(late->Level(0)->GetVertexBuffer() != nullptr);
        CHECK(late->Level(gunner)->GetVertexBuffer() == nullptr);
    }
}

TEST_CASE("Late vertex buffers skip released shapes and shapes the mission loaded",
          "[Graphics][remaster][Shape][VertexBuffer]")
{
    VertexBufferEngine engine;
    GEngineGuard engineGuard(&engine);
    EngineConfigGuard configGuard;

    SECTION("a shape released before its first frame drops out of the queue")
    {
        Shapes.OptimizeAll();
        {
            Ref<LODShapeWithShadow> gone = Shapes.New(FixtureShape("flat_quad.p3d").c_str(), true, true);
            REQUIRE(gone);
        }
        Shapes.OptimizeLate();
        CHECK(engine.types.empty());
    }

    SECTION("a shape loaded before OptimizeAll is optimised once, by OptimizeAll")
    {
        Ref<LODShapeWithShadow> early = Shapes.New(FixtureShape("multi_lod_vehicle.p3d").c_str(), true, true);
        REQUIRE(early);
        Shapes.OptimizeAll();
        const int built = static_cast<int>(engine.types.size());
        CHECK(BufferedLevels(*early) > 0);
        Shapes.OptimizeLate();
        CHECK(static_cast<int>(engine.types.size()) == built);
    }
}
