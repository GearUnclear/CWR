#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaModule.hpp>
#include <Poseidon/UI/GameModule.hpp>
#include <Poseidon/UI/OptionsUICommon.hpp>          // CreateSingleMissionBank (per-island description.ext peek)
#include <Poseidon/Game/Guerrilla/FactionTwins.hpp> // shared with ZoneRegistry::ResolveSideCollisions
#include <Poseidon/Game/Guerrilla/OutfitSelect.hpp> // FindGuerrillaFactionEntry (outfit cycler, issue #25)
#include <Poseidon/Core/resincl.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/IO/Filesystem/FileOps.hpp> // FilePathExists (template .pbo check)
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // GetShapeName (preview model probe)
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>       // Shapes bank + ProxyObject (preview mannequin)
#include <Poseidon/World/Scene/Scene.hpp>                    // GScene (preview proxy draw: LOD pick)
#include <Poseidon/World/Scene/Camera/Camera.hpp>            // camera direction for the proxy LOD pick
#include <Poseidon/World/Simulation/FrameInv.hpp>            // FrameWithInverse (preview proxy draw)
#include <Poseidon/World/Simulation/Animation/Animation.hpp> // AnimationSection (zasleh hide)
#include <Poseidon/Foundation/Math/MathDefs.hpp>             // H_PI (preview turntable)
#include <Poseidon/Foundation/platform.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

#include <ctype.h> // tolower (body-roster dedupe keys)
#include <math.h>  // fmodf/fabsf (preview turntable + fit)
#include <stdio.h>
#include <string> // body-roster dedupe keys

// 3D-UI camera-space depth scale, defined in UIControlsBase.cpp next to
// ControlObject (whose constructor multiplies the config z by it).
extern const float CameraZoom;

namespace Poseidon
{
// The shape-path resolver ControlObject's constructor runs on its `model`
// config value (OptionsUI.cpp) - declared the way UIControlsBase.cpp does.
RString FindShape(RString name);

namespace
{
// Injected occupier/resistance cycler buttons. Safely clear of the idcs the
// reused RscDisplaySelectIsland resource occupies (IDC_OK/IDC_CANCEL = 1/2,
// IDC_SELECT_ISLAND* = 101..103) and of any future RscDisplayGuerrillaNewGame
// that keeps the same layout.
constexpr int kIdcOccupier = 150;
constexpr int kIdcResistance = 151;
// 152 is RESERVED for issue #16's gmSelStartTown cycler; the character
// outfit cycler (issue #25) takes 153.
constexpr int kIdcOutfit = 153;
// 154 is the outfit-preview mannequin (issue #25 M4): a CT_OBJECT rendering
// the resolved playerClassWarrior/playerClassCiv body for the current
// island/resistance/outfit, the CHead rotating-preview technique from the
// player identity screen. Injected only once a model actually resolves;
// hidden (never a substitute body) whenever one does not.
constexpr int kIdcOutfitPreview = 154;
// 155 is the BODY browser (player-body pick): a cycler over every creatable
// Man-derived class of the loaded package, grouped by side (see
// GuerrillaListPlayerBodies). Next free idc: 156.
constexpr int kIdcBody = 155;

// Normalized-screen slots for the injected 2D cyclers — bottom-left column,
// clear of the island-list notebook that fills the screen centre. The
// body browser and outfit cycler stack ABOVE the faction pair so the
// shipped 0.80/0.87 slots (and the e2e assertions on them) stay put.
constexpr float kCyclerX = 0.02f;
constexpr float kCyclerW = 0.32f;
constexpr float kCyclerH = 0.05f;
constexpr float kCyclerBodyY = 0.66f;
constexpr float kCyclerOutfitY = 0.73f;
constexpr float kCyclerOccupierY = 0.80f;
constexpr float kCyclerResistanceY = 0.87f;

// The preview mannequin's slot: the free stretch of the bottom-left column
// above the body browser. Authored as a normalized-screen box like the 2D
// cyclers; the 3D placement is derived from it at runtime (Convert2DTo3D)
// so it tracks the engine's aspect settings.
constexpr float kPreviewCX = 0.18f;      // column centre (the cyclers' midline)
constexpr float kPreviewTopY = 0.26f;    // below the screen-top header strip
constexpr float kPreviewBottomY = 0.61f; // above the body browser at 0.66
// Camera-space depth in the units ControlObject's constructor reads (it
// multiplies by CameraZoom): inside the 3D-UI ray range (ControlObject::
// IsInside probes out to 2.0 * CameraZoom).
constexpr float kPreviewDepthCfg = 1.0f;
// Seconds per full turntable revolution. CHead spins a head in 4; a full
// body reads better a touch slower.
constexpr float kPreviewTurnPeriod = 8.0f;

// The outfit-cycler tokens. WARRIOR must be index 0 (the default): it is
// defined as the authored mission.sqm class, so an untouched cycler
// publishes a value the substitution seam ignores — indistinguishable from
// publishing nothing (the same invariant GuerrillaDefaultSelections keeps
// for the faction pair).
constexpr const char* kOutfitWarrior = "WARRIOR";
constexpr const char* kOutfitCivilian = "CIVILIAN";

// The one player-facing wording for an unlaunchable pair, shared by the guard
// helper and the display so the message box and the tests cannot disagree.
RString SameSideMessage(RString side)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "OCCUPIER and RESISTANCE both resolve to side %s.\nPick factions on two different sides.",
             (const char*)side);
    return RString(buffer);
}

// The turntable mannequin (issue #25 M4): a ControlObject whose shape can be
// re-pointed at a different body model as the cyclers change (the CHead
// shape-swap precedent, DisplayUIMenus.cpp), refitted to a fixed 2D slot
// from its bounding box, and yaw-rotated from the display's OnSimulate.
// Renders whatever pose the p3d authors (base pose) - no UI control animates
// a Man skeleton, and that is accepted for this milestone.
class GuerrillaOutfitPreview : public ControlObject
{
  public:
    GuerrillaOutfitPreview(ControlsContainer* parent, int idc, const ParamEntry& cls) : ControlObject(parent, idc, cls)
    {
    }

    // Swap to a different body model (raw CfgVehicles `model` value, the
    // same string the injected config carried). No-op when unchanged; the
    // caller re-fits via PlaceInSlot either way.
    void SetModel(RString modelName)
    {
        if (stricmp(modelName, _modelName) == 0)
        {
            return;
        }
        _modelName = modelName;
        Ref<LODShapeWithShadow> shape = Shapes.New(FindShape(modelName), false, false);
        if (shape && shape->NLevels() > 0)
        {
            // the same UI-space treatment ControlObject's constructor applies
            shape->LevelOpaque(0)->MakeCockpit();
            shape->OrSpecial(BestMipmap | NoDropdown | DisableSun);
        }
        HideMuzzleFlashSections(shape);
        SetShape(shape);
    }

    // The weapon-proxy models author their muzzle-flash ("zasleh") sections
    // VISIBLE in the p3d (ak_47_v58_proxy.p3d carries a permanent white
    // zasleh3 star). In-mission Man::DrawProxies substitutes the real weapon
    // model and re-hides its zasleh every frame keyed on firing
    // (SoldierOldSimProxy.cpp); the raw ControlObject render path has no such
    // pass, so hide the section once per body swap on every proxy shape (and
    // on the body itself, should one ever author it). AnimationSection::Hide
    // is persistent per-shape state, exactly the not-firing state the mission
    // pass maintains, and nothing ever Unhides these proxy placeholders.
    static void HideMuzzleFlashSections(LODShapeWithShadow* body)
    {
        auto hideOnShape = [](LODShapeWithShadow* shape)
        {
            if (!shape)
            {
                return;
            }
            AnimationSection zasleh;
            zasleh.Init(shape, "zasleh", nullptr);
            for (int level = 0; level < shape->NLevels(); level++)
            {
                zasleh.Hide(shape, level);
            }
        };
        if (!body)
        {
            return;
        }
        hideOnShape(body);
        for (int level = 0; level < body->NLevels(); level++)
        {
            Shape* lShape = body->Level(level);
            if (!lShape)
            {
                continue;
            }
            for (int i = 0; i < lShape->NProxies(); i++)
            {
                const ProxyObject& proxy = lShape->Proxy(i);
                if (proxy.obj)
                {
                    hideOnShape(proxy.obj->GetShape());
                }
            }
        }
    }

    // Object::DrawProxies with two filters: the flag proxy never draws (its
    // model is hard-textured with a default US flag that only in-mission
    // flag-carrier machinery may rebind, so NO flag is the only rendering
    // that is safe for every faction), and of the authored weapon proxies
    // only the primary rifle survives - launchers and pistols hide, so the
    // mannequin stops wearing every weapon at once (see
    // GuerrillaPreviewHideWeaponProxy).
    void DrawProxies(int level, ClipFlags clipFlags, const Matrix4& transform, const Matrix4& invTransform, float dist2,
                     float z2, const LightList& lights) override
    {
        // Pars lives for the process; the entry lookup is one hash probe,
        // cheap enough per frame and always current should a mod reload
        // ever rebuild the merged config.
        const ParamEntry* nonAI = Pars.FindEntry("CfgNonAIVehicles");
        Shape* sShape = _shape->LevelOpaque(level);
        for (int i = 0; i < sShape->NProxies(); i++)
        {
            const ProxyObject& proxy = sShape->Proxy(i);
            if (!proxy.obj || GuerrillaPreviewIsFlagProxy(proxy.name) ||
                GuerrillaPreviewHideWeaponProxy(nonAI, proxy.name))
            {
                continue;
            }
            // from here the base Object::DrawProxies body, unchanged
            Matrix4Val pTransform = transform * proxy.obj->Transform();
            Matrix4Val invPTransform = proxy.invTransform * invTransform;
            LODShapeWithShadow* pshape = proxy.obj->GetShapeOnPos(pTransform.Position());
            if (!pshape)
            {
                continue;
            }
            int pLevel = GScene->LevelFromDistance2(pshape, dist2, pTransform.Scale(), pTransform.Direction(),
                                                    GScene->GetCamera()->Direction());
            if (pLevel == LOD_INVISIBLE)
            {
                continue;
            }
            FrameWithInverse pFrame(pTransform, invPTransform);
            proxy.obj->Draw(pLevel, ClipAll, pFrame);
        }
    }

    // Fit the current shape into the 2D slot: bounding-box centre on the
    // slot centre, model height scaled to the slot height. Per-shape, so a
    // LoBo body and a vanilla body render the same on-screen size no matter
    // where each p3d puts its origin (feet, waist, ...).
    void PlaceInSlot()
    {
        LODShapeWithShadow* shape = GetShape();
        if (!shape)
        {
            return;
        }
        const float depth = kPreviewDepthCfg * CameraZoom;
        Vector3 top = Convert2DTo3D(Point2DFloat(kPreviewCX, kPreviewTopY), depth);
        Vector3 bottom = Convert2DTo3D(Point2DFloat(kPreviewCX, kPreviewBottomY), depth);
        float slotHeight = fabsf(top[1] - bottom[1]);
        float modelHeight = shape->Max()[1] - shape->Min()[1];
        _fitScale = modelHeight > 0.01f ? slotHeight / modelHeight : 1.0f;
        Vector3 pos = (top + bottom) * 0.5f;
        pos[1] -= 0.5f * (shape->Min()[1] + shape->Max()[1]) * _fitScale;
        _position = pos;
        SetPosition(pos);
        SetScale(_fitScale);
    }

    // CHead::Simulate's turntable at full-body cadence. SetOrientation
    // resets the scale part of the transform, so reapply the fit, exactly
    // like CHead reapplies Scale().
    void SimulateTurntable()
    {
        float t = fmodf(Glob.uiTime.toFloat(), kPreviewTurnPeriod);
        Matrix3 orient(MRotationY, (H_PI * 2.0f / kPreviewTurnPeriod) * t);
        SetOrientation(orient);
        SetScale(_fitScale);
    }

    void OnDraw(float alpha) override
    {
        if (!GetShape())
        {
            return; // a body that failed to load draws nothing, never crashes
        }
        ControlObject::OnDraw(alpha);
    }

  private:
    RString _modelName;
    float _fitScale = 1.0f;
};
} // namespace

std::vector<RString> GuerrillaListIslands(const ParamEntry* worldList, const std::function<bool(RString)>& worldExists)
{
    std::vector<RString> islands;
    if (!worldList)
    {
        return islands;
    }
    for (int i = 0; i < worldList->GetEntryCount(); i++)
    {
        const ParamEntry& entry = worldList->GetEntry(i);
        if (!entry.IsClass())
        {
            continue;
        }
        RString name = entry.GetName();
        if (worldExists && !worldExists(name))
        {
            continue;
        }
        islands.push_back(name);
    }
    return islands;
}

std::vector<RString> GuerrillaListFactions(const ParamEntry* factionsCfg)
{
    std::vector<RString> out;
    if (!factionsCfg)
    {
        return out;
    }
    for (int i = 0; i < factionsCfg->GetEntryCount(); i++)
    {
        const ParamEntry& e = factionsCfg->GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        RString name = e.GetName();
        // the class name is the default side, so a `class CIV` needs no
        // `side` key to be excluded here
        RString side = e.ReadValue("side", name);
        if (stricmp(side, "CIV") == 0)
        {
            continue; // the population is ambience, never a combatant choice
        }
        out.push_back(name);
    }
    return out;
}

std::vector<GuerrillaBodyChoice> GuerrillaListPlayerBodies(const ParamEntry* vehiclesCfg,
                                                           const std::function<bool(RString)>& shapeFileExists)
{
    std::vector<GuerrillaBodyChoice> out;
    if (!vehiclesCfg)
    {
        return out;
    }
    const ParamEntry* manEntry = vehiclesCfg->FindEntry("Man");
    if (!manEntry || !manEntry->IsClass())
    {
        return out; // no Man base class: nothing is a body
    }
    const ParamClass* manCls = static_cast<const ParamClass*>(manEntry);
    // TargetSide values 0..3 (World/Scene/Object.hpp) to the script-side
    // strings the rest of the Guerrilla surface speaks
    static constexpr const char* kSideNames[] = {"EAST", "WEST", "GUER", "CIV"};
    constexpr int kNSides = 4;
    // per-side buckets, WEST/EAST/GUER/CIV output order (index into these)
    static constexpr int kSideOrder[] = {1, 0, 2, 3};
    std::vector<GuerrillaBodyChoice> buckets[kNSides];
    std::vector<std::string> seen[kNSides]; // lowercased displayName|model dedupe keys
    for (int i = 0; i < vehiclesCfg->GetEntryCount(); i++)
    {
        const ParamEntry& e = vehiclesCfg->GetEntry(i);
        if (!e.IsClass())
        {
            continue;
        }
        const ParamClass* cls = static_cast<const ParamClass*>(&e);
        if (cls == manCls || !cls->IsDerivedFrom(*manCls))
        {
            continue;
        }
        // scope > 0: scope=1 script-only classes (SoldierGFakeC...) are
        // deliberate picks, scope=0 abstract bases are not creatable
        if (e.ReadValue("scope", 0) <= 0)
        {
            continue;
        }
        int side = e.ReadValue("side", (int)kNSides);
        if (side < 0 || side >= kNSides)
        {
            continue; // TLogic and friends are not playable bodies
        }
        // plan-15 shaped: a class whose p3d the package does not ship is
        // skipped, never crashes and never reaches the publish channel
        RString model = e.ReadValue("model", RString());
        if (model.GetLength() == 0 || !shapeFileExists || !shapeFileExists(GetShapeName(model)))
        {
            continue;
        }
        // dedupe by what the player can SEE: one look (displayName+model)
        // per side, first class in config scan order wins
        RString displayName = e.ReadValue("displayName", RString());
        std::string key = std::string((const char*)displayName) + "|" + std::string((const char*)model);
        for (char& c : key)
        {
            c = (char)tolower((unsigned char)c);
        }
        bool dup = false;
        for (const std::string& k : seen[side])
        {
            if (k == key)
            {
                dup = true;
                break;
            }
        }
        if (dup || (int)buckets[side].size() >= kGuerrillaMaxBodiesPerSide)
        {
            continue;
        }
        seen[side].push_back(key);
        buckets[side].push_back(GuerrillaBodyChoice{RString(e.GetName()), RString(kSideNames[side])});
    }
    for (int side : kSideOrder)
    {
        for (const GuerrillaBodyChoice& b : buckets[side])
        {
            out.push_back(b);
        }
    }
    return out;
}

RString GuerrillaTemplateMissionBase(RString island)
{
    return RString("missions\\Guerrilla.") + island;
}

bool GuerrillaTemplateExists(RString island, const std::function<bool(RString)>& pboExists,
                             const std::function<bool(RString)>& missionFileExists)
{
    RString base = GuerrillaTemplateMissionBase(island);
    if (pboExists && pboExists(base + RString(".pbo")))
    {
        return true;
    }
    return missionFileExists && missionFileExists(base + RString("\\mission.sqm"));
}

RString GuerrillaFactionSide(const ParamEntry* factionsCfg, RString faction)
{
    if (!factionsCfg || faction.GetLength() == 0)
    {
        return RString();
    }
    const ParamEntry* cls = factionsCfg->FindEntry(faction);
    if (!cls || !cls->IsClass())
    {
        return RString();
    }
    return cls->ReadValue("side", faction);
}

int GuerrillaIndexOfSelection(const ParamEntry* factionsCfg, const std::vector<RString>& list, RString selection)
{
    if (selection.GetLength() == 0)
    {
        return -1;
    }
    // side first, then class name — the exact order ZoneRegistry::FindFaction
    // scans in, so an index found here names the record the registry would
    // have matched for the same string.
    for (int i = 0; i < (int)list.size(); i++)
    {
        RString side = GuerrillaFactionSide(factionsCfg, list[i]);
        if (side.GetLength() > 0 && stricmp(side, selection) == 0)
        {
            return i;
        }
    }
    for (int i = 0; i < (int)list.size(); i++)
    {
        if (stricmp(list[i], selection) == 0)
        {
            return i;
        }
    }
    return -1;
}

void GuerrillaDefaultSelections(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg,
                                const std::vector<RString>& list, int& outOccupier, int& outResistance)
{
    outOccupier = -1;
    outResistance = -1;
    if (list.empty())
    {
        return; // no real choice: SelectedOccupier/Resistance stay EMPTY
    }
    // Whatever the cyclers start on IS published on OK, so it has to be the
    // same pair a no-UI direct launch of this template resolves. Walk the same
    // precedence chain ZoneRegistry::LoadFromParams does, minus the gmSel*
    // rung (that rung is what these selections BECOME).
    RString defOccupier = zonesCfg ? zonesCfg->ReadValue("defaultOccupier", RString()) : RString();
    RString defResistance = zonesCfg ? zonesCfg->ReadValue("defaultResistance", RString()) : RString();
    outOccupier = GuerrillaIndexOfSelection(factionsCfg, list, defOccupier);
    outResistance = GuerrillaIndexOfSelection(factionsCfg, list, defResistance);
    // Then the registry's built-in EAST occupier / GUER resistance, for a
    // template that authors neither default* key.
    if (outOccupier < 0)
    {
        outOccupier = GuerrillaIndexOfSelection(factionsCfg, list, RString("EAST"));
    }
    if (outResistance < 0)
    {
        outResistance = GuerrillaIndexOfSelection(factionsCfg, list, RString("GUER"));
    }
    // Floor: two DISTINCT indices. Index 0 for both would open the screen on a
    // campaign fighting itself. (A template that deliberately points both
    // default* keys at one descriptor is honoured above and lands here only if
    // neither key resolved at all.)
    if (outOccupier < 0)
    {
        outOccupier = (outResistance == 0 && list.size() > 1) ? 1 : 0;
    }
    if (outResistance < 0)
    {
        outResistance = (outOccupier == 0 && list.size() > 1) ? 1 : 0;
    }
}

bool GuerrillaSelectionIsResolvable(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg, RString occupier,
                                    RString resistance, RString& outMessage)
{
    outMessage = RString();
    RString occSide = GuerrillaFactionSide(factionsCfg, occupier);
    RString resSide = GuerrillaFactionSide(factionsCfg, resistance);
    if (occSide.GetLength() == 0 || resSide.GetLength() == 0)
    {
        // Empty selections (no real faction config) cannot be validated here:
        // the mission's own default* keys decide what these become.
        return true;
    }

    RString playerSide = zonesCfg ? zonesCfg->ReadValue("playerSide", RString()) : RString();
    if (playerSide.GetLength() == 0)
    {
        // Legacy template: ZoneRegistry::ResolveSideCollisions returns early,
        // so two picks on one side would spawn the campaign fighting itself.
        if (stricmp(occSide, resSide) == 0)
        {
            outMessage = SameSideMessage(occSide);
            return false;
        }
        return true;
    }

    // Step 1, the resistance pin: twin substitution and rebase both land on
    // playerSide, so the registry's outcome is playerSide either way.
    resSide = playerSide;
    if (stricmp(occSide, resSide) != 0)
    {
        return true;
    }
    // Step 2, the occupier stepping off: its sideTwin, else the first free war
    // side. With three war sides and one pinned there is always one free, so
    // this block is unreachable by construction on any template that authors a
    // playerSide — it stays live because the floor has to be correct anyway.
    if (Guerrilla::TwinOffSide(factionsCfg, occupier, resSide).GetLength() > 0)
    {
        return true;
    }
    if (Guerrilla::FirstFreeWarSide(resSide).GetLength() > 0)
    {
        return true;
    }
    outMessage = SameSideMessage(resSide);
    return false;
}

std::vector<RString> GuerrillaOutfitChoices(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg,
                                            RString resistance)
{
    std::vector<RString> out;
    if (!factionsCfg)
    {
        return out;
    }
    // the same resistance-block precedence the substitution seam walks
    // (OutfitSelect): selection > defaultResistance > the built-in GUER side
    const ParamEntry* faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, resistance);
    if (!faction && zonesCfg)
    {
        RString defResistance = zonesCfg->ReadValue("defaultResistance", RString());
        faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, defResistance);
    }
    if (!faction)
    {
        faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, "GUER");
    }
    if (!faction)
    {
        return out;
    }
    // the pair is offered iff the descriptor authors a civilian player body;
    // playerClassWarrior needs no probe here — WARRIOR always means "the
    // authored mission.sqm class", key or no key
    RString civ = faction->ReadValue("playerClassCiv", RString());
    if (civ.GetLength() == 0)
    {
        return out;
    }
    out.push_back(RString(kOutfitWarrior));
    out.push_back(RString(kOutfitCivilian));
    return out;
}

RString GuerrillaOutfitPreviewClass(const ParamEntry* factionsCfg, const ParamEntry* zonesCfg, RString resistance,
                                    RString outfit)
{
    // the same resistance-block precedence GuerrillaOutfitChoices (and the
    // OutfitSelect substitution seam) walks: selection > defaultResistance >
    // the built-in GUER side
    const ParamEntry* faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, resistance);
    if (!faction && zonesCfg)
    {
        RString defResistance = zonesCfg->ReadValue("defaultResistance", RString());
        faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, defResistance);
    }
    if (!faction)
    {
        faction = Guerrilla::FindGuerrillaFactionEntry(factionsCfg, "GUER");
    }
    if (!faction)
    {
        return RString();
    }
    if (outfit.GetLength() > 0 && stricmp(outfit, kOutfitCivilian) == 0)
    {
        return faction->ReadValue("playerClassCiv", RString());
    }
    // WARRIOR and "no pair offered" both preview the warrior body: every
    // shipped template's playerClassWarrior documents the authored
    // mission.sqm class, and a descriptor without the key just hides the
    // preview (EMPTY here).
    if (outfit.GetLength() == 0 || stricmp(outfit, kOutfitWarrior) == 0)
    {
        return faction->ReadValue("playerClassWarrior", RString());
    }
    return RString(); // unknown token: show nothing rather than guess
}

RString GuerrillaOutfitPreviewModel(const ParamEntry* vehiclesCfg, RString className,
                                    const std::function<bool(RString)>& shapeFileExists)
{
    if (!vehiclesCfg || className.GetLength() == 0)
    {
        return RString();
    }
    const ParamEntry* cls = vehiclesCfg->FindEntry(className);
    if (!cls || !cls->IsClass())
    {
        return RString(); // class not in the loaded package: hide
    }
    // inherited `model` keys resolve too, ParamClass::FindEntry follows the
    // base chain
    RString model = cls->ReadValue("model", RString());
    if (model.GetLength() == 0)
    {
        return RString();
    }
    if (!shapeFileExists || !shapeFileExists(GetShapeName(model)))
    {
        return RString(); // shape file missing: hide, never substitute
    }
    return model;
}

namespace
{
// case-insensitive substring scan shared by the proxy predicates
bool ProxyNameHasStem(const char* proxyName, const char* stem)
{
    const int stemLen = (int)strlen(stem);
    for (const char* p = proxyName; *p; p++)
    {
        if (strnicmp(p, stem, stemLen) == 0)
        {
            return true;
        }
    }
    return false;
}
} // namespace

bool GuerrillaPreviewIsFlagProxy(const char* proxyName)
{
    if (!proxyName)
    {
        return false;
    }
    // Case-insensitive substring match on the two naming stems: the vanilla
    // bodies all reference "flag_vojak", community bodies use either the
    // English word or the Czech "vlajka" the rest of the flag machinery uses
    // (the flagpole cloth selection House.cpp animates is "vlajka").
    static constexpr const char* kStems[] = {"flag", "vlajka"};
    for (const char* stem : kStems)
    {
        if (ProxyNameHasStem(proxyName, stem))
        {
            return true;
        }
    }
    return false;
}

bool GuerrillaPreviewHideWeaponProxy(const ParamEntry* nonAIVehiclesCfg, const char* proxyName)
{
    if (!proxyName || !*proxyName)
    {
        return false;
    }
    // Belt-and-braces name stems for gear no vanilla config classes at all
    // (Classic+AddOns author no Proxynvg_proxy / Proxydalekohled_proxy, so
    // those proxies never even get created - see the header note) but a mod
    // package might: binoculars ("dalekohled" is the OFP-era model stem,
    // "binoc" the English one) and night-vision goggles.
    static constexpr const char* kHideStems[] = {"dalekohled", "binoc", "nvg"};
    for (const char* stem : kHideStems)
    {
        if (ProxyNameHasStem(proxyName, stem))
        {
            return true;
        }
    }
    if (!nonAIVehiclesCfg)
    {
        return false; // no config to classify against: draw (never hide the rifle)
    }
    // The engine's own proxy identity: CfgNonAIVehicles class "Proxy<name>",
    // spaces underscored - the exact lookup ShapeLOD.cpp builds the proxy
    // object from. The inherited simulation key types the weapon slot
    // (ProxyRPG7_Proxy : ProxySecWeapon inherits "ProxySecWeapon").
    char clsName[256];
    snprintf(clsName, sizeof(clsName), "Proxy%s", proxyName);
    for (char* p = clsName; *p; p++)
    {
        if (*p == ' ')
        {
            *p = '_';
        }
    }
    const ParamEntry* cls = nonAIVehiclesCfg->FindEntry(clsName);
    if (!cls || !cls->IsClass())
    {
        return false;
    }
    RString sim = cls->ReadValue("simulation", RString());
    // launchers and pistols hide; proxyweapon (the primary rifle) and every
    // non-weapon proxy draw
    return sim.GetLength() > 0 && (stricmp(sim, "proxysecweapon") == 0 || stricmp(sim, "proxyhandgun") == 0);
}

RString GuerrillaFactionsDescriptionPath(RString island, RString bankPrefix)
{
    if (bankPrefix.GetLength() > 0)
    {
        return bankPrefix + RString("description.ext");
    }
    return GuerrillaTemplateMissionBase(island) + RString("\\description.ext");
}

GuerrillaNewGame::GuerrillaNewGame(ControlsContainer* parent) : Display(parent)
{
    _exitWhenClose = -1;

    // Prefer a dedicated resource when the game data ships one; fall back to
    // the vanilla island-selection layout; load nothing when no menu
    // resources exist (headless runs) — see the class comment.
    const char* rsc = nullptr;
    if (Res.FindEntry("RscDisplayGuerrillaNewGame"))
    {
        rsc = "RscDisplayGuerrillaNewGame";
    }
    else if (Res.FindEntry("RscDisplaySelectIsland"))
    {
        rsc = "RscDisplaySelectIsland";
    }
    if (rsc)
    {
        Load(rsc);
    }
    // Never keep the resource's idd — the reused RscDisplaySelectIsland
    // carries the editor's IDD_SELECT_ISLAND, and DisplayMain keys the
    // Guerrilla launch on IDD_GUERRILLA_NEW_GAME.
    _idd = IDD_GUERRILLA_NEW_GAME;

    // The island-select wizard button belongs to the editor flow.
    if (IControl* wizard = GetCtrl(IDC_SELECT_ISLAND_WIZARD))
    {
        wizard->ShowCtrl(false);
    }

    // The island list (when present) is already populated with a current
    // selection by Load() above, so SelectedIsland() resolves to a real
    // island here — pull that island's own faction roster before wiring up
    // the cyclers, instead of the never-populated global Pars lookup.
    RefreshFactionsForIsland(SelectedIsland());

    // The BODY browser roster comes from the loaded package (Pars), not from
    // any island's template, so one build here covers the display's life.
    _bodies = GuerrillaListPlayerBodies(Pars.FindEntry("CfgVehicles"),
                                        [](RString path) { return QIFStreamB::FileExist(path); });

    InjectFactionCyclers();
}

Control* GuerrillaNewGame::OnCreateCtrl(int type, int idc, const ParamEntry& cls)
{
    switch (idc)
    {
        case IDC_SELECT_ISLAND:
        {
            // Same enumeration as the editor's DisplaySelectIsland: every
            // CfgWorldList class whose .wrp actually exists on disk.
            C3DListBox* lbox = new C3DListBox(this, idc, cls);
            std::vector<RString> islands = GuerrillaListIslands(Pars.FindEntry("CfgWorldList"), [](RString name)
                                                                { return QIFStreamB::FileExist(GetWorldName(name)); });
            const ParamEntry* worlds = Pars.FindEntry("CfgWorlds");
            int sel = 0;
            for (const RString& name : islands)
            {
                RString description = name;
                if (worlds)
                {
                    if (const ParamEntry* world = worlds->FindEntry(name))
                    {
                        description = world->ReadValue("description", name);
                    }
                }
                // An installed world is only launchable when its
                // "Guerrilla.<World>" template is installed too — say so in
                // the list instead of letting OK fail with a message box.
                if (!GuerrillaTemplateExists(
                        name, [](RString p) { return FilePathExists(p); },
                        [](RString p) { return QIFStreamB::FileExist(p); }))
                {
                    description = description + RString(" (no Guerrilla template)");
                }
                int index = lbox->AddString(description);
                lbox->SetData(index, name);
                if (stricmp(name, Glob.header.worldname) == 0)
                {
                    sel = index;
                }
            }
            if (lbox->GetSize() > 0)
            {
                lbox->SetCurSel(sel);
            }
            return lbox;
        }
    }
    return Display::OnCreateCtrl(type, idc, cls);
}

ControlObject* GuerrillaNewGame::OnCreateObject(int type, int idc, const ParamEntry& cls)
{
    if (idc == kIdcOutfitPreview)
    {
        // the DisplayNewUser::OnCreateObject -> CHead hook, for the mannequin
        return new GuerrillaOutfitPreview(this, idc, cls);
    }
    return Display::OnCreateObject(type, idc, cls);
}

void GuerrillaNewGame::OnSimulate(EntityAI* vehicle)
{
    // Turntable for the mannequin: the DisplayNewUser::OnSimulate ->
    // CHead::Simulate pattern (UI displays get no other per-frame tick).
    if (auto* preview = dynamic_cast<GuerrillaOutfitPreview*>(GetCtrl(kIdcOutfitPreview)))
    {
        if (preview->IsVisible())
        {
            preview->SimulateTurntable();
        }
    }
    Display::OnSimulate(vehicle);
}

void GuerrillaNewGame::InjectFactionCyclers()
{
    // Clone the main menu's Quit button class so the cyclers pick up the
    // menu's font/colors/sounds — the exact technique DisplayMain uses to
    // inject the MODS entry (see the long comment there for why the local
    // idc/text/style entries must be added BEFORE SetBase).
    const char* mainRsc = Res.FindEntry("RscDisplayMainRemaster") ? "RscDisplayMainRemaster" : "RscDisplayMain";
    ParamEntry* quitCls = Res.FindEntry(mainRsc) ? (Res >> mainRsc).FindEntry("Quit") : nullptr;
    if (quitCls == nullptr || !quitCls->IsClass())
    {
        return; // no styled menu resource — selections stay at their defaults
    }

    // Outlives the injected controls — like DisplayMain's s_modsInjectCfg,
    // each control back-references its ParamClass for the display's lifetime.
    static ParamFile s_guerrillaCyclerCfg;
    s_guerrillaCyclerCfg.Clear();
    const struct
    {
        const char* cls;
        int idc;
        float y;
    } slots[] = {
        {"GuerrillaBody", kIdcBody, kCyclerBodyY},
        {"GuerrillaOutfit", kIdcOutfit, kCyclerOutfitY},
        {"GuerrillaOccupier", kIdcOccupier, kCyclerOccupierY},
        {"GuerrillaResistance", kIdcResistance, kCyclerResistanceY},
    };
    for (const auto& slot : slots)
    {
        ParamClass* cls = s_guerrillaCyclerCfg.AddClass(slot.cls);
        cls->Add("idc", slot.idc);
        cls->Add("text", RString(""));
        cls->Add("style", ST_LEFT);
        cls->SetBase(quitCls->GetClassInterface());
        LoadControl(*cls);
        if (Control* ctrl = dynamic_cast<Control*>(GetCtrl(slot.idc)))
        {
            ctrl->SetPos(kCyclerX, slot.y, kCyclerW, kCyclerH);
        }
        UpdateFactionLabel(slot.idc);
    }

    // The cyclers exist now, so the preview gate is open; show the body the
    // opening selections resolve to.
    UpdateOutfitPreview();
}

void GuerrillaNewGame::UpdateOutfitPreview()
{
    // Gate on the styled outfit cycler being present: with no menu resources
    // (headless runs) this display owns no controls and must keep owning
    // none, and a mannequin without its cycler label would be unreadable.
    if (!GetCtrl(kIdcOutfit))
    {
        return;
    }
    // The BODY browser's explicit pick previews directly; on its "(match
    // outfit)" default the mannequin falls back to the outfit-resolution
    // chain, exactly what the launch-time substitution seam will do.
    RString previewClass = SelectedPlayerClass();
    if (previewClass.GetLength() == 0)
    {
        previewClass =
            GuerrillaOutfitPreviewClass(_islandFactions, _islandZones, SelectedResistance(), SelectedOutfit());
    }
    RString model = GuerrillaOutfitPreviewModel(Pars.FindEntry("CfgVehicles"), previewClass,
                                                [](RString path) { return QIFStreamB::FileExist(path); });
    GuerrillaOutfitPreview* preview = dynamic_cast<GuerrillaOutfitPreview*>(GetCtrl(kIdcOutfitPreview));
    if (model.GetLength() == 0)
    {
        // plan-15 shaped degrade: no descriptor key, unknown class or a
        // missing shape file just hides the preview.
        if (preview)
        {
            preview->ShowCtrl(false);
        }
        return;
    }
    if (!preview)
    {
        // First model that resolved: inject the CT_OBJECT now. Function-
        // static lifetime for the same reason as s_guerrillaCyclerCfg: the
        // control back-references its ParamClass for the display's lifetime.
        static ParamFile s_guerrillaPreviewCfg;
        s_guerrillaPreviewCfg.Clear();
        ParamClass* cls = s_guerrillaPreviewCfg.AddClass("GuerrillaOutfitPreview");
        cls->Add("type", CT_OBJECT);
        cls->Add("idc", kIdcOutfitPreview);
        cls->Add("model", model);
        ParamEntry* position = cls->AddArray("position");
        position->AddValue(0.0f);
        position->AddValue(0.0f);
        position->AddValue(kPreviewDepthCfg); // PlaceInSlot refines below
        ParamEntry* direction = cls->AddArray("direction");
        direction->AddValue(0.0f);
        direction->AddValue(0.0f);
        direction->AddValue(1.0f);
        ParamEntry* up = cls->AddArray("up");
        up->AddValue(0.0f);
        up->AddValue(1.0f);
        up->AddValue(0.0f);
        cls->Add("scale", 1.0f);
        LoadObject(*cls);
        preview = dynamic_cast<GuerrillaOutfitPreview*>(GetCtrl(kIdcOutfitPreview));
        if (!preview)
        {
            return;
        }
    }
    // On the injection pass the constructor already loaded this model and
    // SetModel just records the name (the shape bank caches the reload); on
    // later passes it swaps the shape when the body actually changed.
    preview->SetModel(model);
    preview->PlaceInSlot();
    preview->ShowCtrl(true);
}

void GuerrillaNewGame::RefreshFactionsForIsland(RString island)
{
    // CfgGuerrillaFactions is authored per-island, inside that island's own
    // Guerrilla.<island> template mission's description.ext (see
    // guerrilla-mode/mission/*/description.ext) — it is never an addon's
    // global config, so Pars does not carry it. Peek the template's own
    // description.ext the same way the single-mission browser previews a
    // banked mission's overview (CreateSingleMissionBank), without running
    // the full SetMission()/launch path.
    //
    // Lifetime: _islandCfg.Clear() invalidates every ParamEntry* previously
    // handed out of it, and OnLBSelChanged re-enters here on every island
    // change — so the two pointers must be nulled BEFORE the Clear and
    // re-derived after the Parse. _occupiers/_resistances hold owning RString
    // copies and are unaffected.
    // Keep the player's picks across a refresh by NAME, not by index: the two
    // lists are rebuilt from scratch below and an index means nothing against
    // the new one.
    RString keepOccupier = SelectedOccupier();
    RString keepResistance = SelectedResistance();

    _islandFactions = nullptr;
    _islandZones = nullptr;
    _islandCfg.Clear();
    _islandForFactions = island;

    RString missionBase = GuerrillaTemplateMissionBase(island);
    RString bankPrefix;
    if (FilePathExists(missionBase + RString(".pbo")))
    {
        bankPrefix = CreateSingleMissionBank(missionBase);
    }
    bool unbanked = bankPrefix.GetLength() == 0 && QIFStreamB::FileExist(missionBase + RString("\\mission.sqm"));
    if (bankPrefix.GetLength() > 0 || unbanked)
    {
        RString descPath = GuerrillaFactionsDescriptionPath(island, bankPrefix);
        if (QIFStreamB::FileExist(descPath))
        {
            _islandCfg.Parse(descPath);
            _islandFactions = _islandCfg.FindEntry("CfgGuerrillaFactions");
            _islandZones = _islandCfg.FindEntry("CfgGuerrillaZones");
        }
    }
    // No per-island descriptor (missing template, or a template with no
    // CfgGuerrilla* of its own) — fall back to a global config an addon might
    // publish, then to an empty list (cyclers show "(mission default)"; OK
    // still works via the mission's own default* keys). Pars lives for the
    // process, so storing a pointer into it is safe. Both entries fall back
    // INDEPENDENTLY: sourcing the factions from Pars while leaving the zones
    // null left the OK guard reading playerSide off nothing, so it took its
    // legacy branch and blocked pairs the registry rebases happily — and left
    // the cyclers with no default* keys to seed from.
    if (!_islandFactions)
    {
        _islandFactions = Pars.FindEntry("CfgGuerrillaFactions");
    }
    if (!_islandZones)
    {
        _islandZones = Pars.FindEntry("CfgGuerrillaZones");
    }
    // One list, both cyclers: any faction may occupy or resist, and the
    // registry rebases whatever the pick collides with.
    _occupiers = GuerrillaListFactions(_islandFactions);
    _resistances = GuerrillaListFactions(_islandFactions);
    // Seed from the template's own defaultOccupier/defaultResistance so that
    // opening this screen and pressing OK launches exactly what a direct,
    // no-UI launch of the same template would. The lists used to be empty
    // (the descriptor was looked up in Pars, which never carries it), so the
    // selections resolved to EMPTY, nothing was published and the default*
    // keys always won — that is the contract this restores now that the lists
    // are real. Seeding 0/0 instead would both override those keys silently
    // and open on occupier == resistance.
    GuerrillaDefaultSelections(_islandFactions, _islandZones, _occupiers, _occupierSel, _resistanceSel);
    // A pick the new roster still carries survives the refresh.
    int keptOccupier = GuerrillaIndexOfSelection(_islandFactions, _occupiers, keepOccupier);
    if (keptOccupier >= 0)
    {
        _occupierSel = keptOccupier;
    }
    int keptResistance = GuerrillaIndexOfSelection(_islandFactions, _resistances, keepResistance);
    if (keptResistance >= 0)
    {
        _resistanceSel = keptResistance;
    }
    // Cyclers don't exist yet on the first call (constructor, before
    // InjectFactionCyclers) — UpdateFactionLabel no-ops safely via its own
    // GetCtrl null check.
    UpdateFactionLabel(kIdcOccupier);
    UpdateFactionLabel(kIdcResistance);
    // The outfit pair is read off the (possibly re-seeded) resistance block.
    RefreshOutfitChoices();
}

void GuerrillaNewGame::RefreshOutfitChoices()
{
    // Keep the player's pick by NAME across the rebuild — same contract as
    // the faction cyclers (an index means nothing against a new list).
    RString keep = SelectedOutfit();
    _outfits = GuerrillaOutfitChoices(_islandFactions, _islandZones, SelectedResistance());
    _outfitSel = _outfits.empty() ? -1 : 0; // WARRIOR default (index 0)
    for (int i = 0; i < (int)_outfits.size(); i++)
    {
        if (keep.GetLength() > 0 && stricmp(_outfits[i], keep) == 0)
        {
            _outfitSel = i;
            break;
        }
    }
    UpdateFactionLabel(kIdcOutfit);
    // Island changes and resistance cycling both land here, so this one call
    // keeps the mannequin tracking every path that can change the body.
    UpdateOutfitPreview();
}

void GuerrillaNewGame::UpdateFactionLabel(int idc)
{
    IControl* ctrl = GetCtrl(idc);
    if (!ctrl)
    {
        return;
    }
    char buffer[256];
    if (idc == kIdcBody)
    {
        // "BODY: <SIDE> <class>" — the exact-text form the e2e asserts; the
        // default reads "(match outfit)" because that is literally what it
        // does: publish nothing and let the OUTFIT token resolve the body.
        if (_bodySel >= 0 && _bodySel < (int)_bodies.size())
        {
            snprintf(buffer, sizeof(buffer), "BODY: %s %s", (const char*)_bodies[_bodySel].side,
                     (const char*)_bodies[_bodySel].className);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "BODY: (match outfit)");
        }
        if (CActiveText* text = dynamic_cast<CActiveText*>(ctrl))
        {
            text->SetText(buffer);
        }
        else if (CStatic* text = dynamic_cast<CStatic*>(ctrl))
        {
            text->SetText(buffer);
        }
        return;
    }
    const char* prefix = idc == kIdcOccupier ? "OCCUPIER" : (idc == kIdcResistance ? "RESISTANCE" : "OUTFIT");
    const std::vector<RString>& list = idc == kIdcOccupier     ? _occupiers
                                       : idc == kIdcResistance ? _resistances
                                                               : _outfits;
    const int sel = idc == kIdcOccupier ? _occupierSel : (idc == kIdcResistance ? _resistanceSel : _outfitSel);
    // An empty list means no config offered a real choice — nothing will be
    // published and the mission's own defaults decide (defaultOccupier /
    // defaultResistance keys, or the authored mission.sqm class for the
    // outfit), so say so instead of showing a value the mission may override.
    const char* selected = (sel >= 0 && sel < (int)list.size()) ? (const char*)list[sel] : "(mission default)";
    snprintf(buffer, sizeof(buffer), "%s: %s", prefix, selected);
    if (CActiveText* text = dynamic_cast<CActiveText*>(ctrl))
    {
        text->SetText(buffer);
    }
    else if (CStatic* text = dynamic_cast<CStatic*>(ctrl))
    {
        text->SetText(buffer);
    }
}

void GuerrillaNewGame::OnButtonClicked(int idc)
{
    switch (idc)
    {
        case kIdcOccupier:
            if (!_occupiers.empty())
            {
                _occupierSel = (_occupierSel + 1) % (int)_occupiers.size();
            }
            UpdateFactionLabel(idc);
            break;
        case kIdcResistance:
            if (!_resistances.empty())
            {
                _resistanceSel = (_resistanceSel + 1) % (int)_resistances.size();
            }
            UpdateFactionLabel(idc);
            // The outfit pair is authored per resistance block — the new
            // roster may or may not offer a civilian body.
            RefreshOutfitChoices();
            break;
        case kIdcOutfit:
            if (!_outfits.empty())
            {
                _outfitSel = (_outfitSel + 1) % (int)_outfits.size();
            }
            UpdateFactionLabel(idc);
            UpdateOutfitPreview();
            break;
        case kIdcBody:
            // default (-1) -> 0 -> ... -> n-1 -> default: the no-op is a
            // regular ring stop, so cycling past the roster restores the
            // untouched-screen behaviour instead of trapping the player on
            // an explicit pick.
            if (!_bodies.empty())
            {
                _bodySel = _bodySel + 1 >= (int)_bodies.size() ? -1 : _bodySel + 1;
            }
            UpdateFactionLabel(idc);
            UpdateOutfitPreview();
            break;
        case IDC_CANCEL:
        {
            // Close the notebook with its animation first (matching
            // DisplaySelectIsland); exit immediately when the resource has no
            // animated notebook (or no controls loaded at all).
            ControlObjectContainerAnim* ctrl =
                dynamic_cast<ControlObjectContainerAnim*>(GetCtrl(IDC_SELECT_ISLAND_NOTEBOOK));
            if (ctrl)
            {
                _exitWhenClose = idc;
                ctrl->Close();
            }
            else
            {
                Exit(idc);
            }
        }
        break;
        case IDC_OK:
        {
            // A pair the registry cannot resolve to two distinct sides would
            // spawn the campaign fighting itself — keep the player in the
            // dialog. The descriptor comes from the SELECTED ISLAND's own
            // template (Pars never carries CfgGuerrillaFactions, which is why
            // this guard used to be unreachable).
            RString message;
            if (!GuerrillaSelectionIsResolvable(_islandFactions, _islandZones, SelectedOccupier(), SelectedResistance(),
                                                message))
            {
                CreateMsgBox(MB_BUTTON_OK, message);
                break;
            }
            // The launch itself runs in DisplayMain::OnChildDestroyed
            // (IDD_GUERRILLA_NEW_GAME) after the base Exit(IDC_OK).
            Display::OnButtonClicked(idc);
        }
        break;
        default:
            Display::OnButtonClicked(idc);
            break;
    }
}

void GuerrillaNewGame::OnLBDblClick(int idc, int curSel)
{
    if (idc == IDC_SELECT_ISLAND)
    {
        OnButtonClicked(IDC_OK);
    }
    else
    {
        Display::OnLBDblClick(idc, curSel);
    }
}

void GuerrillaNewGame::OnLBSelChanged(int idc, int curSel)
{
    if (idc == IDC_SELECT_ISLAND)
    {
        // Different islands ship different rosters (e.g. Guerrilla.Demo's
        // vanilla EAST/GUER vs Guerrilla.Sinai's @LoBo IDF/EgyptFrontier) —
        // re-peek the newly selected island's own template instead of
        // leaving the previous island's faction list on screen.
        //
        // Only when the island ACTUALLY changed. C3DListBox::OnLButtonUp calls
        // SetCurSel with sendUpdate=true, which skips SetCurSel's
        // same-index early-out — so this fires on every click, including a
        // click on the already-selected row. Refreshing there would re-seed
        // the cyclers from the template defaults and throw the player's picks
        // away, which double-click-to-launch (OnLBDblClick -> IDC_OK) does in
        // one gesture: LButtonUp lands first, then OK publishes the reset
        // pair. It would also unmount and remount the template PBO per
        // mouse-up.
        RString island = SelectedIsland();
        if (island.GetLength() > 0 && stricmp(island, _islandForFactions) != 0)
        {
            RefreshFactionsForIsland(island);
        }
    }
    Display::OnLBSelChanged(idc, curSel);
}

void GuerrillaNewGame::OnCtrlClosed(int idc)
{
    if (idc == IDC_SELECT_ISLAND_NOTEBOOK)
    {
        Exit(_exitWhenClose);
    }
    else
    {
        Display::OnCtrlClosed(idc);
    }
}

RString GuerrillaNewGame::SelectedIsland()
{
    if (C3DListBox* lbox = dynamic_cast<C3DListBox*>(GetCtrl(IDC_SELECT_ISLAND)))
    {
        int sel = lbox->GetCurSel();
        if (sel >= 0)
        {
            return lbox->GetData(sel);
        }
    }
    // No island list (no menu resources) — the current menu cutscene world is
    // always a valid installed world.
    return Glob.header.worldname;
}

RString GuerrillaNewGame::SelectedOccupier() const
{
    if (_occupierSel >= 0 && _occupierSel < (int)_occupiers.size())
    {
        return _occupiers[_occupierSel];
    }
    // No real faction config was offered — return an EMPTY selection so the
    // launch path publishes nothing and the mission's own
    // defaultOccupier/defaultResistance config keys keep precedence
    // (ZoneRegistry::LoadFromParams). Publishing the built-in "EAST" here
    // used to match a mission faction by side and silently override them.
    return RString();
}

RString GuerrillaNewGame::SelectedResistance() const
{
    if (_resistanceSel >= 0 && _resistanceSel < (int)_resistances.size())
    {
        return _resistances[_resistanceSel];
    }
    return RString(); // see SelectedOccupier
}

RString GuerrillaNewGame::SelectedPlayerClass() const
{
    if (_bodySel >= 0 && _bodySel < (int)_bodies.size())
    {
        return _bodies[_bodySel].className;
    }
    // "(match outfit)" default — publish nothing so the outfit token (or the
    // authored mission.sqm class) keeps deciding (see SelectedOccupier).
    return RString();
}

RString GuerrillaNewGame::SelectedOutfit() const
{
    if (_outfitSel >= 0 && _outfitSel < (int)_outfits.size())
    {
        return _outfits[_outfitSel];
    }
    // No descriptor offered an outfit pair — publish nothing so the authored
    // mission.sqm class keeps deciding (see SelectedOccupier).
    return RString();
}

void __cdecl CreateDisplayGuerrilla(ControlsContainer* parent)
{
    parent->CreateChild(new GuerrillaNewGame(parent));
}

} // namespace Poseidon
