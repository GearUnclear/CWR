// Generated occupier bases, step 2: the outpost layout (OCCUPIER-FORCES.md
// 5.7, ISLAND-PACKS.md 11).  The pure planner on a fake world (the rotation
// with the heading, the yard, the road margin and the mirror, missing
// classes), the service (one layout per base, a deleted prop re-created
// once, a reload never duplicating), the shipped layout's contract, the
// garrison nudge and the LZ exclusion.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <Poseidon/Core/SaveVersion.hpp> // WorldSerializeVersion
#include <Poseidon/Game/Guerrilla/BaseGenerate.hpp>
#include <Poseidon/Game/Guerrilla/BaseProps.hpp>
#include <Poseidon/Game/Guerrilla/OccupierAir.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <map>
#include <string.h>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;
using Catch::Approx;

namespace
{

std::string S(const RString& s)
{
    return std::string((const char*)s);
}

float D(float ax, float az, float bx, float bz)
{
    return sqrtf((ax - bx) * (ax - bx) + (az - bz) * (az - bz));
}

// A flat, dry world: every class spawns with a 2 x 2 m half-extent box unless
// listed missing; an optional east-west road at z = roadZ (or a north-south
// one at x = roadX); trees as points of radius treeR.  The "objects" live in
// a map keyed by base and row, so they outlast a save like world objects do.
struct FakeWorld final : BasePropWorld
{
    std::vector<std::string> missing;
    bool roadEW = false;
    float roadZ = 0;
    bool roadNS = false;
    float roadX = 0;
    std::vector<std::pair<float, float>> trees;
    float treeR = 1.0f;
    std::vector<std::pair<float, float>> walls; // solid map objects (the lane applies)
    float wallR = 1.0f;
    std::vector<std::pair<std::string, std::string>> faction; // the occupier descriptor's keys
    float half = 2.0f;
    mutable std::map<std::string, int> live;
    mutable int creates = 0;
    mutable int boxCalls = 0;
    mutable int next = 0;

    static std::string Key(const BasePropState& s) { return S(s.base) + "#" + std::to_string(s.row); }

    bool Spawnable(const char* cls) const override
    {
        for (const std::string& m : missing)
        {
            if (stricmp(m.c_str(), cls) == 0)
            {
                return false;
            }
        }
        return true;
    }
    bool Box(const char* /*cls*/, BasePropBox& out) const override
    {
        boxCalls++;
        out.minX = -half;
        out.maxX = half;
        out.minZ = -half;
        out.maxZ = half;
        return true;
    }
    bool NearRoad(float x, float z, float clearance) const override
    {
        return (roadEW && fabsf(z - roadZ) < clearance) || (roadNS && fabsf(x - roadX) < clearance);
    }
    bool Wet(float, float) const override { return false; }
    bool Blocked(float x, float z, float r, float lane) const override
    {
        for (const auto& t : trees)
        {
            if (D(x, z, t.first, t.second) < r + treeR)
            {
                return true;
            }
        }
        // solids (walls, gate posts) keep the lane as well
        for (const auto& w : walls)
        {
            if (D(x, z, w.first, w.second) < r + wallR + lane)
            {
                return true;
            }
        }
        return false;
    }
    RString FactionClass(const char* key) const override
    {
        for (const auto& f : faction)
        {
            if (stricmp(f.first.c_str(), key) == 0)
            {
                return RString(f.second.c_str());
            }
        }
        return RString();
    }
    bool Exists(const BasePropState& s) const override { return live.count(Key(s)) > 0; }
    bool Create(BasePropState& s) const override
    {
        creates++;
        s.token = ++next;
        live[Key(s)] = s.token;
        return true;
    }
};

GeneratedBaseRow Base(const char* name, float x, float z, float heading, int seed = 77)
{
    GeneratedBaseRow b;
    b.name = name;
    b.type = "OUTPOST";
    b.x = x;
    b.z = z;
    b.heading = heading;
    b.layoutSeed = seed;
    return b;
}

BaseLayout TestLayout()
{
    BaseLayout l;
    l.name = "Outpost";
    l.type = "OUTPOST";
    auto row = [&l](const char* cls, float dx, float dz, float dir, bool jitter = false)
    {
        BasePropRow r;
        r.cls = cls;
        r.dx = dx;
        r.dz = dz;
        r.dir = dir;
        r.jitter = jitter;
        l.rows.Add(r);
    };
    row("Fortress1", -10, 40, 0);
    row("Fortress1", 10, 40, 0);
    row("ViewTower", -34, 34, 0);
    row("ViewTower", 34, -34, 180);
    row("Wire", -46, 12, 90);
    row("Wire", -46, -10, 90);
    row("Wire", 46, 12, 90);
    row("Wire", 46, -10, 90);
    row("CampEast|Camp", -14, -36, 0, true);
    row("Camp", 6, -38, 0, true);
    row("FlagCarrier", -2, -29, 0);
    row("FuelStationNew2", 32, 20, 270);
    return l;
}

std::filesystem::path ConfigDir()
{
    return std::filesystem::path(TESTS_ROOT_DIR).parent_path() / "guerrilla-mode" / "config";
}

} // namespace

TEST_CASE("BaseProps - the layout rotates with the heading", "[guerrilla][baseprops]")
{
    float x = 0, z = 0;
    // heading 0 (the road to the north): local = world offsets
    BaseProps::LocalToWorld(1000, 2000, 0, 10, 40, x, z);
    CHECK(x == Approx(1010));
    CHECK(z == Approx(2040));
    // heading 90 (the road to the east): +dz runs east, +dx runs south
    BaseProps::LocalToWorld(1000, 2000, 90, 10, 40, x, z);
    CHECK(x == Approx(1040).margin(1e-3));
    CHECK(z == Approx(1990).margin(1e-3));
    // heading 180: everything mirrored through the centre
    BaseProps::LocalToWorld(1000, 2000, 180, 10, 40, x, z);
    CHECK(x == Approx(990).margin(1e-3));
    CHECK(z == Approx(1960).margin(1e-3));

    FakeWorld world;
    const BaseLayout layout = TestLayout();
    for (float heading : {0.0f, 37.0f, 90.0f, 211.0f})
    {
        AutoArray<BasePropState> out;
        const GeneratedBaseRow base = Base("Hill Outpost", 3000, 4000, heading);
        REQUIRE(BaseProps::PlanLayout(layout, base, world, out, nullptr) == 12);
        for (int i = 0; i < out.Size(); i++)
        {
            const BasePropRow& row = layout.rows[out[i].row];
            float ex = 0, ez = 0;
            BaseProps::LocalToWorld(base.x, base.z, heading, row.dx, row.dz, ex, ez);
            CHECK(out[i].x == Approx(ex).margin(1e-2));
            CHECK(out[i].z == Approx(ez).margin(1e-2));
            // the direction turns with the heading; jitter rows within 5 degrees
            float want = fmodf(heading + row.dir, 360.0f);
            float diff = fabsf(out[i].dir - want);
            diff = diff > 180 ? 360 - diff : diff;
            CHECK(diff <= (row.jitter ? 5.0f + 1e-3f : 1e-3f));
        }
        // the gate faces the road: both gate bunkers sit on the road side
        const float rx = base.x + sinf(heading * 3.14159265f / 180.0f) * 100.0f;
        const float rz = base.z + cosf(heading * 3.14159265f / 180.0f) * 100.0f;
        CHECK(D(out[0].x, out[0].z, rx, rz) < D(out[3].x, out[3].z, rx, rz));
    }
    // jitter is seeded: the same seed and row give the same turn
    CHECK(BaseProps::JitterDeg(77, 8, 5) == BaseProps::JitterDeg(77, 8, 5));
    CHECK(fabsf(BaseProps::JitterDeg(77, 8, 5)) <= 5.0f);
    CHECK(BaseProps::JitterDeg(77, 8, 5) != BaseProps::JitterDeg(78, 8, 5));
}

TEST_CASE("BaseProps - the yard and the road margin hold", "[guerrilla][baseprops]")
{
    const BaseLayout layout = TestLayout();
    SECTION("a row authored inside the yard is refused")
    {
        BaseLayout l = layout;
        BasePropRow r;
        r.cls = "Camp";
        r.dx = 5;
        r.dz = 10;
        l.rows.Add(r);
        FakeWorld world;
        AutoArray<BasePropState> out;
        std::vector<std::string> notes;
        CHECK(BaseProps::PlanLayout(l, Base("A", 0, 0, 0), world, out, &notes) == 12);
        bool refused = false;
        for (const std::string& n : notes)
        {
            refused = refused || n.find("yard") != std::string::npos;
        }
        CHECK(refused);
        for (int i = 0; i < out.Size(); i++)
        {
            CHECK(D(out[i].x, out[i].z, 0, 0) - out[i].box.Radius() >= l.yard);
        }
    }
    SECTION("a road along one flank: those rows mirror, nothing within the margin")
    {
        // heading 0, a north-south road 40 m east of the centre takes the
        // right flank: the fuel point and the rear tower mirror to the left;
        // the right wire's mirror lands on the left wire, so it is pushed
        // out 4 m on its own side instead, past the road margin
        FakeWorld world;
        world.roadNS = true;
        world.roadX = 40;
        AutoArray<BasePropState> out;
        std::vector<std::string> notes;
        const int n = BaseProps::PlanLayout(layout, Base("B", 0, 0, 0), world, out, &notes);
        CHECK(n == 12);
        for (int i = 0; i < out.Size(); i++)
        {
            CHECK(fabsf(out[i].x - 40.0f) >= out[i].box.Radius() + layout.roadMargin);
        }
        bool mirrored = false, pushed = false;
        for (const std::string& s : notes)
        {
            mirrored = mirrored || s.find("mirrored") != std::string::npos;
            pushed = pushed || s.find("pushed out 4 m") != std::string::npos;
        }
        CHECK(mirrored);
        CHECK(pushed);
        // a pushed row still stands within 60 m of the centre
        for (int i = 0; i < out.Size(); i++)
        {
            CHECK(D(out[i].x, out[i].z, 0, 0) < BaseProps::LzClear);
        }
    }
    SECTION("a road across the gate side drops the rows it touches, never moves them onto it")
    {
        FakeWorld world;
        world.roadEW = true;
        world.roadZ = 44; // through the gate bunkers (dz 40) at heading 0
        AutoArray<BasePropState> out;
        const int n = BaseProps::PlanLayout(layout, Base("C", 0, 0, 0), world, out, nullptr);
        CHECK(n == 10); // both gate bunkers dropped (their mirrors and push-outs are on the road too)
        for (int i = 0; i < out.Size(); i++)
        {
            CHECK(fabsf(out[i].z - 44.0f) >= out[i].box.Radius() + layout.roadMargin);
        }
    }
    SECTION("a tree in the footprint mirrors the row")
    {
        FakeWorld world;
        world.trees.push_back({-34.0f, 34.0f}); // under the front-left tower
        AutoArray<BasePropState> out;
        REQUIRE(BaseProps::PlanLayout(layout, Base("D", 0, 0, 0), world, out, nullptr) == 12);
        CHECK(out[2].x == Approx(34.0f)); // mirrored to the front-right corner
        CHECK(out[2].z == Approx(34.0f));
    }
}

TEST_CASE("BaseProps - a missing class is resolved or skipped, never thrown", "[guerrilla][baseprops]")
{
    const BaseLayout layout = TestLayout();
    FakeWorld world;
    CHECK(S(BaseProps::ResolveClass("CampEast|Camp", world)) == "CampEast");
    world.missing = {"CampEast"};
    CHECK(S(BaseProps::ResolveClass("CampEast|Camp", world)) == "Camp");
    CHECK(S(BaseProps::ResolveClass(" CampEast | Camp ", world)) == "Camp");
    world.missing = {"CampEast", "Camp", "ViewTower"};
    CHECK(S(BaseProps::ResolveClass("CampEast|Camp", world)).empty());
    AutoArray<BasePropState> out;
    std::vector<std::string> notes;
    int n = -1;
    REQUIRE_NOTHROW(n = BaseProps::PlanLayout(layout, Base("E", 0, 0, 0), world, out, &notes));
    CHECK(n == 8); // two towers and two tents skipped
    int skipped = 0;
    for (const std::string& s : notes)
    {
        skipped += s.find("no spawnable class") != std::string::npos ? 1 : 0;
    }
    CHECK(skipped == 4);
}

TEST_CASE("BaseProps - one layout per base, a deleted prop re-created once", "[guerrilla][baseprops]")
{
    BaseProps props;
    // the layouts through the parser, as the engine reads them
    const char* cfg = "class CfgGuerrillaBaseLayouts\n"
                      "{\n"
                      "    class Outpost\n"
                      "    {\n"
                      "        type = \"OUTPOST\";\n"
                      "        yard = 25;\n"
                      "        items[] = {{\"Fortress1\", -10, 40, 0}, {\"Fortress1\", 10, 40, 0},\n"
                      "                   {\"ViewTower\", -34, 34, 0}, {\"ViewTower\", 34, -34, 180},\n"
                      "                   {\"Wire\", -46, 12, 90}, {\"Wire\", -46, -10, 90},\n"
                      "                   {\"Wire\", 46, 12, 90}, {\"Wire\", 46, -10, 90},\n"
                      "                   {\"Camp\", -14, -36, 0, 1}, {\"Camp\", 6, -38, 0, 1}};\n"
                      "    };\n"
                      "};\n";
    ParamFile file;
    QIStream in(cfg, strlen(cfg));
    file.Parse(in);
    props.LoadLayouts(file.FindEntry("CfgGuerrillaBaseLayouts"));
    REQUIRE(props.NLayouts() == 1);
    REQUIRE(props.FindLayout("OUTPOST") != nullptr);
    CHECK(props.FindLayout("OUTPOST")->rows.Size() == 10);
    CHECK(props.FindLayout("OUTPOST")->rows[8].jitter);
    CHECK(props.FindLayout("CHECKPOINT") == nullptr);

    FakeWorld world;
    GeneratedBaseRow a = Base("Vigny Outpost", 4175, 7025, 300);
    GeneratedBaseRow b = Base("La Riviere Outpost", 3975, 4775, 45);
    GeneratedBaseRow c = Base("Hill 80 Checkpoint", 6000, 6000, 0);
    c.type = "CHECKPOINT"; // no layout: a bare zone
    GeneratedBaseRow d = Base("Detected Outpost", 8000, 8000, 0);
    d.detected = true; // real map props already stand there
    const GeneratedBaseRow* bases[] = {&a, &b, &c, &d};
    CHECK(props.Sync(bases, 4, world) == 20);
    CHECK(props.NPropsOf("Vigny Outpost") == 10);
    CHECK(props.NPropsOf("La Riviere Outpost") == 10);
    CHECK(props.NPropsOf("Hill 80 Checkpoint") == 0);
    CHECK(props.NPropsOf("Detected Outpost") == 0);
    CHECK(props.IsPlanned("Hill 80 Checkpoint"));
    // a second pass plans nothing and creates nothing
    const int boxes = world.boxCalls;
    CHECK(props.Sync(bases, 4, world) == 0);
    CHECK(world.boxCalls == boxes);
    CHECK(world.creates == 20);

    // a deleted prop comes back once, at its planned spot
    const BasePropState victim = *props.PropAt(3);
    world.live.erase(FakeWorld::Key(victim));
    CHECK(props.Sync(bases, 4, world) == 1);
    CHECK(props.Sync(bases, 4, world) == 0);
    CHECK(world.creates == 21);
    CHECK(props.NProps() == 20);
    CHECK(props.PropAt(3)->x == victim.x);
    CHECK(props.PropAt(3)->z == victim.z);
    CHECK(props.PropAt(3)->made == 2);
}

TEST_CASE("BaseProps - a reload re-binds the saved rows and never duplicates", "[guerrilla][baseprops][save][load]")
{
    const std::filesystem::path dir = std::filesystem::current_path() / "tmp";
    std::filesystem::create_directories(dir);
    const std::filesystem::path archivePath = dir / "base-props-roundtrip.bin";

    FakeWorld world; // the world objects outlast the save, like _buildings
    const BaseLayout layout = TestLayout();
    GeneratedBaseRow a = Base("Vigny Outpost", 4175, 7025, 300);
    const GeneratedBaseRow* bases[] = {&a};
    std::vector<std::pair<float, float>> saved;
    {
        BaseProps props;
        // a layout set by hand (the parser has its own case)
        props.LoadLayouts(nullptr);
        ParamFile file;
        const char* cfg = "class L { class Outpost { type = \"OUTPOST\"; items[] = {{\"Fortress1\", -10, 40, 0}, "
                          "{\"Fortress1\", 10, 40, 0}, {\"ViewTower\", -34, 34, 0}, {\"ViewTower\", 34, -34, 180}, "
                          "{\"Wire\", -46, 12, 90}, {\"Wire\", -46, -10, 90}, {\"Wire\", 46, 12, 90}, "
                          "{\"Wire\", 46, -10, 90}, {\"Camp\", -14, -36, 0, 1}}; }; };\n";
        QIStream in(cfg, strlen(cfg));
        file.Parse(in);
        props.LoadLayouts(file.FindEntry("L"));
        REQUIRE(props.Sync(bases, 1, world) == 9);
        for (int i = 0; i < props.NProps(); i++)
        {
            saved.push_back({props.PropAt(i)->x, props.PropAt(i)->z});
        }
        ParamArchiveSave ar(WorldSerializeVersion);
        REQUIRE(props.Serialize(ar) == LSOK);
        REQUIRE(ar.SaveBin(archivePath.string().c_str()));
    }
    // a fresh process: its own layouts may differ (a newer build), the saved
    // rows win and nothing is planned again
    BaseProps loaded;
    {
        ParamArchiveLoad ar;
        REQUIRE(ar.LoadBin(archivePath.string().c_str()));
        ar.FirstPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
        ar.SecondPass();
        REQUIRE(loaded.Serialize(ar) == LSOK);
    }
    REQUIRE(loaded.NProps() == 9);
    CHECK(loaded.IsPlanned("Vigny Outpost"));
    for (int i = 0; i < loaded.NProps(); i++)
    {
        CHECK(loaded.PropAt(i)->x == Approx(saved[i].first));
        CHECK(loaded.PropAt(i)->z == Approx(saved[i].second));
        CHECK(loaded.PropAt(i)->box.Radius() > 0);
    }
    const int creates = world.creates;
    const int boxes = world.boxCalls;
    CHECK(loaded.Sync(bases, 1, world) == 0);
    CHECK(world.creates == creates);
    CHECK(world.boxCalls == boxes);
    CHECK(loaded.NProps() == 9);
    std::filesystem::remove(archivePath);
}

TEST_CASE("BaseProps - the shipped outpost layout meets the contract", "[guerrilla][baseprops]")
{
    ParamFile pf;
    pf.Parse((ConfigDir() / "guerrilla-hq.hpp").string().c_str());
    BaseProps props;
    props.LoadLayouts(pf.FindEntry("CfgGuerrillaBaseLayouts"));
    const BaseLayout* outpost = props.FindLayout("OUTPOST");
    REQUIRE(outpost != nullptr);
    CHECK(outpost->rows.Size() >= 8);
    CHECK(outpost->rows.Size() <= 12);
    CHECK(outpost->yard == Approx(25.0f));
    CHECK(outpost->roadMargin == Approx(6.0f));
    bool flag = false, tower = false, wire = false, tent = false, gate = false;
    for (int i = 0; i < outpost->rows.Size(); i++)
    {
        const BasePropRow& r = outpost->rows[i];
        const std::string c = S(r.cls);
        flag = flag || c == "FlagCarrier";
        tower = tower || c.find("ViewTower") != std::string::npos; // "$basePropTower|ViewTower"
        wire = wire || c == "Wire";
        tent = tent || c.find("Camp") != std::string::npos;
        gate = gate || (c == "Fortress1" && r.dz > 30);
        const float d = sqrtf(r.dx * r.dx + r.dz * r.dz);
        CHECK(d > outpost->yard); // the yard stays clear
        CHECK(d <= 52.0f);        // a row pushed 8 m still stands within 60 m (ISLAND-PACKS.md 11)
    }
    CHECK(flag);
    CHECK(tower);
    CHECK(wire);
    CHECK(tent);
    CHECK(gate);
    // on flat empty ground the whole layout places
    FakeWorld world;
    AutoArray<BasePropState> out;
    CHECK(BaseProps::PlanLayout(*outpost, Base("F", 5000, 5000, 120), world, out, nullptr) == outpost->rows.Size());
}

TEST_CASE("BaseProps - every generated kind has its own shipped layout", "[guerrilla][baseprops]")
{
    ParamFile pf;
    pf.Parse((ConfigDir() / "guerrilla-hq.hpp").string().c_str());
    BaseProps props;
    props.LoadLayouts(pf.FindEntry("CfgGuerrillaBaseLayouts"));
    CHECK(props.NLayouts() == 4);
    const char* types[] = {"OUTPOST", "CHECKPOINT", "COASTAL", "HELIPAD"};
    std::vector<std::string> names;
    for (const char* type : types)
    {
        INFO(type);
        const BaseLayout* l = props.FindLayout(type);
        REQUIRE(l != nullptr);
        names.push_back(S(l->name));
        CHECK(l->rows.Size() >= l->minProps);
        CHECK(l->rows.Size() <= 12);
        CHECK(l->minProps >= 6);
        CHECK(l->lane == Approx(4.0f));
        CHECK(l->roadMargin == Approx(6.0f));
        bool flag = false, tower = false;
        for (int i = 0; i < l->rows.Size(); i++)
        {
            const BasePropRow& r = l->rows[i];
            const std::string c = S(r.cls);
            flag = flag || c == "FlagCarrier";
            tower = tower || c.find("ViewTower") != std::string::npos;
            const float d = sqrtf(r.dx * r.dx + r.dz * r.dz);
            CHECK(d > l->yard);
            CHECK(d <= 52.0f);
            // a "$key" alternate always has a stock class after it
            if (c[0] == '$')
            {
                CHECK(c.find('|') != std::string::npos);
            }
        }
        CHECK(flag);
        CHECK(tower);
        // on flat empty ground every row places, whatever the heading
        for (float heading : {0.0f, 77.0f, 200.0f})
        {
            FakeWorld world;
            AutoArray<BasePropState> out;
            GeneratedBaseRow base = Base("K", 5000, 5000, heading);
            base.type = type;
            CHECK(BaseProps::PlanLayout(*l, base, world, out, nullptr) == l->rows.Size());
        }
    }
    // four distinct layouts, not one reused
    std::sort(names.begin(), names.end());
    CHECK(std::unique(names.begin(), names.end()) == names.end());
    // a checkpoint plans its own rows, not the outpost's
    FakeWorld world;
    AutoArray<BasePropState> out;
    GeneratedBaseRow cp = Base("Gate Checkpoint", 5000, 5000, 0);
    cp.type = "CHECKPOINT";
    const BaseLayout* layout = props.FindLayout(cp.type);
    REQUIRE(layout != nullptr);
    BaseProps::PlanLayout(*layout, cp, world, out, nullptr);
    CHECK(out.Size() == props.FindLayout("CHECKPOINT")->rows.Size());
    CHECK(out.Size() != props.FindLayout("OUTPOST")->rows.Size());
}

TEST_CASE("BaseProps - a $key row takes the occupier faction's style, else the stock class", "[guerrilla][baseprops]")
{
    FakeWorld world;
    CHECK(S(BaseProps::ResolveClass("$basePropTower|ViewTower", world)) == "ViewTower"); // no key
    world.faction.push_back({"basePropTower", "Brown_Turm"});
    CHECK(S(BaseProps::ResolveClass("$basePropTower|ViewTower", world)) == "Brown_Turm");
    world.missing.push_back("Brown_Turm"); // a package without @LoBo
    CHECK(S(BaseProps::ResolveClass("$basePropTower|ViewTower", world)) == "ViewTower");
    world.missing.push_back("ViewTower");
    CHECK(S(BaseProps::ResolveClass("$basePropTower|ViewTower", world)).empty()); // skipped, never thrown
    CHECK(S(BaseProps::ResolveClass("$", world)).empty());
}

TEST_CASE("BaseProps - a prop keeps a lane from a solid map object, not from a tree", "[guerrilla][baseprops]")
{
    BaseLayout l;
    l.name = "One";
    l.type = "OUTPOST";
    BasePropRow row;
    row.cls = "Fortress1";
    row.dx = 0;
    row.dz = 40;
    l.rows.Add(row);
    const GeneratedBaseRow base = Base("Lane", 5000, 5000, 0); // the row lands at [5000, 5040]
    // a tree 3.5 m from the prop's footprint edge: no matter
    {
        FakeWorld world;
        world.trees.push_back({5000.0f + 2.0f * 1.4143f + 1.0f + 3.5f, 5040.0f});
        AutoArray<BasePropState> out;
        REQUIRE(BaseProps::PlanLayout(l, base, world, out, nullptr) == 1);
        CHECK(out[0].x == Approx(5000.0f));
    }
    // a wall post at the same spot: inside the 4 m lane, so the row moves
    // (pushed outward: a gap a soldier walks through is never closed)
    {
        FakeWorld world;
        world.walls.push_back({5000.0f + 2.0f * 1.4143f + 1.0f + 3.5f, 5040.0f});
        AutoArray<BasePropState> out;
        std::vector<std::string> notes;
        REQUIRE(BaseProps::PlanLayout(l, base, world, out, &notes) == 1);
        CHECK(out[0].z > 5040.0f);
        CHECK(D(out[0].x, out[0].z, world.walls[0].first, world.walls[0].second) >=
              out[0].box.Radius() + world.wallR + l.lane);
    }
    // a lane of 0 (an author's choice) allows it
    {
        FakeWorld world;
        world.walls.push_back({5000.0f + 2.0f * 1.4143f + 1.0f + 3.5f, 5040.0f});
        BaseLayout tight = l;
        tight.lane = 0;
        AutoArray<BasePropState> out;
        REQUIRE(BaseProps::PlanLayout(tight, base, world, out, nullptr) == 1);
        CHECK(out[0].z == Approx(5040.0f));
    }
}

TEST_CASE("BaseProps - a garrison point inside a prop box is nudged clear", "[guerrilla][baseprops]")
{
    BaseProps props;
    ParamFile file;
    const char* cfg = "class L { class Outpost { type = \"OUTPOST\"; items[] = {{\"ViewTower\", -34, 34, 30}, "
                      "{\"Wire\", 0, -40, 90}, {\"Camp\", 3, -36, 0}}; }; };\n";
    QIStream in(cfg, strlen(cfg));
    file.Parse(in);
    props.LoadLayouts(file.FindEntry("L"));
    FakeWorld world;
    world.half = 3.0f;
    GeneratedBaseRow a = Base("G", 1000, 1000, 75);
    const GeneratedBaseRow* bases[] = {&a};
    // the tent overlaps the wire's radius: dropped as overlap
    REQUIRE(props.Sync(bases, 1, world) == 2);
    for (int i = 0; i < props.NProps(); i++)
    {
        const BasePropState& s = *props.PropAt(i);
        float x = s.x + 0.4f, z = s.z - 0.7f; // inside
        REQUIRE(BaseProps::InsideBox(s, x, z, 0));
        CHECK(props.NudgeClear(x, z));
        for (int k = 0; k < props.NProps(); k++)
        {
            CHECK_FALSE(BaseProps::InsideBox(*props.PropAt(k), x, z, BaseProps::NudgeMargin));
        }
        // the nudge is short: out of a 6 m box plus the margin
        CHECK(D(x, z, s.x, s.z) < 3.0f * 1.5f + BaseProps::NudgeMargin + 1.0f);
    }
    float fx = 1000, fz = 1000; // the yard is clear: nothing to do
    CHECK_FALSE(props.NudgeClear(fx, fz));
    CHECK(fx == 1000);
    CHECK(fz == 1000);
}

namespace
{
// a flat LZ probe that refuses what the generated-base exclusion refuses
struct ExclusionProbe final : AirLzProbe
{
    const BaseProps* props = nullptr;
    AirLzFacts Probe(Vector3Par pos, float /*clearRadius*/) const override
    {
        AirLzFacts f;
        f.landY = 20.0f;
        f.objects = props && props->ExcludesLz(pos.X(), pos.Z()) ? 1 : 0;
        return f;
    }
};
} // namespace

TEST_CASE("BaseProps - the LZ probe lands outside 60 m of a generated base's props", "[guerrilla][baseprops][air]")
{
    // the target and the ring's first candidate: PickLzPure's first try is
    // straight out along +x at the ring's mid radius
    AirTuning t;
    float off = 0, r = 0;
    OccupierAir::LzCandidate(0, t, off, r);
    const Vector3 target(5000, 0, 5000);
    const float firstX = 5000 + cosf(off) * r;
    const float firstZ = 5000 + sinf(off) * r;

    BaseProps props;
    ParamFile file;
    const char* cfg = "class L { class Outpost { type = \"OUTPOST\"; items[] = {{\"Fortress1\", -10, 40, 0}, "
                      "{\"ViewTower\", 34, -34, 0}, {\"Camp\", -14, -36, 0}}; }; };\n";
    QIStream in(cfg, strlen(cfg));
    file.Parse(in);
    props.LoadLayouts(file.FindEntry("L"));
    FakeWorld world;
    GeneratedBaseRow a = Base("H", firstX, firstZ, 0); // the outpost on the first candidate
    const GeneratedBaseRow* bases[] = {&a};
    REQUIRE(props.Sync(bases, 1, world) == 3);
    REQUIRE(props.ExcludesLz(firstX, firstZ));

    ExclusionProbe probe;
    Vector3 lz;
    RString why;
    // without the exclusion the first candidate wins, among the props
    REQUIRE(OccupierAir::PickLzPure(target, nullptr, t, probe, lz, why));
    CHECK(D(lz.X(), lz.Z(), firstX, firstZ) < 1.0f);
    probe.props = &props;
    REQUIRE(OccupierAir::PickLzPure(target, nullptr, t, probe, lz, why));
    for (int i = 0; i < props.NProps(); i++)
    {
        CHECK(D(lz.X(), lz.Z(), props.PropAt(i)->x, props.PropAt(i)->z) >= BaseProps::LzClear);
    }
    const float dt = D(lz.X(), lz.Z(), target.X(), target.Z());
    CHECK(dt >= t.lzMin - 1.0f);
    CHECK(dt <= t.lzMax + 1.0f);
}
