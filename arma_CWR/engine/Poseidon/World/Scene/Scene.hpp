#pragma once

#include <Poseidon/Core/Types.hpp>
#include <Poseidon/Graphics/Textures/TexturePreload.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp>
#include <Poseidon/Core/Config/EngineConfig.hpp>

#define HORIZONT_Z ENGINE_CONFIG.horizontZ
#define OBJECT_Z ENGINE_CONFIG.objectsZ

#define MIN_FOG (200.0f)
#define MAX_FOG (HORIZONT_Z - 20)

#define MIN_SHADOWFOG (100.0f)
#define MAX_SHADOWFOG (ENGINE_CONFIG.shadowsZ)

#include <Poseidon/Foundation/Memory/MemFreeReq.hpp>

namespace Poseidon
{
const float CloudScale = 0.1;

const float MinSkyFog = 4000.0f * CloudScale;
const float MaxSkyFog = 19000.0f * CloudScale;

#define LEN_FOG_TABLE 256
class FogFunction
{
  private:
    byte _fog[LEN_FOG_TABLE];
    float _start2, _end2;
    float _divisor;

  public:
    FogFunction();
    void Set(float start, float end, float (*function)(float distRel, float start, float end));
    int operator()(float distSquare) const; // avoid partial stall
};

class SortObject : public RefCount
{
  public:
    Ref<Object> object;        // must outlive drawing; Ref guarantees the shape exists
    LODShapeWithShadow* shape; // randomized shape

    signed char drawLOD, shadowLOD;
    signed char passNum;
    signed char forceDrawLOD; // forced draw LOD
    // The hill cull hid the object this frame (Pass1): it keeps its draw LOD,
    // so the sorts order the list as if it were drawn and the shadow-map pass
    // casts its shadow from that LOD; only the draws skip it.
    bool hillHidden = false;

    // EX2-POP (LodPop.hpp): the level AdjustComplexity chose last frame, before
    // the occlusion tests hid anything, and the pop counters' memory.
    signed char heldLOD = -2;
    signed char popShown = -2; // what was on screen last frame
    signed char popPrev = -2;  // what was on screen before the last change
    unsigned char popWhy = 0;  // why it was hidden: 1 cull, 2 occlusion, 3 hill
    unsigned short popAge = 0; // frames since the last change
    // The fade in progress: the level going away (LOD_INVISIBLE: the object is
    // fading in from nothing; -2: no fade), how far it has gone, and whether the
    // object itself is going away, in which case drawLOD is the level fading out.
    signed char fadeFrom = -2;
    bool fadeOut = false;
    float fadeT = 0.0f;
    // Round 2: the level this object cast into the sun shadow maps this frame
    // and last (LOD_INVISIBLE: none; -2: not seen yet), and why it cast nothing
    // (1 past the reach, 2 off screen, 3 too small, 4 hidden by an object, 5 no
    // level). Written only while POSEIDON_XP_LODLOG is set.
    signed char smCast = -2;
    signed char smCastPrev = -2;
    unsigned char smWhy = 0;
    // POSEIDON_XP_SHADOWFADE: the level this object cast last, the frame it
    // cast it in, and the dissolve in progress (the level going away, -2: none).
    signed char smHeld = -2;
    signed char smFadeFrom = -2;
    unsigned smFrame = 0;
    float smFadeT = 0.0f;

    unsigned char orClip; // or clip flags only - andClip would always be 0

    bool notUsed; // not used when list was created - delete it

    float radius; // bounding sphere radius

    float distance2;

    float zCoord; // alpha-pass sort key: camera-space depth; see AlphaSortOrder.hpp

    USE_FAST_ALLOCATOR
};

typedef RefArray<SortObject> SortObjectList;

class RemmemberShadow : public RefCount, public CLRefLink
{
    friend class ShadowCache;

  private:
    Ref<Shape> _shadow;
    OLink<Object> _object;
    Vector3 _lightDir;
    Matrix4 _objectPos;
    int _level; // which LOD of object is used
    Foundation::Time _lastUsed;
    bool _splitOnly; // shadow cache is also used to contain split surfaces

  public:
    RemmemberShadow();
    // init shadow or split to fit on surface
    void Init(Object* object, Vector3Par lightDir, int level, Matrix4Par pos, bool splitOnly = false);
    bool IsShadow(Object* object, int level) const;
    Object* GetObject() const { return _object; }
    Vector3Val LightDir() const { return _lightDir; }
    Matrix4Val ObjectPos() const { return _objectPos; }

    USE_FAST_ALLOCATOR
};

class ShadowCache : public Foundation::MemoryFreeOnDemandHelper
{
  private:
    CLRefList<RemmemberShadow> _data;

  public:
    ShadowCache();
    ~ShadowCache() override;

    Ref<Shape> Shadow(Object* object, Vector3Par lightDir, int level, Matrix4Par pos, bool splitOnly = false);
    void ShadowChanged(Object* obj);
    void Clear();
    void CleanUp();

    size_t FreeOneItem() override;
    float Priority() override;

    // memory-budget observability (dev panel). ~10 KB per remembered shadow,
    // matching ShadowMemSize in Shadow.cpp.
    const char* DomainName() const override { return "Shadows.Cache"; }
    size_t HeldBytes() const override { return (size_t)_data.Size() * (10 * 1024); }
};

enum PreloadedShape
{
    CobraLight,
    SphereLight,
    HalfLight,
    Marker,
    CraterShell,
    SlopBlood,
    CloudletBasic,
    CloudletFire,
    CloudletWater,
    Cloud1,
    Cloud2,
    Cloud3,
    Cloud4,
    CinemaBorder,

    FootStepL,
    FootStepR,

    ForceArrowModel,
    SphereModel,
    RectangleModel,
    BulletLine,

    MaxPreloadedShape
};

typedef Ref<Light> ActiveLightPointer;

class LightList : public FindArray<ActiveLightPointer, Foundation::MemAllocSS>
{
  public:
    LightList(bool staticStorage = false);
    LightList(const LightList& src);
};

class PreloadedTextures
{
    RefArray<Texture> _data;

  public:
    PreloadedTextures();
    ~PreloadedTextures();

    void Preload(bool all);
    void Clear();

    Texture* New(RStringB name);                   // make texture permanent
    Texture* New(::Poseidon::PreloadedTexture id); // predefined texture ids
};

extern PreloadedTextures GPreloadedTextures;

// Scene graph management: LOD and light management.
class Scene
{
  private:
    Color _constantColor;
    Color _skyColor;
    float _constantFog;

    Ref<Texture> _skyTexture;

    Ref<LODShapeWithShadow> _preloaded[MaxPreloadedShape];

    Camera* _camera;
    LightSun* _mainLight;

    FindArray<Link<Light>> _lights;
    LightList _aLights;

    Ref<Object> _collisionStar;

    // Rain you can see (POSEIDON_XP_RAIN, Graphics/Remaster/RainXp.hpp): the
    // overlay DrawRain held back for DrawRainLate (mode late).
    struct RainLate
    {
        bool pending = false;
        float alpha = 0.0f, yDensity = 0.0f, xOffset = 0.0f, yOffset = 0.0f;
    } _rainLate;

    // fog functions for different types of objects
    FogFunction _fog, _skyFog, _shadowFog, _tacticalFog;
    float _tacticalVisibility;                    // AI sensors
    float _rainRange;                             // display boundaries main control
    float _fogMaxRange, _fogMinRange;             // display boundaries
    float _shadowFogMaxRange, _shadowFogMinRange; // display boundaries

    mutable float _lodInvWidth;

    float _frameRateSettings;
    float _qualitySettings;

    mutable Foundation::UITime _lastScaleBetterTime; // avoid oscillation
    mutable Foundation::UITime _lastScaleWorseTime;
    mutable float _maxTargetFrameDuration;
    mutable float _minTargetFrameDuration;

    mutable float _minLodInvWidth; // visual quality limits
    mutable float _maxLodInvWidth;

    enum
    {
        NStoreComplexities = 4
    };

    mutable int _lastComplexity[NStoreComplexities]; // complexity history

    mutable SortObjectList _drawObjects;
    mutable SortObjectList _drawMergers;

    mutable ShadowCache _shadowCache;
    SRef<Landscape> _landscape; // only pointer to landscape

    bool _objectShadows, _vehicleShadows, _cloudlets;
    float _objectLODBias = 1.0f;
    float _lodPinFactor = 0.0f; // PinObjectDetail: 0 = adaptive
    int _drawnObjects = 0;      // DrawnObjectCount
    int _hillHiddenObjects = 0; // HillHiddenObjectCount

    // POSEIDON_XP_SHADOWKEEP (LodPop.hpp), refreshed in BeginObjects: the sun
    // ray the shadow maps use, their far distance, and the camera this frame.
    struct ShadowKeep
    {
        bool on = false;
        Vector3 sun = VZero;     // unit, the way the light travels
        Vector3 eye = VZero;     // camera position
        Vector3 forward = VZero; // camera forward
        float sunForward = 0.0f; // sun . forward
        float mapFar = 0.0f;     // receivers past this eye depth get no shadow
        float corner = 1.0f;     // distance of the frustum corner per metre of eye depth
        float cellLen = 0.0f;    // shadow length of a grid cell's tallest object
    };
    ShadowKeep _shadowKeep;
    float _preferredTerrainGrid;
    float _preferredViewDistance;

  public:
    Scene();
    void Init(Engine* engine, Landscape* landscape);
    void ResetFog();
    void CleanUp();
    ~Scene();

    bool GetObjectShadows() const { return _objectShadows; }
    bool GetVehicleShadows() const { return _vehicleShadows; }
    bool GetCloudlets() const { return _cloudlets; }

    float GetMinimalTerrainGrid() const;
    float GetPreferredTerrainGrid() const { return _preferredTerrainGrid; }
    float GetPreferredViewDistance() const { return _preferredViewDistance; }

    void SetPreferredTerrainGrid(float x);
    void SetPreferredViewDistance(float x);

    void SetObjectShadows(bool set = true);
    void SetVehicleShadows(bool set = true);
    void SetCloudlets(bool set = true);

    // Object LOD bias driving entity-LOD selection — multiplier on the
    // entity's projected screen size before LOD lookup.  >1 picks a
    // higher LOD (more detail) at a given distance; <1 picks lower.
    // Range clamped 0.25..4.0 in setter.  Read by RenderShape /
    // LODSelect path; default 1.0 (no bias).
    void SetObjectLODBias(float bias);
    float GetObjectLODBias() const { return _objectLODBias; }

    // Object detail for scripted A/B captures (triPinObjectDetail). By default
    // (0) AdjustComplexity picks the LOD width from the measured frame time,
    // so two shots of one view can draw different objects. A factor >= 1
    // holds the width at that multiple of the finest the quality setting
    // allows (1 = finest, 16 = coarsest, the adaptive range).
    void PinObjectDetail(float factor);
    float GetObjectDetailPin() const { return _lodPinFactor; }
    // Objects with a visible LOD in the last Pass1 draw list, before occlusion
    // culling: the same for both lighting models on one view at one detail.
    int DrawnObjectCount() const { return _drawnObjects; }
    // Of those, the objects the hill cull hid behind the terrain in the same
    // Pass1 (they still count in DrawnObjectCount, so capture tables compare).
    int HillHiddenObjectCount() const { return _hillHiddenObjects; }

    // POSEIDON_XP_SHADOWKEEP: can the shadow of an object (of a grid cell of
    // objects) outside the screen reach ground the shadow maps shade; is a
    // caster inside the maps' reach; how many grid cells past the camera's
    // rectangle may still hold such casters.
    void UpdateShadowKeep();
    bool ShadowKeepOn() const { return _shadowKeep.on; }
    Vector3 ShadowKeepSun() const { return _shadowKeep.sun; }
    bool ShadowKeepObject(Vector3Par pos, float radius) const;
    bool ShadowKeepCell(Vector3Par center, float radius) const;
    bool ShadowKeepInReach(Vector3Par pos, float radius) const;
    int ShadowKeepBorder(float grid) const;
    // The sun (or moon) ray the shadow maps are rendered along this frame.
    Vector3 SunShadowMapDirection() const;
    // The far distance of the shadow maps' casters and receivers this frame.
    float ShadowMapCasterFar() const;

    Camera* GetCamera() { return _camera; }
    const Camera* GetCamera() const { return _camera; }
    void SetCamera(const Camera& camera);

    void SetConstantColor(ColorVal color) { _constantColor = color; }
    ColorVal GetConstantColor() const { return _constantColor; }

    void SetConstantFog(float fog) { _constantFog = fog; }
    float GetConstantFog() const { return _constantFog; }

    const Matrix4& ScaledInvTransform() const;
    const Matrix3& CamNormalTrans() const;
    const Matrix4& CamInvTrans() const;

    Texture* SkyTexture() const { return _skyTexture; }

    void ResetLights();
    void AddLight(Light* light);

    Light* GetLight(int i) const { return _lights[i]; }
    int NLights() const { return _lights.Size(); }

    void SelectActiveLights(Object* dimmed);
    void SetActiveLights(const LightList& lights);
    const LightList& ActiveLights() const { return _aLights; }

    // Select light affecting given position
    const LightList& SelectLights(Vector3Par pos, float radius, LightList& work); // may return work or something else

    // Select light affecting given object
    const LightList& SelectLights(Matrix4Par objPos, const Object* object, int level, LightList& work);

    LightSun* MainLight() const { return _mainLight; }
    void SetMainLight(LightSun* light) { _mainLight = light; }
    void MainLightChanged(); // fog/light color has been changed

    void SetTacticalVisibility(float tacVis, float rainRange);
    float GetTacticalVisibility() const { return _tacticalVisibility; }

    float GetLodInvWidth() const { return _lodInvWidth; }
    float GetSmokeGeneralization() const;

    float GetFrameRateSettings() const { return _frameRateSettings; }
    void SetFrameRateSettings(float val);
    RString GetFrameRateText() const;

    float GetQualitySettings() const { return _qualitySettings; }
    void SetQualitySettings(float val);
    RString GetQualityText() const;

    void LoadConfig();
    void SaveConfig() const;

    float GetFogMaxRange() const { return _fogMaxRange; }
    float GetFogMinRange() const { return _fogMinRange; }
    float GetShadowFogMaxRange() const { return _shadowFogMaxRange; }
    float GetShadowFogMinRange() const { return _shadowFogMinRange; }

    int TacticalFog8(float distSquare) const { return _tacticalFog(distSquare); }
    int Fog8(float distSquare) const { return _fog(distSquare); }
    int ShadowFog8(float distSquare) const { return _shadowFog(distSquare); }
    int SkyFog8(float distSquare) const { return _skyFog(distSquare); }

    void CalculateSkyColor(Texture* texture);

    Landscape* GetLandscape() const { return _landscape; }
    ShadowCache& GetShadowCache() const { return _shadowCache; }

    int LevelFromDistance2(LODShape* shape, float distance2, float oScale, Vector3Par direction,
                           Vector3Par viewDirection);
    int LevelHeldFromDistance2(LODShape* shape, float distance2, float oScale, int held, float band);
    // The resol2 LevelFromDistance2 hands FindSqrtLevel (POSEIDON_XP_LODBAND).
    float LodResolution2(float distance2) const;
    int LevelShadowFromDistance2(LODShape* shape, float distance2, float oScale, Vector3Par direction,
                                 Vector3Par viewDirection);

    void AdjustComplexity();

    int AdjustComplexity(SortObjectList& objs);
    int AdjustShadowComplexity(SortObjectList& objs);

    void BeginObjects();
    void ObjectForDrawing(Object* obj, int forceLOD, ClipFlags clip);
    void CloudletForDrawing(Object* obj);
    void ObjectForDrawing(Object* obj);

    void EndObjects(); // sort all objects
    void DrawReflections(const WaterLevel& water);
    void DrawObjectsAndShadowsPass1();
    void DrawObjectsAndShadowsPass2();
    // Shadow-map depth pass (off by default): collect the visible casters and render
    // the cascade depth maps from the sun.  Lives in SceneShadowPass.cpp to keep
    // Scene.cpp under the file-size limit.  Called from Pass2 when shadow maps are on.
    void RenderShadowMapDepthPass(int nDraw);
    void DrawObjectsAndShadowsPass3(); // last draw cockpits
    void ObjectsDrawn();               // release all temporary information

    // light color and position
    void DrawFlare(ColorVal color, Vector3Par pos, bool secondary = true);
    void DrawFlares();

    void DrawRainLevel(float alpha, float yDensity, float xOffset, float yOffset, float z);
    void DrawRain();
    // POSEIDON_XP_RAIN=late: the overlay DrawRain held back, drawn after the
    // post pass (World::Draw, right after EndWorldScene); nothing otherwise.
    void DrawRainLate();
    // A cockpit (pass 3) draws over the world this frame.
    bool CockpitThisFrame() const;

    void DrawDiagModel(Vector3Par pos, LODShapeWithShadow* shape, float size = 0.1, PackedColor color = PackedWhite);
    void DrawCollisionStar(Vector3Par pos, float size = 0.1, PackedColor color = PackedWhite);
    void DrawVolumeLight(LODShapeWithShadow* shape, PackedColor color, const Frame& pos, float size);

    bool ShadowPos(Vector3Par pos, Vector3& aprox, LightSun* light);

    LODShapeWithShadow* ForceArrow() const { return Preloaded(ForceArrowModel); }

    LODShapeWithShadow* Preloaded(PreloadedShape type) const
    {
        PoseidonAssert(type < MaxPreloadedShape);
        return _preloaded[type];
    }
    void SetPreloaded(PreloadedShape type, LODShapeWithShadow* shape)
    {
        PoseidonAssert(type < MaxPreloadedShape);
        _preloaded[type] = shape;
    }
    void SetCollisionStar(Object* obj) { _collisionStar = obj; }
    bool HasPreloaded() const { return _preloaded[CraterShell] != nullptr; }
    Texture* Preloaded(::Poseidon::PreloadedTexture type) const { return GPreloadedTextures.New(type); }
    Texture* Preloaded(RStringB name) const { return GPreloadedTextures.New(name); }

    void DrawExShadow(SortObject* oi); // exact shadow casting
};

extern Scene* GScene;
#define GLOB_SCENE (GScene)

#define IS_SHADOW_VEHICLE (GScene ? GScene->GetVehicleShadows() : true)
#define IS_SHADOW_OBJECT (GScene ? GScene->GetObjectShadows() : true)

} // namespace Poseidon
