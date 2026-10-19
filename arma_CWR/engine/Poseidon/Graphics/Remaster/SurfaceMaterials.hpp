#pragma once

#include <Poseidon/Graphics/Rendering/RenderPassDescriptor.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// Surface classes for the remastered wet weather (feature switch
// `wetWeather`, research/INTEGRATION-PLAN.md section 5 B7, research track
// materials). Pure, no GL, no engine state.
//
// CWA's models carry no material data beyond a texture, a colour set and
// sometimes a specular power. What a section is made of (paint that sheds the
// rain as a film, cloth that soaks it up and darkens, plaster in between)
// comes from its texture's name and the kind of object drawing it: the stock
// data names textures in Czech (sklo glass, okno window, kolo wheel, plachta
// tarp, xicht face, silnice road), mods mostly in English.
//
//   - A table of rules (SurfaceTable): each names a class, the tokens one of
//     which must be in the lower-case texture path, the tokens none of which may
//     be, and the object kinds it applies to. The first rule that matches wins;
//     no match falls back to the kind's own class (KindDefaultClass: paint on a
//     vehicle, cloth on a man, plaster on a house, road under a road decal).
//   - The stock table is DefaultSurfaceTable(); a mod extends or overrides it
//     with CfgSurfaceMaterials in its config (MergeSurfaceRules: a class whose
//     name is a stock rule's replaces that rule, any other goes before the
//     stock rules, so a mod's tokens win).
//   - The backend classifies each texture once per kind (TextureGL33) and hands
//     the shader the class of each draw (LitConstants slot MatClass); the
//     shader's table (kWetGLSL, EngineGL33_LightingShadersSurface.cpp) holds the
//     porosity of each class, which SurfacePorosity mirrors.
//
// The terrain is not classified here: its squares are blends of four corner
// materials named by island-specific two-letter codes (abel\p4p4ttp4.pac), so
// the shader tells grass from bare ground per pixel from the albedo.

namespace Poseidon::remaster
{

enum class SurfaceClass : std::uint8_t
{
    Unknown,
    Paint,   // vehicle paint (matte military): sheds the rain as a film
    Metal,   // bare metal, sheet metal, exhausts
    Glass,   // windows, canopies, gauges
    Rubber,  // tyres, wheels, tracks
    Fabric,  // uniforms, tarps, canvas, tents, nets: soaks up the rain
    Skin,    // faces and hands
    Road,    // asphalt, tracks, paving: holds puddles
    Masonry, // plaster, brick, concrete (a house's default)
    Wood,    // planks, crates, bark
    Leather, // boots
    Weapon,  // rifles, launchers
    Lens,    // lamp and headlight glass
    Roof,    // tiled roofs
    Count
};
constexpr int kSurfaceClassCount = static_cast<int>(SurfaceClass::Count);

// "paint", "glass", ... (the names CfgSurfaceMaterials uses); "unknown" for
// anything out of range.
const char* SurfaceClassName(SurfaceClass c);
bool ParseSurfaceClass(std::string_view name, SurfaceClass& out);

// How much water a class soaks up (0 sealed: the rain forms a film; 1 cloth:
// the rain darkens it and forms none). The shader's table, kept equal by a test.
float SurfacePorosity(SurfaceClass c);

// The kind of object from its config: simulation "car", "tank", "motorcycle"
// are land vehicles, "soldier" and "invisible" men, "helicopter", "airplane"
// and "parachute" aircraft, "ship" ships, "proxy..." and "weapon" weapons;
// tents and camouflage nets by their class name (camp, tent, camnet), whatever
// their simulation; anything else is Static.
render::DrawObjectKind DrawObjectKindFromNames(std::string_view simulation, std::string_view className);
// "vehicle" (land vehicles, aircraft and ships), "landvehicle", "man",
// "static", "aircraft", "ship", "weapon", "fabric", "unknown" -> a mask of
// KindBit()s; 0 for an unknown name.
std::uint32_t ParseKindMask(std::string_view name);

constexpr std::uint32_t KindBit(render::DrawObjectKind k)
{
    return 1u << static_cast<unsigned>(k);
}
constexpr std::uint32_t kAllKinds = (1u << render::kDrawObjectKindCount) - 1u;
constexpr std::uint32_t kVehicleKinds = KindBit(render::DrawObjectKind::LandVehicle) |
                                        KindBit(render::DrawObjectKind::Aircraft) |
                                        KindBit(render::DrawObjectKind::Ship);
// Static objects and draws without an object (roads and other decals the
// landscape draws, things without a type).
constexpr std::uint32_t kStaticKinds =
    KindBit(render::DrawObjectKind::Static) | KindBit(render::DrawObjectKind::Unknown);

struct SurfaceRule
{
    std::string name; // the rule's id (a CfgSurfaceMaterials class of this name replaces it)
    SurfaceClass surface = SurfaceClass::Unknown;
    std::vector<std::string> any;  // lower case; one must be in the texture path (none listed: any path)
    std::vector<std::string> none; // lower case; none may be in it
    std::uint32_t kinds = kAllKinds;
};

struct SurfaceTable
{
    std::vector<SurfaceRule> rules; // in order: the first rule that matches wins
};

// The stock rules (Czech and English tokens of the stock data, @LoBo's
// English), in the order the research prototype tested them.
SurfaceTable DefaultSurfaceTable();
// A mod's rules (CfgSurfaceMaterials, in declaration order): one named like a
// rule already in the table replaces it in place, the others go before the
// table's rules.
void MergeSurfaceRules(SurfaceTable& table, const std::vector<SurfaceRule>& rules);
// One CfgSurfaceMaterials class as a rule. `kindNames` empty = every kind.
// False (and why) for an unknown class or kind name.
bool MakeSurfaceRule(const std::string& name, std::string_view surface, const std::vector<std::string>& any,
                     const std::vector<std::string>& none, const std::vector<std::string>& kindNames, SurfaceRule& out,
                     std::string* why = nullptr);

// When no rule matches: paint on a vehicle, cloth on a man or a tent, a
// weapon's own class, road under an on-surface draw (roads, tracks), plaster on
// a static object, Unknown for a draw without an object.
SurfaceClass KindDefaultClass(render::DrawObjectKind kind, bool onSurface);
// The class of a section: its texture path (any case, any slashes), the kind
// of object drawing it and whether the draw lies on the ground (a road decal).
SurfaceClass ClassifySurface(const SurfaceTable& table, std::string_view texturePath, render::DrawObjectKind kind,
                             bool onSurface);

} // namespace Poseidon::remaster
