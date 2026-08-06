#pragma once

// The turntable body-preview mannequin shared by the Guerrilla new-game
// screen (idc 154) and the character-select screen (idc 161): a
// ControlObject whose shape can be re-pointed at a different body model as
// the selection changes (the CHead shape-swap precedent, DisplayUIMenus.cpp),
// refitted to a caller-authored 2D slot from its bounding box, and
// yaw-rotated from the owning display's OnSimulate. Renders whatever pose the
// p3d authors (base pose); no UI control animates a Man skeleton, and that is
// accepted for this milestone. Extracted from GuerrillaNewGame.cpp so both
// displays drive one implementation (issue #43).

#include <Poseidon/UI/Controls/UIControls.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

namespace Poseidon
{
// Camera-space depth in the units ControlObject's constructor reads (it
// multiplies by CameraZoom): inside the 3D-UI ray range
// (ControlObject::IsInside probes out to 2.0 * CameraZoom). Shared by every
// display that authors a preview slot; only the 2D slot rectangle is
// per-display (PlaceInSlot parameters).
constexpr float kGuerrillaPreviewDepthCfg = 1.0f;
// Seconds per full turntable revolution. CHead spins a head in 4; a full
// body reads better a touch slower.
constexpr float kGuerrillaPreviewTurnPeriod = 8.0f;

class GuerrillaBodyPreview : public ControlObject
{
  public:
    GuerrillaBodyPreview(ControlsContainer* parent, int idc, const ParamEntry& cls);

    // Swap to a different body model (raw CfgVehicles `model` value, the
    // same string the injected config carried). No-op when unchanged; the
    // caller re-fits via PlaceInSlot either way.
    void SetModel(RString modelName);

    // Civilian rendering rule (issue #43 defect 3): when true, DrawProxies
    // hides EVERY weapon proxy including the primary rifle, so CIV-side
    // bodies preview unarmed. Callers derive the flag from the previewed
    // class's config side (GuerrillaClassIsCivilian) at SetModel time.
    void SetCivilian(bool civilian) { _civilian = civilian; }

    // The weapon-proxy models author their muzzle-flash ("zasleh") sections
    // VISIBLE in the p3d (ak_47_v58_proxy.p3d carries a permanent white
    // zasleh3 star). In-mission Man::DrawProxies substitutes the real weapon
    // model and re-hides its zasleh every frame keyed on firing
    // (SoldierOldSimProxy.cpp); the raw ControlObject render path has no such
    // pass, so hide the section once per body swap on every proxy shape (and
    // on the body itself, should one ever author it). AnimationSection::Hide
    // is persistent per-shape state, exactly the not-firing state the mission
    // pass maintains, and nothing ever Unhides these proxy placeholders.
    static void HideMuzzleFlashSections(LODShapeWithShadow* body);

    // Object::DrawProxies with two filters: the flag proxy never draws (its
    // model is hard-textured with a default US flag that only in-mission
    // flag-carrier machinery may rebind, so NO flag is the only rendering
    // that is safe for every faction), and of the authored weapon proxies
    // at most the primary rifle survives; launchers and pistols always hide,
    // and the rifle too when SetCivilian(true) applies (see
    // GuerrillaPreviewHideWeaponProxy).
    void DrawProxies(int level, ClipFlags clipFlags, const Matrix4& transform, const Matrix4& invTransform, float dist2,
                     float z2, const LightList& lights) override;

    // Fit the current shape into the caller's 2D slot: bounding-box centre
    // on the slot centre, model height scaled to the slot height. Per-shape,
    // so a LoBo body and a vanilla body render the same on-screen size no
    // matter where each p3d puts its origin (feet, waist, ...). The slot is
    // parameterized so each display authors its own placement; the depth is
    // the shared kGuerrillaPreviewDepthCfg.
    void PlaceInSlot(float cx, float topY, float bottomY);

    // CHead::Simulate's turntable at full-body cadence. SetOrientation
    // resets the scale part of the transform, so reapply the fit, exactly
    // like CHead reapplies Scale().
    void SimulateTurntable();

    void OnDraw(float alpha) override;

  private:
    RString _modelName;
    float _fitScale = 1.0f;
    bool _civilian = false;
};

} // namespace Poseidon
