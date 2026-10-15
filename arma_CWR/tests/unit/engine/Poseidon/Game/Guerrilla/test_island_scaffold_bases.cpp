// Occupation-network auto-detection (issue #76): the pure core shared by the
// runtime base seed (ZoneRegistry, seedBases) and `PoseidonTools guerrilla
// scaffold / lint`.  Synthetic ILS tables and object lists modelled on the
// Remaster's Malden (abel.wrp): the runway and taxi paths of CfgWorlds >> Abel,
// the Le Port quay pair (molo_beton + molo_krychle on the shore at x 8475) and
// the Southeast Compound's sheds and watchtowers.

#include <catch2/catch_test_macros.hpp>

#include <Poseidon/Game/Guerrilla/BaseDetect.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/platform.hpp>

#include <cmath>
#include <string>
#include <vector>

using namespace Poseidon;
using namespace Poseidon::Guerrilla;

namespace
{

std::string Str(const RString& s)
{
    return std::string((const char*)s);
}

float Dist(float ax, float az, float bx, float bz)
{
    return sqrtf((ax - bx) * (ax - bx) + (az - bz) * (az - bz));
}

// Malden in miniature: 12.8 km of dry ground at 30 m, sea east of x = 8475
// south of z = 4300 (the Le Port shore), sea everywhere east of x = 12000,
// and a 300 m lagoon round (2000, 2000) for the inland and water cases.
struct FakeMalden final : HeightField
{
    float Height(float x, float z) const override
    {
        if (x < 0 || z < 0 || x > 12800 || z > 12800)
        {
            return -100.0f;
        }
        if (x > 12000.0f)
        {
            return -10.0f;
        }
        if (z < 4300.0f && x > 8475.0f && x < 9000.0f)
        {
            return -3.0f; // the harbour basin
        }
        if (z < 4300.0f && x > 8375.0f && x <= 8475.0f)
        {
            return (8475.0f - x) * 0.08f; // the quay slope: 0 at the water, 8 m 100 m inland
        }
        if (Dist(x, z, 2000, 2000) < 300.0f)
        {
            return -4.0f;
        }
        return 30.0f;
    }
};

FakeMalden gMalden;

struct Parsed
{
    ParamFile file;
    explicit Parsed(const std::string& text)
    {
        QIStream in(text.c_str(), text.size());
        file.Parse(in);
    }
};

// CfgWorlds >> Abel's ILS block with its expressions evaluated (the raw
// parser used here has no expression evaluator; the engine's own reader is
// exercised on the real config by the tool and tri runs)
const char* kAbelIls = "ilsPosition[]={7922,9590,0};\n"
                       "ilsDirection[]={0,0.08,-1};\n"
                       "ilsTaxiIn[]={7991,9990, 7991,9617, 7983,9609, 7930,9609, 7922,9617, 7922,9672};\n"
                       "ilsTaxiOff[]={7922,9590, 7922,10450, 7930,10458, 7983,10458, 7991,10450, 7991,9990};\n";

std::vector<IlsEntry> AbelIls()
{
    Parsed p(std::string("class CfgWorlds { class Abel {\n") + kAbelIls + "}; };\n");
    std::vector<IlsEntry> ils;
    ReadIlsEntries(*p.file.FindEntry("CfgWorlds")->FindEntry("Abel"), ils);
    return ils;
}

DetectObject Obj(const char* model, float x, float z, float radius = 0)
{
    DetectObject o;
    o.model = model;
    o.x = x;
    o.z = z;
    o.radius = radius;
    return o;
}

void AddLePortQuay(std::vector<DetectObject>& objs)
{
    objs.push_back(Obj("data3d\\molo_beton.p3d", 8476.3f, 3754.1f));
    objs.push_back(Obj("data3d\\molo_krychle.p3d", 8472.0f, 3774.2f));
}

void AddCompound(std::vector<DetectObject>& objs)
{
    objs.push_back(Obj("data3d\\hlaska.p3d", 9642.2f, 3867.2f));
    objs.push_back(Obj("data3d\\hlaska.p3d", 9638.8f, 4005.9f));
    objs.push_back(Obj("data3d\\garaz_bez_tanku.p3d", 9691.0f, 4002.0f));
    objs.push_back(Obj("data3d\\garaz_bez_tanku.p3d", 9709.0f, 4002.0f));
    objs.push_back(Obj("data3d\\garaz_s_tankem.p3d", 9745.0f, 4002.0f));
    objs.push_back(Obj("data3d\\strazni_vez.p3d", 9771.2f, 4005.8f));
    objs.push_back(Obj("data3d\\hlaska.p3d", 9791.6f, 3866.1f));
    objs.push_back(Obj("data3d\\hlaska.p3d", 9796.4f, 4007.0f));
}

BaseDetectInputs MaldenInputs()
{
    BaseDetectInputs in;
    in.worldClass = "Abel";
    in.worldDisplay = "Malden";
    in.height = &gMalden;
    in.worldSize = 12800.0f;
    in.towns.push_back(DetectPlace{"Le Port", 8193.0f, 3129.0f});
    in.towns.push_back(DetectPlace{"Larche", 9000.0f, 9000.0f});
    // houses round both, so they are settlements a base may be named after
    // (a bare Names label names nothing)
    for (int i = 0; i < 3; i++)
    {
        in.objects.push_back(Obj("data3d\\dum01.p3d", 8193.0f + 30.0f * (float)i, 3129.0f, 9.0f));
        in.objects.push_back(Obj("data3d\\dum01.p3d", 9000.0f + 30.0f * (float)i, 9000.0f, 9.0f));
    }
    return in;
}

const BaseCandidate* Find(const BaseDetectResult& r, const char* type, int nth = 0)
{
    for (const BaseCandidate& c : r.candidates)
    {
        if (Str(c.type) == type && nth-- == 0)
        {
            return &c;
        }
    }
    return nullptr;
}

std::string Notes(const BaseDetectResult& r)
{
    std::string s;
    for (const std::string& n : r.notes)
    {
        s += "\n  - " + n;
    }
    return s;
}

} // namespace

TEST_CASE("base model vocabulary: harbour and military weights", "[game][guerrilla][scaffold][bases]")
{
    CHECK(ClassifyBaseModel("data3d\\molo_beton.p3d").harbour == 1.0f);
    CHECK(ClassifyBaseModel("DATA3D\\MOLO_KRYCHLE.P3D").harbour == 1.0f);
    CHECK(ClassifyBaseModel("data3d\\molo_drevo_bs.p3d").harbour == 0.5f);
    CHECK(ClassifyBaseModel("ags_port\\kontejner1.p3d").harbour == 1.0f);
    CHECK(ClassifyBaseModel("x\\harbour_crane.p3d").harbour == 1.0f);
    // a road sign naming St. Pierre is not a pier, a church tower not a watchtower
    CHECK(ClassifyBaseModel("data3d\\znacka_spierre.p3d").harbour == 0.0f);
    CHECK(ClassifyBaseModel("data3d\\vez.p3d").military == 0.0f);
    CHECK(ClassifyBaseModel("data3d\\garaz.p3d").military == 0.0f);
    CHECK(ClassifyBaseModel("data3d\\dum01.p3d").military == 0.0f);
    CHECK(ClassifyBaseModel("data3d\\pletivo_wired.p3d").military == 0.0f);

    CHECK(ClassifyBaseModel("data3d\\budova1.p3d").military == 1.0f);
    CHECK(ClassifyBaseModel("data3d\\helfenburk_budova2.p3d").military == 0.0f);
    CHECK(ClassifyBaseModel("data3d\\hlaska.p3d").military == 1.0f);
    CHECK(ClassifyBaseModel("data3d\\fortress2.p3d").military == 1.0f);
    CHECK(ClassifyBaseModel("data3d\\garaz_s_tankem.p3d").military == 1.0f);
    CHECK(ClassifyBaseModel("data3d\\hangar.p3d").military == 1.0f);
    CHECK(ClassifyBaseModel("baracken\\barrack_a.p3d").military == 1.0f);
    CHECK(ClassifyBaseModel("").military == 0.0f);
    CHECK(ClassifyBaseModel(nullptr).harbour == 0.0f);
}

TEST_CASE("ReadIlsEntries reads the main runway, secondary airports and inheritance", "[game][guerrilla][scaffold][bases]")
{
    Parsed p("class CfgWorlds {\n"
             " class DefaultWorld { };\n"
             " class Abel: DefaultWorld {\n"
             "  ilsPosition[]={7922,9590,0}; ilsDirection[]={0,0.08,-1};\n"
             "  ilsTaxiIn[]={7991,9990, 7991,9617}; ilsTaxiOff[]={7922,9590, 7922,10450, 7991,10450};\n"
             "  class SecondaryAirports { class Strip2 { ilsPosition[]={3000,3000}; ilsDirection[]={1,0.08,0}; }; };\n"
             " };\n"
             " class Demo: Abel { };\n"
             " class Cain: DefaultWorld { };\n"
             "};\n");
    const ParamEntry* worlds = p.file.FindEntry("CfgWorlds");
    REQUIRE(worlds);

    std::vector<IlsEntry> ils;
    CHECK(ReadIlsEntries(*worlds->FindEntry("Abel"), ils) == 2);
    REQUIRE(ils.size() == 2);
    CHECK(ils[0].x == 7922.0f);
    CHECK(ils[0].z == 9590.0f);
    CHECK(ils[0].dirZ == -1.0f);
    CHECK(ils[0].taxiIn.size() == 2);
    CHECK(ils[0].taxiOff.size() == 3);
    CHECK(ils[0].taxiOff[1].z == 10450.0f);
    CHECK_FALSE(ils[0].inherited);
    CHECK(Str(ils[0].source) == "CfgWorlds >> Abel");
    CHECK(ils[1].x == 3000.0f);
    CHECK(ils[1].dirX == 1.0f);
    CHECK(Str(ils[1].source) == "CfgWorlds >> Abel >> SecondaryAirports >> Strip2");

    ils.clear();
    CHECK(ReadIlsEntries(*worlds->FindEntry("Demo"), ils) >= 1);
    REQUIRE_FALSE(ils.empty());
    CHECK(ils[0].inherited); // Demo only inherits Abel's runway

    ils.clear();
    CHECK(ReadIlsEntries(*worlds->FindEntry("Cain"), ils) == 0);
}

TEST_CASE("an ILS entry becomes an AIRFIELD on the apron, agreeing with Abel's authored Airfield",
          "[game][guerrilla][scaffold][bases]")
{
    BaseDetectInputs in = MaldenInputs();
    in.ils = AbelIls();
    REQUIRE(in.ils.size() == 1);
    BaseDetectOptions opt;
    opt.income = 25;
    BaseDetectResult r;
    DetectBases(in, opt, r);

    const BaseCandidate* af = Find(r, "AIRFIELD");
    REQUIRE(af);
    INFO(Notes(r));
    CHECK(af->status == BaseStatus::Seed);
    CHECK(Str(af->name) == "Malden Airfield");
    // the apron: the first ilsTaxiIn point
    CHECK(af->x == 7991.0f);
    CHECK(af->z == 9990.0f);
    CHECK(af->y == 30.0f);
    // Abel's hand-surveyed Airfield zone sits 94 m away
    CHECK(Dist(af->x, af->z, 8030.0f, 9905.0f) < 300.0f);
    // the footprint covers the 860 m strip
    CHECK(af->radius > 430.0f);
    CHECK(af->garrison == 12);
    CHECK(af->income == 25);
    CHECK(af->confidence >= 0.9f);
    REQUIRE(af->evidence.size() >= 2);
    CHECK(af->evidence[0].find("ilsPosition [7922, 9590]") != std::string::npos);

    // the authored zone wins
    in.authored.push_back(DetectAuthoredZone{"Airfield", "AIRFIELD", 8030.0f, 9905.0f});
    DetectBases(in, opt, r);
    af = Find(r, "AIRFIELD");
    REQUIRE(af);
    CHECK(af->status == BaseStatus::Covered);
    CHECK(Str(af->coveredBy) == "Airfield");

    // an authored post at the far end of the strip (inside the footprint,
    // beyond 300 m of the apron) still counts as the airfield's
    in.authored.clear();
    in.authored.push_back(DetectAuthoredZone{"North Post", "OUTPOST", 7950.0f, 10400.0f});
    DetectBases(in, opt, r);
    CHECK(Find(r, "AIRFIELD")->status == BaseStatus::Covered);

    // an authored CITY covers only within zoneArea
    in.authored.clear();
    in.authored.push_back(DetectAuthoredZone{"Airfield Town", "CITY", 8200.0f, 9990.0f});
    DetectBases(in, opt, r);
    CHECK(Find(r, "AIRFIELD")->status == BaseStatus::Seed);
}

TEST_CASE("ILS entries that are no airfield: origin, off the map, in the water, inherited",
          "[game][guerrilla][scaffold][bases]")
{
    BaseDetectInputs in = MaldenInputs();
    BaseDetectOptions opt;
    BaseDetectResult r;

    IlsEntry origin; // Cain and Intro: ilsPosition[]={0,0,0}
    origin.dirZ = -1;
    origin.source = "CfgWorlds >> Cain";
    in.ils = {origin};
    DetectBases(in, opt, r);
    CHECK(Find(r, "AIRFIELD") == nullptr);
    REQUIRE(r.notes.size() == 1);
    CHECK(r.notes[0].find("map origin") != std::string::npos);

    IlsEntry wet;
    wet.x = 2000;
    wet.z = 2000; // the lagoon
    wet.dirZ = -1;
    wet.source = "CfgWorlds >> Wet";
    in.ils = {wet};
    DetectBases(in, opt, r);
    CHECK(Find(r, "AIRFIELD") == nullptr);

    IlsEntry off = wet;
    off.x = 20000;
    in.ils = {off};
    DetectBases(in, opt, r);
    CHECK(Find(r, "AIRFIELD") == nullptr);

    // no taxi paths: the strip runs 860 m against the approach direction and
    // the centre is its midpoint
    IlsEntry bare;
    bare.x = 5000;
    bare.z = 5000;
    bare.dirZ = -1;
    bare.source = "CfgWorlds >> Bare";
    in.ils = {bare};
    DetectBases(in, opt, r);
    const BaseCandidate* af = Find(r, "AIRFIELD");
    REQUIRE(af);
    CHECK(af->x == 5000.0f);
    CHECK(af->z == 5430.0f);
    CHECK(af->status == BaseStatus::Seed);

    // inherited from a base world: reported, never seeded
    bare.inherited = true;
    in.ils = {bare};
    DetectBases(in, opt, r);
    af = Find(r, "AIRFIELD");
    REQUIRE(af);
    CHECK(af->status == BaseStatus::LowConfidence);
}

TEST_CASE("pier clusters become a SEAPORT on dry ground, agreeing with Le Port Harbour",
          "[game][guerrilla][scaffold][bases]")
{
    BaseDetectInputs in = MaldenInputs();
    AddLePortQuay(in.objects);
    BaseDetectOptions opt;
    BaseDetectResult r;
    DetectBases(in, opt, r);

    INFO(Notes(r));
    const BaseCandidate* sp = Find(r, "SEAPORT");
    REQUIRE(sp);
    CHECK(sp->status == BaseStatus::Seed);
    CHECK(Str(sp->name) == "Le Port Harbour");
    CHECK(sp->garrison == 8);
    // walked inland off the quay: centre >= 2 m, every spawn-square corner dry
    CHECK(gMalden.Height(sp->x, sp->z) >= 2.0f);
    CHECK(gMalden.Height(sp->x + 25, sp->z + 25) >= 0.5f);
    CHECK(gMalden.Height(sp->x + 25, sp->z - 25) >= 0.5f);
    CHECK(sp->x < 8475.0f);
    // the authored Le Port Harbour [8410, 3690] is within 300 m
    CHECK(Dist(sp->x, sp->z, 8410.0f, 3690.0f) < 300.0f);
    CHECK(sp->confidence >= 0.5f);
    bool cites = false;
    for (const std::string& e : sp->evidence)
    {
        cites = cites || e.find("data3d\\molo_beton.p3d at [8476, 3754]") != std::string::npos;
    }
    CHECK(cites);

    // authored: covered, and the name may repeat the authored one
    in.authored.push_back(DetectAuthoredZone{"Le Port Harbour", "SEAPORT", 8410.0f, 3690.0f});
    DetectBases(in, opt, r);
    sp = Find(r, "SEAPORT");
    REQUIRE(sp);
    CHECK(sp->status == BaseStatus::Covered);
    CHECK(Str(sp->coveredBy) == "Le Port Harbour");
}

TEST_CASE("what is not a seaport: one pier, jetties only, a dock inland", "[game][guerrilla][scaffold][bases]")
{
    BaseDetectInputs in = MaldenInputs();
    BaseDetectOptions opt;
    BaseDetectResult r;

    in.objects = {Obj("data3d\\molo_beton.p3d", 8476.3f, 3754.1f)};
    DetectBases(in, opt, r);
    CHECK(Find(r, "SEAPORT") == nullptr);

    // two wooden jetties on the quay: a fishing landing, reported, not seeded
    in.objects = {Obj("data3d\\molo_drevo.p3d", 8476.0f, 3900.0f), Obj("data3d\\molo_drevo_end.p3d", 8480.0f, 3920.0f),
                  Obj("data3d\\molo_drevo_bs.p3d", 8478.0f, 3960.0f)};
    DetectBases(in, opt, r);
    const BaseCandidate* sp = Find(r, "SEAPORT");
    REQUIRE(sp);
    CHECK(sp->status == BaseStatus::LowConfidence);

    // harbour names 1 km from any water
    in.objects = {Obj("x\\dock_a.p3d", 5000.0f, 5000.0f), Obj("x\\dock_b.p3d", 5050.0f, 5000.0f)};
    DetectBases(in, opt, r);
    CHECK(Find(r, "SEAPORT") == nullptr);

    // waterside buildings alone never make a harbour
    in.objects = {Obj("data3d\\dum01.p3d", 8470.0f, 3650.0f, 9.0f), Obj("data3d\\dum02.p3d", 8470.0f, 3690.0f, 9.0f),
                  Obj("data3d\\dum01.p3d", 8470.0f, 3720.0f, 9.0f)};
    DetectBases(in, opt, r);
    CHECK(Find(r, "SEAPORT") == nullptr);

    // ... but they back a quay pair as geometry evidence
    AddLePortQuay(in.objects);
    DetectBases(in, opt, r);
    sp = Find(r, "SEAPORT");
    REQUIRE(sp);
    bool waterside = false;
    for (const std::string& e : sp->evidence)
    {
        waterside = waterside || e.find("waterside building") != std::string::npos;
    }
    CHECK(waterside);
}

TEST_CASE("military clusters become offline OUTPOST candidates", "[game][guerrilla][scaffold][bases]")
{
    BaseDetectInputs in = MaldenInputs();
    AddCompound(in.objects);
    // two lone fortress pieces 470 m apart: no post
    in.objects.push_back(Obj("data3d\\fortress1.p3d", 10477.0f, 4017.6f));
    in.objects.push_back(Obj("data3d\\fortress2.p3d", 10525.8f, 4025.2f));
    BaseDetectOptions opt;
    BaseDetectResult r;
    DetectBases(in, opt, r);

    INFO(Notes(r));
    const BaseCandidate* op = Find(r, "OUTPOST");
    REQUIRE(op);
    CHECK(Find(r, "OUTPOST", 1) == nullptr);
    CHECK(op->status == BaseStatus::Offline);
    CHECK(op->garrison == 6);
    // the Southeast Compound's yard [9690, 3990]
    CHECK(Dist(op->x, op->z, 9690.0f, 3990.0f) < 300.0f);
    CHECK(Str(op->name) == "Le Port Post");

    in.authored.push_back(DetectAuthoredZone{"Southeast Compound", "OUTPOST", 9690.0f, 3990.0f});
    DetectBases(in, opt, r);
    CHECK(Find(r, "OUTPOST")->status == BaseStatus::Covered);

    // barracks on an airfield belong to the airfield
    in = MaldenInputs();
    in.ils = AbelIls();
    for (int i = 0; i < 5; i++)
    {
        in.objects.push_back(Obj("data3d\\budova1.p3d", 8088.0f, 9920.0f + 11.0f * (float)i));
    }
    DetectBases(in, opt, r);
    CHECK(Find(r, "AIRFIELD"));
    CHECK(Find(r, "OUTPOST") == nullptr);
}

TEST_CASE("base names are stable and unique, playRegion bounds the candidates", "[game][guerrilla][scaffold][bases]")
{
    BaseDetectInputs in = MaldenInputs();
    in.ils = AbelIls();
    IlsEntry second;
    second.x = 4000;
    second.z = 6000;
    second.dirZ = -1;
    second.source = "CfgWorlds >> Abel >> SecondaryAirports >> Strip2";
    in.ils.push_back(second);
    AddLePortQuay(in.objects);
    BaseDetectOptions opt;
    BaseDetectResult r;
    DetectBases(in, opt, r);
    REQUIRE(Find(r, "AIRFIELD", 1));
    CHECK(Str(Find(r, "AIRFIELD", 0)->name) == "Malden Airfield");
    CHECK(Str(Find(r, "AIRFIELD", 1)->name) == "Malden Airfield 2");

    // the same input yields the same result
    BaseDetectResult again;
    DetectBases(in, opt, again);
    REQUIRE(again.candidates.size() == r.candidates.size());
    for (size_t i = 0; i < r.candidates.size(); i++)
    {
        CHECK(Str(again.candidates[i].name) == Str(r.candidates[i].name));
        CHECK(again.candidates[i].x == r.candidates[i].x);
        CHECK(again.candidates[i].z == r.candidates[i].z);
    }

    // an authored zone (or a town) already named "Le Port Harbour" elsewhere
    in.authored.push_back(DetectAuthoredZone{"Le Port Harbour", "CITY", 1000.0f, 1000.0f});
    DetectBases(in, opt, r);
    CHECK(Str(Find(r, "SEAPORT")->name) == "Le Port Harbour 2");

    // an unresolved stringtable key is no name
    CHECK(Str(BaseWorldName("Sinai", "$STR_SINAI")) == "Sinai");
    CHECK(Str(BaseWorldName("Sinai", "")) == "Sinai");
    CHECK(Str(BaseWorldName("Eden", "Everon")) == "Everon");

    // playRegion: only the south of the island
    in.authored.clear();
    in.hasRegion = true;
    in.regionMinX = 0;
    in.regionMinZ = 0;
    in.regionMaxX = 12800;
    in.regionMaxZ = 5000;
    DetectBases(in, opt, r);
    CHECK(Find(r, "AIRFIELD", 0)->status == BaseStatus::OutsideRegion);
    CHECK(Find(r, "AIRFIELD", 1)->status == BaseStatus::OutsideRegion);
    CHECK(Find(r, "SEAPORT")->status == BaseStatus::Seed);
    CHECK(Str(BaseStatusName(BaseStatus::OutsideRegion)) == "outside-region");
}

TEST_CASE("DetectBases without a height sampler detects nothing", "[game][guerrilla][scaffold][bases]")
{
    BaseDetectInputs in;
    in.ils = AbelIls();
    BaseDetectResult r;
    DetectBases(in, BaseDetectOptions(), r);
    CHECK(r.candidates.empty());
    CHECK(r.notes.size() == 1);
}

TEST_CASE("hangar clusters with no ILS entry become airfields, named after an airfield label",
          "[game][guerrilla][scaffold][bases]")
{
    // @LoBo's Egyptian Airbase in miniature: baracken hangars, an RKSL
    // shelter and a control tower, with the type-less Names label nearby
    BaseDetectInputs in = MaldenInputs();
    in.towns.push_back(DetectPlace{"Egyptian Airbase", 3443.0f, 1479.0f});
    in.objects.push_back(Obj("baracken\hangar01.p3d", 3573.0f, 1583.0f));
    in.objects.push_back(Obj("baracken\hangar01.p3d", 3500.0f, 1600.0f));
    in.objects.push_back(Obj("baracken\hangar03.p3d", 3400.0f, 1650.0f));
    in.objects.push_back(Obj("rksl-hangers\rksl_tmphanger_des.p3d", 3250.0f, 1600.0f));
    in.objects.push_back(Obj("baracken\brown_start_turm.p3d", 3300.0f, 1500.0f));
    BaseDetectOptions opt;
    BaseDetectResult r;
    DetectBases(in, opt, r);

    INFO(Notes(r));
    const BaseCandidate* af = Find(r, "AIRFIELD");
    REQUIRE(af);
    CHECK(Find(r, "AIRFIELD", 1) == nullptr);
    CHECK(af->status == BaseStatus::Seed);
    CHECK(Str(af->name) == "Egyptian Airbase");
    CHECK(af->garrison == 12);
    CHECK(af->confidence >= 0.5f);
    // the hangars are the airfield's, not an outpost of their own
    CHECK(Find(r, "OUTPOST") == nullptr);

    // two hangars alone are a shed row
    in.objects.resize(2);
    DetectBases(in, opt, r);
    CHECK(Find(r, "AIRFIELD") == nullptr);

    // a hangar cluster beside an ILS runway is that runway's airfield
    in = MaldenInputs();
    in.ils = AbelIls();
    in.objects.push_back(Obj("data3d\hangar.p3d", 8150.0f, 9900.0f));
    in.objects.push_back(Obj("data3d\hangar.p3d", 8150.0f, 9950.0f));
    in.objects.push_back(Obj("data3d\hangar.p3d", 8150.0f, 10000.0f));
    DetectBases(in, opt, r);
    REQUIRE(Find(r, "AIRFIELD"));
    CHECK(Find(r, "AIRFIELD", 1) == nullptr);
    CHECK(Str(Find(r, "AIRFIELD")->name) == "Malden Airfield");

    // no label: the nearest town names an object airfield
    in = MaldenInputs();
    for (int i = 0; i < 3; i++)
    {
        in.objects.push_back(Obj("data3d\hangar.p3d", 9000.0f + 40.0f * (float)i, 8500.0f));
    }
    DetectBases(in, opt, r);
    REQUIRE(Find(r, "AIRFIELD"));
    CHECK(Str(Find(r, "AIRFIELD")->name) == "Larche Airfield");
}

TEST_CASE("an airbase spread over two hangar clusters is one airfield", "[game][guerrilla][scaffold][bases]")
{
    // @LoBo's Egyptian Airbase: one hangar group near [2500, 1540], another
    // near [3570, 1650], the type-less label at [3443, 1479]
    BaseDetectInputs in = MaldenInputs();
    in.towns.push_back(DetectPlace{"Egyptian Airbase", 3443.0f, 1479.0f});
    for (int i = 0; i < 4; i++)
    {
        in.objects.push_back(Obj("rksl-hangers\rksl_medhang_des.p3d", 2450.0f + 40.0f * (float)i, 1540.0f));
        in.objects.push_back(Obj("baracken\hangar03.p3d", 2450.0f + 40.0f * (float)i, 1600.0f));
    }
    for (int i = 0; i < 3; i++)
    {
        in.objects.push_back(Obj("baracken\hangar01.p3d", 3530.0f + 40.0f * (float)i, 1650.0f));
    }
    BaseDetectResult r;
    DetectBases(in, BaseDetectOptions(), r);
    INFO(Notes(r));
    REQUIRE(Find(r, "AIRFIELD"));
    CHECK(Find(r, "AIRFIELD", 1) == nullptr);
    CHECK(Str(Find(r, "AIRFIELD")->name) == "Egyptian Airbase");
    // centred between the two hangar groups, its footprint covering both
    CHECK(Dist(Find(r, "AIRFIELD")->x, Find(r, "AIRFIELD")->z, 3040.0f, 1610.0f) < 100.0f);
    CHECK(Dist(Find(r, "AIRFIELD")->x, Find(r, "AIRFIELD")->z, 3443.0f, 1479.0f) < 700.0f);
    CHECK(Find(r, "AIRFIELD")->radius > 600.0f);
}

TEST_CASE("only a settlement names a base, and a rowan is no crane", "[game][guerrilla][scaffold][bases]")
{
    // an islet label nearer the quay than Le Port: no houses, so it names nothing
    BaseDetectInputs in = MaldenInputs();
    in.towns.push_back(DetectPlace{"Tiran Is", 8600.0f, 3800.0f});
    AddLePortQuay(in.objects);
    BaseDetectResult r;
    DetectBases(in, BaseDetectOptions(), r);
    REQUIRE(Find(r, "SEAPORT"));
    CHECK(Str(Find(r, "SEAPORT")->name) == "Le Port Harbour");

    // nothing within 4 km: the world's name
    in = MaldenInputs();
    in.towns.clear();
    AddLePortQuay(in.objects);
    DetectBases(in, BaseDetectOptions(), r);
    REQUIRE(Find(r, "SEAPORT"));
    CHECK(Str(Find(r, "SEAPORT")->name) == "Malden Harbour");

    CHECK(ClassifyBaseModel("data3d\str jerabina.p3d").harbour == 0.0f);
    CHECK(ClassifyBaseModel("x\jerab_port.p3d").harbour == 1.0f);
}
