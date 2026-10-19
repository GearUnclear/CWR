#include <Poseidon/Graphics/Remaster/SurfaceMaterials.hpp>

#include <algorithm>
#include <cctype>
#include <iterator>

namespace Poseidon::remaster
{

namespace
{

using K = render::DrawObjectKind;

struct ClassRow
{
    SurfaceClass surface;
    const char* name;
    float porosity;
};

// In enum order (checked below). Porosity from the research prototype's
// per-class response (materials/override_v9, surfFor).
constexpr ClassRow kClasses[] = {
    {SurfaceClass::Unknown, "unknown", 0.5f}, {SurfaceClass::Paint, "paint", 0.05f},
    {SurfaceClass::Metal, "metal", 0.0f},     {SurfaceClass::Glass, "glass", 0.0f},
    {SurfaceClass::Rubber, "rubber", 0.1f},   {SurfaceClass::Fabric, "fabric", 0.85f},
    {SurfaceClass::Skin, "skin", 0.3f},       {SurfaceClass::Road, "road", 0.35f},
    {SurfaceClass::Masonry, "masonry", 0.6f}, {SurfaceClass::Wood, "wood", 0.7f},
    {SurfaceClass::Leather, "leather", 0.4f}, {SurfaceClass::Weapon, "weapon", 0.0f},
    {SurfaceClass::Lens, "lens", 0.0f},       {SurfaceClass::Roof, "roof", 0.4f},
};
static_assert(sizeof(kClasses) / sizeof(kClasses[0]) == kSurfaceClassCount, "one row per SurfaceClass");

constexpr bool ClassesInOrder()
{
    for (int i = 0; i < kSurfaceClassCount; ++i)
    {
        if (static_cast<int>(kClasses[i].surface) != i)
            return false;
    }
    return true;
}
static_assert(ClassesInOrder(), "kClasses rows follow the SurfaceClass enum");

bool EqualNoCase(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    }
    return true;
}

std::string Lower(std::string_view s)
{
    std::string out(s);
    for (char& c : out)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

bool Contains(const std::string& s, const std::string& token)
{
    return s.find(token) != std::string::npos;
}

bool Matches(const SurfaceRule& r, const std::string& lowerPath, K kind)
{
    if ((r.kinds & KindBit(kind)) == 0)
        return false;
    for (const std::string& t : r.none)
    {
        if (Contains(lowerPath, t))
            return false;
    }
    if (r.any.empty())
        return true;
    for (const std::string& t : r.any)
    {
        if (Contains(lowerPath, t))
            return true;
    }
    return false;
}

SurfaceRule Rule(const char* name, SurfaceClass surface, std::uint32_t kinds, std::vector<std::string> any,
                 std::vector<std::string> none = {})
{
    SurfaceRule r;
    r.name = name;
    r.surface = surface;
    r.kinds = kinds;
    r.any = std::move(any);
    r.none = std::move(none);
    return r;
}

// The token lists of the research prototype (EngineGL33_Lighting.cpp
// ClassifySurface, round 6; every texture of three islands in
// classification_log_r6.txt).
const std::vector<std::string> kGlass = {"sklo",       "skla",   "glass",  "window",   "windscreen",
                                         "windshield", "armwin", "canopy", "cifernik", "budik"};
const std::vector<std::string> kLens = {"reflekt", "svetl", "blink", "lamp", "light"};
const std::vector<std::string> kRubber = {"pneu", "guma", "tyre", "tire", "wheel", "kolo", "pasy", "track"};
const std::vector<std::string> kBarrel = {"hlaven", "hlavn", "delo", "kanon", "barrel", "lauf"};
const std::vector<std::string> kMetal = {"hlaven",  "hlavn",  "delo",  "kulomet", "lauf",  "barrel", "kanon", "trub",
                                         "roura",   "_rez",   "rez_",  "_kov",    "kov_",  "kovn",   "chrom", "vyfuk",
                                         "exhaust", "antena", "plech", "zink",    "zelez", "metal",  "steel", "pipe"};
const std::vector<std::string> kFabric = {"plachta", "sedl",   "sedad",    "taska",  "igelit", "lano",
                                          "_sit",    "net",    "tarp",     "canvas", "seat",   "bench",
                                          "cloth",   "camnet", "maskovac", "stan_",  "stan.",  "tent"};
const std::vector<std::string> kSkin = {"xicht", "hlava", "head", "face", "hand", "ruce"};
const std::vector<std::string> kRoad = {"silnic", "asfalt", "cesta", "road", "dlazb", "chodnik", "runway", "draha"};
const std::vector<std::string> kWood = {"drev", "prkn", "plank", "wood", "bark", "kmen", "dreva", "bedna", "crate"};
const std::vector<std::string> kWeapon = {"ak47", "ak74", "m16",    "m21",  "m60", "rpg", "law",    "carl",  "g36",
                                          "fal",  "uzi",  "pistol", "m249", "pk",  "mag", "weapon", "zbran", "scope"};

constexpr std::uint32_t kMan = KindBit(K::Man);

} // namespace

const char* SurfaceClassName(SurfaceClass c)
{
    const int i = static_cast<int>(c);
    return i >= 0 && i < kSurfaceClassCount ? kClasses[i].name : "unknown";
}

bool ParseSurfaceClass(std::string_view name, SurfaceClass& out)
{
    for (const ClassRow& row : kClasses)
    {
        if (EqualNoCase(name, row.name))
        {
            out = row.surface;
            return true;
        }
    }
    return false;
}

float SurfacePorosity(SurfaceClass c)
{
    const int i = static_cast<int>(c);
    return kClasses[i >= 0 && i < kSurfaceClassCount ? i : 0].porosity;
}

render::DrawObjectKind DrawObjectKindFromNames(std::string_view simulation, std::string_view className)
{
    const std::string cls = Lower(className);
    if (Contains(cls, "camp") || Contains(cls, "tent") || Contains(cls, "camnet"))
        return K::Fabric;
    const std::string sim = Lower(simulation);
    if (sim == "car" || sim == "tank" || sim == "motorcycle")
        return K::LandVehicle;
    if (sim == "soldier" || sim == "invisible")
        return K::Man;
    if (sim == "helicopter" || sim == "airplane" || sim == "parachute")
        return K::Aircraft;
    if (sim == "ship")
        return K::Ship;
    if (sim.rfind("proxy", 0) == 0 || sim == "weapon")
        return K::Weapon;
    return K::Static;
}

std::uint32_t ParseKindMask(std::string_view name)
{
    struct Row
    {
        const char* name;
        std::uint32_t mask;
    };
    static const Row rows[] = {
        {"vehicle", kVehicleKinds},     {"landvehicle", KindBit(K::LandVehicle)}, {"man", KindBit(K::Man)},
        {"static", KindBit(K::Static)}, {"aircraft", KindBit(K::Aircraft)},       {"ship", KindBit(K::Ship)},
        {"weapon", KindBit(K::Weapon)}, {"fabric", KindBit(K::Fabric)},           {"unknown", KindBit(K::Unknown)},
    };
    for (const Row& r : rows)
    {
        if (EqualNoCase(name, r.name))
            return r.mask;
    }
    return 0;
}

SurfaceTable DefaultSurfaceTable()
{
    SurfaceTable t;
    t.rules = {
        // Glass by name on anything (the prototype's first test).
        Rule("glass", SurfaceClass::Glass, kAllKinds, kGlass),
        Rule("window", SurfaceClass::Glass, kAllKinds, {"okno"}, {"ram", "_in", "bez"}),
        // Tents and camouflage nets: their textures are anonymous atlases.
        Rule("tentObject", SurfaceClass::Fabric, KindBit(K::Fabric), {}),
        // Vehicles: alpha-blended vehicle textures are mostly cutout details
        // (wheel rims, tools), so glass comes from the name only.
        Rule("vehicleGlass", SurfaceClass::Glass, kVehicleKinds, {"skl"}),
        Rule("vehicleLens", SurfaceClass::Lens, kVehicleKinds, kLens),
        Rule("vehicleRubber", SurfaceClass::Rubber, kVehicleKinds, kRubber),
        // Gun barrels and cannons on military vehicles are painted steel.
        Rule("vehicleBarrel", SurfaceClass::Paint, kVehicleKinds, kBarrel),
        Rule("vehicleMetal", SurfaceClass::Metal, kVehicleKinds, kMetal, {"plech"}),
        Rule("vehicleFabric", SurfaceClass::Fabric, kVehicleKinds, kFabric),
        // Men: face and hands, boots, carried weapons; the rest is cloth.
        Rule("manSkin", SurfaceClass::Skin, kMan, kSkin),
        Rule("manHand", SurfaceClass::Skin, kMan, {"ruka"}, {"rukav"}),
        Rule("manBoot", SurfaceClass::Leather, kMan, {"bota", "boot"}),
        Rule("manWeapon", SurfaceClass::Weapon, kMan, kWeapon),
        // Static objects and draws without an object (roads, decals).
        Rule("road", SurfaceClass::Road, kStaticKinds, kRoad),
        Rule("roof", SurfaceClass::Roof, kStaticKinds, {"strech", "roof"}, {"plech"}),
        Rule("staticMetal", SurfaceClass::Metal, kStaticKinds, kMetal),
        Rule("staticWood", SurfaceClass::Wood, kStaticKinds, kWood),
        Rule("staticFabric", SurfaceClass::Fabric, kStaticKinds, kFabric),
        Rule("staticLens", SurfaceClass::Lens, kStaticKinds, kLens),
    };
    return t;
}

void MergeSurfaceRules(SurfaceTable& table, const std::vector<SurfaceRule>& rules)
{
    std::vector<SurfaceRule> front;
    for (const SurfaceRule& r : rules)
    {
        auto same = std::find_if(table.rules.begin(), table.rules.end(),
                                 [&r](const SurfaceRule& x) { return EqualNoCase(x.name, r.name); });
        if (same != table.rules.end())
            *same = r;
        else
            front.push_back(r);
    }
    table.rules.insert(table.rules.begin(), front.begin(), front.end());
}

bool MakeSurfaceRule(const std::string& name, std::string_view surface, const std::vector<std::string>& any,
                     const std::vector<std::string>& none, const std::vector<std::string>& kindNames, SurfaceRule& out,
                     std::string* why)
{
    SurfaceRule r;
    r.name = name;
    if (!ParseSurfaceClass(surface, r.surface))
    {
        if (why)
            *why = "unknown surface class '" + std::string(surface) + "'";
        return false;
    }
    for (const std::string& t : any)
    {
        if (!t.empty())
            r.any.push_back(Lower(t));
    }
    for (const std::string& t : none)
    {
        if (!t.empty())
            r.none.push_back(Lower(t));
    }
    if (!kindNames.empty())
    {
        r.kinds = 0;
        for (const std::string& k : kindNames)
        {
            const std::uint32_t mask = ParseKindMask(k);
            if (mask == 0)
            {
                if (why)
                    *why = "unknown object kind '" + k + "'";
                return false;
            }
            r.kinds |= mask;
        }
    }
    out = std::move(r);
    return true;
}

SurfaceClass KindDefaultClass(render::DrawObjectKind kind, bool onSurface)
{
    switch (kind)
    {
        case K::LandVehicle:
        case K::Aircraft:
        case K::Ship:
            return SurfaceClass::Paint;
        case K::Man:
        case K::Fabric:
            return SurfaceClass::Fabric;
        case K::Weapon:
            return SurfaceClass::Weapon;
        case K::Static:
            return onSurface ? SurfaceClass::Road : SurfaceClass::Masonry;
        case K::Unknown:
        case K::Count:
            break;
    }
    return onSurface ? SurfaceClass::Road : SurfaceClass::Unknown;
}

SurfaceClass ClassifySurface(const SurfaceTable& table, std::string_view texturePath, render::DrawObjectKind kind,
                             bool onSurface)
{
    const std::string path = Lower(texturePath);
    for (const SurfaceRule& r : table.rules)
    {
        if (Matches(r, path, kind))
            return r.surface;
    }
    return KindDefaultClass(kind, onSurface);
}

} // namespace Poseidon::remaster
