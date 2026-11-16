#include <Poseidon/Core/Application.hpp>
#include <Poseidon/Core/Config/EngineConfig.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Input/InputSubsystem.hpp>
#include <SDL3/SDL_scancode.h>
#include <Random/randomGen.hpp>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <Poseidon/Foundation/Common/FltOpts.hpp>
#include <Poseidon/Foundation/Containers/BankArray.hpp>
#include <Poseidon/Foundation/Containers/StaticArray.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Math/MathDefs.hpp>
#include <Poseidon/Foundation/Math/MathOpt.hpp>
#include <Poseidon/Foundation/Memory/CheckMem.hpp>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <vector>

using Poseidon::Foundation::IsOutOfMemory;
using Poseidon::Foundation::MStorage;
using Poseidon::Foundation::Time;
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/LodPop.hpp>
#include <Poseidon/World/TriCamPath.hpp>
#include <Poseidon/Foundation/Platform/AppConfig.hpp>
#include <Poseidon/UI/Settings/GameSettingsConfig.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Shadow/ShadowXp.hpp>
#include <Poseidon/Graphics/Remaster/TemporalDeterminism.hpp>
#include <Poseidon/Graphics/Remaster/RainXp.hpp>
#include <Poseidon/Graphics/Textures/TextureBank.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/Graphics/Textures/TexturePreload.hpp>
#include <Poseidon/World/Scene/Object.hpp>
#include <Poseidon/World/Scene/SurfaceDrawOrder.hpp>
#include <Poseidon/World/Scene/AlphaSortOrder.hpp>
#include <Poseidon/World/Scene/ShapeDrawOrder.hpp>
#include <Poseidon/World/Scene/InstancedRunRules.hpp>
#include <Poseidon/World/Scene/TerrainOcclusion.hpp>
#include <Poseidon/Graphics/Core/PerfCounters.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/SunShadows.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/Graphics/Rendering/Primitives/Poly.hpp>
#include <Poseidon/Graphics/Shadow/ShadowMath.hpp>
#include <Poseidon/World/Simulation/Animation/Animation.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/AI/VehicleAI.hpp>
#include <Poseidon/Graphics/Rendering/Primitives/ClipVert.hpp>
#include <Poseidon/World/Entities/Weapons/Shots.hpp>
#include <Poseidon/World/Scene/ObjLine.hpp>
#include <Poseidon/World/Scene/ObjectClasses.hpp>
#include <Poseidon/World/Simulation/FrameInv.hpp>
#include <Poseidon/AI/AI.hpp> // remove dependency
#include <Poseidon/World/World.hpp>
#include <Poseidon/Foundation/Algorithms/Qsort.hpp>
#include <Poseidon/World/Scene/DrawListSort.hpp>
#include <Poseidon/UI/Locale/StringtableExt.hpp>
#include <time.h>
#include <Poseidon/Dev/Diag/DiagModes.hpp>
#include <Poseidon/World/Terrain/Occlusion.hpp>
#include <Poseidon/World/Scene/OcclusionRange.hpp>

using namespace Poseidon;

#define DRAW_OBJS 1

namespace Poseidon
{
extern bool EnableObjOcc;
SRef<Occlusion>& GetOcclusions();
} // namespace Poseidon

typedef Ref<SortObject> SortObjectItem;

// W3-11: the draw-list sorts read each entry's key once into a contiguous array
// and sort that (DrawListSort.hpp), into exactly the order QSort with the old
// per-entry comparators gave, ties included. The keys are the old comparators'
// inputs; the key comparisons are the old comparisons.

// The occluder list: the first object is the nearest one (was CmpRevDistObj).
static void SortNearestFirst(SortObjectItem* items, int n)
{
    static DrawListSort::Scratch<float, SortObject> scratch;
    DrawListSort::SortByKeys(
        items, n, [](const SortObject* o) { return o->distance2; },
        [](float a, float b) { return DrawListSort::CompareRevDistance(a, b); }, scratch);
}

// Far-to-near by camera-space depth, for the alpha pass (see AlphaSortOrder.hpp;
// was CmpRevAlphaSortObj).
static void SortAlphaFarToNear(SortObjectItem* items, int n)
{
    static DrawListSort::Scratch<float, SortObject> scratch;
    DrawListSort::SortByKeys(
        items, n, [](const SortObject* o) { return o->zCoord; },
        [](float a, float b) { return DrawListSort::CompareAlphaKeys(a, b); }, scratch);
}

// Pass ascending; between shapes, simple shapes first then shape identity; within a
// shape, coarse LODs (large LOD numbers) first, then back to front (helps alpha
// transparency). Shape identity compares addresses, never a truncated pointer
// difference (W1-7): see Scene/ShapeDrawOrder.hpp. Simple = the LOD-0 face count,
// read once per entry (a null LOD 0 counts 0; the old comparator dereferenced it).
static DrawListSort::ShapeKey MakeShapeKey(const SortObject* o)
{
    const LODShape* shape = o->object->GetShape();
    const Shape* level0 = shape->Level(0);
    return DrawListSort::ShapeKey{{o->passNum, shape, o->drawLOD, o->distance2}, level0 ? level0->NFaces() : 0};
}

static int CmpShapeObj(const SortObjectItem* p1, const SortObjectItem* p2)
{
    // no invisible LODs here
    return DrawListSort::CompareShapeKeys(MakeShapeKey(*p1), MakeShapeKey(*p2));
}

static void SortByShape(SortObjectItem* items, int n)
{
    static DrawListSort::Scratch<DrawListSort::ShapeKey, SortObject> scratch;
    DrawListSort::SortByKeys(
        items, n, [](const SortObject* o) { return MakeShapeKey(o); },
        [](const DrawListSort::ShapeKey& a, const DrawListSort::ShapeKey& b)
        { return DrawListSort::CompareShapeKeys(a, b); }, scratch);
}

void Scene::EndObjects()
{
    // sort by screen size
}

void Scene::DrawFlare(ColorVal color, Vector3Par lightPos, bool secondary)
{
    Color lightColor = color * GEngine->GetAccomodateEye();
    float oldA = lightColor.A();
    // we can draw flares now
    // flare positions between posLight (0) and screen center (1)
    static const float flarePos[FlareLast + 1 - Flare0] = {0.0f,    -0.2f,   -0.1f,  +0.25f, +0.275f, +0.3f,
                                                           +0.4f,   +0.5f,   +0.65f, +0.7f,  +0.725f, +0.75f,
                                                           +0.875f, +0.885f, +1.0f,  +1.1f};
    // note - z is ignored when doing 2D draw
    // but is significant for decal draw
    float w = GLOB_ENGINE->Width();
    float h = GLOB_ENGINE->Height();
    // calculate screen position of light
    Vector3Val pos = ScaledInvTransform() * lightPos;
    // apply perspective
    Matrix4Val project = GetCamera()->Projection();
    const float thold = 0.6f;
    if (pos[2] < thold)
    {
        return; // no flares
    }
    float vis = (pos[2] - thold) * (1.0f / (1 - thold));
    saturateMin(vis, 1);
    float a = 0.9f * vis * vis;
    if (a > 0.01)
    {
        float invW = 1 / pos[2];
        Vector3 lightPos(project(0, 2) + project(0, 0) * pos[0] * invW, project(1, 2) + project(1, 1) * pos[1] * invW,
                         1);

        float size = 0.1f;
        float sizeX = +project(0, 0) * size * GetCamera()->InvLeft();
        float sizeY = -project(1, 1) * size * GetCamera()->InvTop();

        Vector3 center(0.5f * w, 0.5f * h, 1);
        const int special = NoZBuf | IsLight | ClampU | ClampV | IsAlphaFog;
        // Flare0 is handled specially
        {
            lightColor.SetA(oldA * a * 2);
            PackedColor color = PackedColor(lightColor);
            Texture* texture = Preloaded(PreloadedTexture(Flare0));
            if (texture)
            {
                MipInfo mip = GLOB_ENGINE->TextBank()->UseMipmap(texture, 0, 0);
                if (mip.IsOK())
                {
                    float sizeCoef = texture->AWidth() * (1.0f / 64);
                    GEngine->DrawDecal(lightPos, 1, sizeX * sizeCoef, sizeY * sizeCoef, color, mip, special);
                }
            }
        }
        if (!secondary)
        {
            return;
        }
        lightColor.SetA(a * oldA);
        PackedColor color = PackedColor(lightColor);
        for (int s = Flare0 + 1; s <= FlareLast; s++)
        {
            float coef = flarePos[s - Flare0];
            Texture* texture = Preloaded(PreloadedTexture(s));
            if (texture)
            {
                MipInfo mip = GLOB_ENGINE->TextBank()->UseMipmap(texture, 0, 0);
                if (mip.IsOK())
                {
                    Vector3 pos = lightPos + coef * (center - lightPos);
                    float sizeCoef = texture->AWidth() * (1.0f / 64);
                    GEngine->DrawDecal(pos, 1, sizeX * sizeCoef, sizeY * sizeCoef, color, mip, special);
                }
            }
        }
    }
}

void Scene::DrawFlares()
{
    if (!GetLandscape())
    {
        return;
    }
    float vis = GetLandscape()->SkyThrough();
    float night = MainLight()->NightEffect();

    Vector3Val cPos = GetCamera()->Position();

    if (vis > 0.05 && night < 0.95)
    {
        // draw sun flare
        CameraType camType = GWorld->GetCameraType();

        bool secondary = (camType != CamInternal);
        // check HasFlares of camera source
        Object* camObj = GWorld->CameraOn();
        if (camObj)
        {
            secondary = camObj->HasFlares(camType);
        }
        Vector3 lightDir = MainLight()->SunDirection();
        Color lightColor = MainLight()->SunColor() * (1 - night) * vis;

        // fictive sun position
        Vector3 sunPos = cPos - lightDir * ENGINE_CONFIG.horizontZ;
        float sunRadius = 0.02 * ENGINE_CONFIG.horizontZ;

        float visLand = 1;
        if (ENGINE_CONFIG.enableHWTL)
        {
            float t = GLandscape->IntersectWithGroundOrSea(nullptr, cPos, -lightDir, 0, ENGINE_CONFIG.horizontZ * 1.1);
            visLand = t >= ENGINE_CONFIG.horizontZ;
        }
        if (visLand > 0)
        {
            float a = visLand * 0.25;
            // check against occlusion buffer
            float occ = GetOcclusions()->TestSphereWSpace(sunPos, sunRadius);
            a *= occ;
            if (a >= 0.01)
            {
                // check line against view geometries
                CollisionBuffer col;
                Object* camOn = GWorld->CameraOn();
                if (camOn && !GWorld->GetCameraEffect())
                {
                    GLandscape->ObjectCollision(col, camOn, nullptr, cPos, sunPos, 0, ObjIntersectView);
                    // check if any of the objects is not considered
                    for (int i = 0; i < col.Size(); i++)
                    {
                        Object* obj = col[i].object;
                        if (obj && obj->GetShape() && !obj->GetShape()->CanOcclude())
                        {
                            // object is not included in occlusion buffer, check it now
                            a = 0;
                        }
                    }
                }
                if (a >= 0.01)
                {
                    saturateMin(a, 1);
                    lightColor.SetA(a);
                    DrawFlare(lightColor, GetCamera()->Position() - lightDir, secondary);
                }
            }
        }
    }
    if (night >= 0.2)
    {
        // draw flares from active lights
        Vector3Val camPos = GetCamera()->Position();
        Vector3Val camDir = GetCamera()->Direction();
        for (int i = 0; i < _aLights.Size(); i++)
        {
            Light* light = _aLights[i];
            Vector3 dir = camPos - light->Position();
            if (dir * GetCamera()->Direction() > 0)
            {
                continue; // this one has no flare
            }
            float intensity = light->FlareIntensity(camPos, camDir);
            if (intensity < 0.01)
            {
                continue;
            }
            // light

            // check if light position is visible from camera position
            float visLand = 1;
            if (ENGINE_CONFIG.enableHWTL)
            {
                float t = GLandscape->IntersectWithGroundOrSea(nullptr, cPos, dir, 0, 1.1);
                visLand = t > 1;
            }

            if (visLand > 0)
            {
                Color lightColor = light->GetObjectColor();
                float a = visLand * intensity * 2;
                float occ = GetOcclusions()->TestSphereWSpace(light->Position(), 0.4);
                a *= occ;
                saturateMin(a, 0.5);
                lightColor.SetA(a);
                DrawFlare(lightColor, light->Position(), false);
            }
        }
    }
}

void Scene::DrawRainLevel(float alpha, float yDensity, float xOffset, float yOffset, float z)
{
    Texture* texture = Preloaded(TextureRain);
    Color color(HWhite);
    color.SetA(alpha);
    Draw2DPars pars;
    pars.mip = GLOB_ENGINE->TextBank()->UseMipmap(texture, 0, 0);
    pars.SetU(xOffset - z * 0.5f, xOffset + z * 0.5f);
    pars.SetV(yOffset - z * 0.5f * yDensity, yOffset + z * 0.5f * yDensity);
    pars.SetColor(PackedColor(color));
    pars.spec = NoZWrite | IsAlpha | NoClamp | IsAlphaFog;
    Rect2DAbs rect(0, 0, GLOB_ENGINE->Width(), GLOB_ENGINE->Height());
    GLOB_ENGINE->Draw2D(pars, rect);
}

void Scene::DrawRain()
{
    // start with single level rain
    _rainLate.pending = false;
    if (!GetLandscape())
    {
        return;
    }
    float density = GetLandscape()->GetRainDensity();
    if (density >= 0.1)
    {
        Vector3Val dir = GetCamera()->Direction();
        float speed = fabs(GetCamera()->Speed() * dir) * 0.05f;
        saturate(speed, 0, 2);
        static float rainOffset;
        static Time rainT;
        float deltaRT = Glob.time - rainT;
        rainT += deltaRT;
        rainOffset += deltaRT;
        float yOffset = -fastFmod(rainOffset, 1);
        // P0-6 deterministic mode (triDeterministic): the phase from the sim
        // clock, not the float sum of every frame's step since the first rain.
        if (remaster::TemporalDeterministic())
            yOffset = -remaster::RainStreakPhase(Glob.time.toInt());
        float xOffset = atan2(dir.X(), dir.Z()) * 0.3f;
        float yDensity = 0.3f + fabs(dir.Y()) + speed;
        saturate(yDensity, 0.1f, 1);
        // draw all levels
        density *= 0.2f;
        // Under the photoreal camera (photoLight) the overlay's streaks are
        // stretched by the exposure and the curve: at full strength they
        // smeared the whole sky with vertical lines. (0.9, not 0.45: under the
        // grey day's exposure and drawn after the opaque copy, at 0.45 no
        // streak read.)
        if (GEngine && GEngine->LightingRemasterActive())
        {
            const remaster::LightingTuning tuning = GEngine->GetLightingTuning();
            if (tuning.features.photoLight > 0.5f)
                density *= 0.9f;
        }
        // Rain you can see (POSEIDON_XP_RAIN, world by default): world-space
        // streaks in place of the overlay, or (=late) the overlay held back
        // until after the TAA resolve. A cockpit draws over the world after
        // the rain, so a frame with one keeps the overlay here, under it.
        const remaster::RainXpMode xpRain = remaster::RainXpModeFromEnv();
        if (xpRain != remaster::RainXpMode::Off && GEngine && !CockpitThisFrame())
        {
            if (xpRain == remaster::RainXpMode::Late)
            {
                _rainLate.pending = true;
                _rainLate.alpha = density;
                _rainLate.yDensity = yDensity;
                _rainLate.xOffset = xOffset;
                _rainLate.yOffset = yOffset;
                return;
            }
            // The drops' travel is summed from frame to frame (RainDriftStep:
            // velocity times the clock made the rain stand or rise whenever
            // the density or the wind changed). A capture's temporal reset
            // starts it again, so a pinned shot repeats.
            static remaster::RainXpDrift drift;
            static std::uint32_t driftReset = 0;
            if (const std::uint32_t serial = remaster::TemporalControl().resetSerial; serial != driftReset)
            {
                driftReset = serial;
                drift = remaster::RainXpDrift();
            }
            remaster::RainXpFrame rain;
            rain.frame = GEngine->GetFrameCounter();
            const Vector3 windV = GetLandscape()->GetWind();
            const float wind[3] = {windV.X(), windV.Y(), windV.Z()};
            Vector3Val pos = GetCamera()->Position();
            const double camPos[3] = {pos.X(), pos.Y(), pos.Z()};
            const double now = 1e-3 * static_cast<double>(Glob.time.toInt());
            const float rainDensity = GetLandscape()->GetRainDensity();
            remaster::RainDriftStep(drift, now, rainDensity, wind, camPos, rain);
            if (remaster::RainXpProbeFromEnv())
            {
                LOG_INFO(Graphics,
                         "xp rain probe: frame {} t {:.3f} density {:.5f} fall {:.3f} wind {:.2f},{:.2f} cam {:.2f},"
                         "{:.2f},{:.2f} | round1 drops move {:.2f},{:.2f},{:.2f} m/s | fixed drops move {:.2f},{:.2f},"
                         "{:.2f} m/s (rel to camera {:.2f},{:.2f},{:.2f})",
                         rain.frame, now, rainDensity, remaster::RainFallSpeed(rainDensity), wind[0], wind[2],
                         drift.camVel[0], drift.camVel[1], drift.camVel[2], drift.r1Speed[0], drift.r1Speed[1],
                         drift.r1Speed[2], rain.relVel[1][0] + drift.camVel[0], rain.relVel[1][1] + drift.camVel[1],
                         rain.relVel[1][2] + drift.camVel[2], rain.relVel[1][0], rain.relVel[1][1], rain.relVel[1][2]);
            }
            if (remaster::RainXpRound1FromEnv())
                remaster::RainDriftAsRound1(drift, rain);
            // Handed over only when the streaks will draw: a frame that is not
            // ready keeps the overlay alone, never both.
            if (GEngine->RainXpWorldReady())
            {
                GEngine->SetRainXpFrame(rain);
                return;
            }
        }
        DrawRainLevel(density, yDensity, xOffset, yOffset, 8);
        DrawRainLevel(density, yDensity, xOffset, yOffset, 2);
    }
}

void Scene::DrawRainLate()
{
    if (!_rainLate.pending)
        return;
    _rainLate.pending = false;
    DrawRainLevel(_rainLate.alpha, _rainLate.yDensity, _rainLate.xOffset, _rainLate.yOffset, 8);
    DrawRainLevel(_rainLate.alpha, _rainLate.yDensity, _rainLate.xOffset, _rainLate.yOffset, 2);
}

bool Scene::CockpitThisFrame() const
{
    for (int i = 0; i < _drawMergers.Size(); i++)
    {
        const SortObject* oi = _drawMergers[i];
        if (oi->passNum == 3 && oi->drawLOD != LOD_INVISIBLE && !oi->hillHidden)
            return true;
    }
    return false;
}

void Scene::ObjectsDrawn()
{
    // release all references
    DrawRain();

    DrawObjectsAndShadowsPass3();

    // clear working list
    _drawMergers.Resize(0);
    // keep drawObjects for next frame

    DrawFlares();
    _aLights.Resize(0);
}

int Scene::LevelFromDistance2(LODShape* shape, float distance2, float oScale, Vector3Par direction,
                              Vector3Par viewDirection)
{
    // if pixel size is lower than 2, object can be considered invisible
    // size in pixels is
    float scale = GetCamera()->Left();
    float diameter = shape->BoundingSphere() * 2 * oScale;

    if (distance2 > Square(ENGINE_CONFIG.objectsZ))
    {
        return LOD_INVISIBLE;
    }

    float pixelLimit = 0.125;
    float detail2 = distance2 * Square(_lodInvWidth);

    if (Square(diameter) < Square(pixelLimit * scale) * detail2)
    {
        return LOD_INVISIBLE;
    }

    if (shape->NLevels() < 2)
    {
        return 0;
    }

    float resol2 = detail2 * Square(scale);
    // disable decal LODs
    int level = shape->FindSqrtLevel(resol2, true);

    return level;
}

// LevelFromDistance2 for an object that drew `held` last frame (LodPop.hpp).
int Scene::LevelHeldFromDistance2(LODShape* shape, float distance2, float oScale, int held, float band)
{
    float scale = GetCamera()->Left();
    float diameter = shape->BoundingSphere() * 2 * oScale;

    if (distance2 > Square(ENGINE_CONFIG.objectsZ))
    {
        return LOD_INVISIBLE;
    }

    const float pixelLimit = held == LOD_INVISIBLE && band > 1.0f ? 0.125f * LodPop::kCullLeaveFactor : 0.125f;
    float detail2 = distance2 * Square(_lodInvWidth);

    if (Square(diameter) < Square(pixelLimit * scale) * detail2)
    {
        return LOD_INVISIBLE;
    }

    if (shape->NLevels() < 2)
    {
        return 0;
    }

    float resol2 = detail2 * Square(scale);
    auto find = [shape](float r2) { return shape->FindSqrtLevel(r2, true); };
    if (held < 0 || held >= shape->NLevels())
    {
        return find(resol2);
    }
    return LodPop::HeldLevel(held, resol2, band, find);
}

float Scene::LodResolution2(float distance2) const
{
    return distance2 * Square(_lodInvWidth) * Square(GetCamera()->Left());
}

int Scene::LevelShadowFromDistance2(LODShape* shape, float distance2, float oScale, Vector3Par direction,
                                    Vector3Par viewDirection)
{
    distance2 *= 8;

    float scale = GetCamera()->Left();
    float diameter = shape->BoundingSphere() * 2 * oScale;

    float pixelLimit = 0.25;
    float detail2 = distance2 * Square(_lodInvWidth);

    if (Square(diameter) < Square(pixelLimit * scale) * detail2)
    {
        return LOD_INVISIBLE;
    }

    if (shape->NLevels() < 2)
    {
        return 0;
    }
    float limit = ENGINE_CONFIG.shadowLODLimit;
    saturateMax(detail2, Square(diameter * limit));
    float resol2 = detail2 * Square(scale);
    // disable decal LODs
    int level = shape->FindSqrtLevel(resol2, true);

    return level;
}

#define DO_STAT 0

#if DO_STAT

#include <Poseidon/Foundation/Math/Statistics.hpp>

NameStatistics Alpha;
NameStatistics Opaque;
NameStatistics Shadow;
#endif

#if _ENABLE_CHEATS

// advances diagnostics - via scripting
#include <Evaluator/express.hpp>

#define NOTHING GameValue()

#define DIAG_DRAW_MODE_ENUM(type, prefix, XX) \
    XX(type, prefix, Normal)                  \
    XX(type, prefix, Roadway)                 \
    XX(type, prefix, Geometry)                \
    XX(type, prefix, ViewGeometry)            \
    XX(type, prefix, FireGeometry)            \
    XX(type, prefix, Paths)

DECLARE_DEFINE_ENUM(DiagDrawMode, DDM, DIAG_DRAW_MODE_ENUM)

namespace Poseidon::Dev
{
DEFINE_ENUM(DiagEnable, DE, DIAG_ENABLE_ENUM)
}

DiagDrawMode DiagDrawModeState = DDMNormal;
namespace Poseidon::Dev
{
int DiagMode;
}

static GameValue SetDiagDrawMode(const GameState* state, GameValuePar oper)
{
    const char* modeStr = (RString)oper;
    DiagDrawMode mode = GetEnumValue<DiagDrawMode>(modeStr);
    if ((int)mode == -1)
        return NOTHING;
    DiagDrawModeState = mode;
    return NOTHING;
}

static GameValue SetDiagEnable(const GameState* state, GameValuePar oper1, GameValuePar oper2)
{
    const char* modeStr = (RString)oper1;
    bool onOff = oper2;
    int modeMask = ~0;
    if (strcmpi(modeStr, "all"))
    {
        DiagEnable mode = GetEnumValue<DiagEnable>(modeStr);
        if ((int)mode == -1)
            return NOTHING;
        modeMask = 1 << mode;
    }

    if (onOff)
        DiagMode |= modeMask;
    else
        DiagMode &= ~modeMask;
    return NOTHING;
}

static GameValue SetDiagToggle(const GameState* state, GameValuePar oper1)
{
    const char* modeStr = (RString)oper1;
    DiagEnable mode = GetEnumValue<DiagEnable>(modeStr);
    if ((int)mode == -1)
        return NOTHING;
    int modeMask = 1 << mode;

    DiagMode ^= modeMask;
    return NOTHING;
}

#include <Poseidon/Foundation/Modules/Modules.hpp>

static const GameFunction ObjUnary[] = {
    GameFunction(GameNothing, "diag_drawmode", SetDiagDrawMode, GameString),
    GameFunction(GameNothing, "diag_toggle", SetDiagToggle, GameString),
};
static const GameOperator ObjBinary[] = {
    GameOperator(GameNothing, "diag_enable", function, SetDiagEnable, GameString, GameBool),
};

INIT_MODULE(GameStateObj, 3)
{
    GGameState.NewOperators(ObjBinary, sizeof(ObjBinary) / sizeof(*ObjBinary));
    GGameState.NewFunctions(ObjUnary, sizeof(ObjUnary) / sizeof(*ObjUnary));
};

#endif

static const float s_lodBand = LodPop::EnvPercentBand("POSEIDON_XP_LODHYST");
static const float s_lodFade = LodPop::EnvMilliseconds("POSEIDON_XP_LODFADE");
// The band this frame: s_lodBand, or 1 with the camera at rest and fades on.
static float s_lodBandNow = s_lodBand;
// POSEIDON_XP_LODBAND (round 3): the distance band of a level switch, a factor.
static const float s_lodDistBand = LodPop::LodDistBand();

int Scene::AdjustComplexity(SortObjectList& objs)
{
    int totalComplexity = 0;
    for (int i = 0; i < objs.Size(); i++)
    {
        SortObject* oi = objs[i];
        Object* obj = oi->object;
        if (!obj)
        {
            Fail("No obj in SortObject info");
            continue;
        }
        LODShape* shape = oi->shape;
        if (oi->forceDrawLOD >= 0)
        {
            oi->drawLOD = oi->forceDrawLOD;
        }
        else
        {
            int drawLevel =
                s_lodBandNow > 0.0f
                    ? LevelHeldFromDistance2(shape, oi->distance2, obj->Scale(), oi->heldLOD, s_lodBandNow)
                    : LevelFromDistance2(shape, oi->distance2, obj->Scale(), obj->Direction(), _camera->Direction());
            if (drawLevel != LOD_INVISIBLE)
            {
#if _ENABLE_CHEATS
                if (DiagDrawModeState != DDMNormal)
                {
                    int geom = -1;
                    switch (DiagDrawModeState)
                    {
                        case DDMGeometry:
                            geom = shape->FindGeometryLevel();
                            break;
                        case DDMViewGeometry:
                            geom = shape->FindViewGeometryLevel();
                            break;
                        case DDMFireGeometry:
                            geom = shape->FindFireGeometryLevel();
                            break;
                        case DDMRoadway:
                            geom = shape->FindRoadwayLevel();
                            break;
                        case DDMPaths:
                            geom = shape->FindPaths();
                            break;
                    }
                    if (geom >= 0)
                        drawLevel = geom;
                }
#endif
            }
            oi->drawLOD = drawLevel;
        }
        // check number of faces in given level
        if (oi->drawLOD != LOD_INVISIBLE)
        {
            oi->passNum = obj->PassNum(oi->drawLOD);
#if _ENABLE_CHEATS
            if (CHECK_DIAG(DETransparent))
            {
                // all geometries are drawn transparent
                if (oi->drawLOD == shape->FindGeometryLevel())
                {
                    if (oi->passNum < 2)
                        oi->passNum = 2;
                }
            }
#endif
            totalComplexity += obj->GetComplexity(oi->drawLOD, *obj);
        }
    }
    return totalComplexity;
}

static int ShadowFactor(Scene* scene)
{
    float addLightsFactor = scene->MainLight()->NightEffect();
    float skyCoef = floatMin(scene->GetLandscape()->SkyThrough(), 0.6f);
    float shadowFactor = skyCoef + 0.1f;
    return toIntFloor(shadowFactor * (1 - addLightsFactor) * 255);
}

int Scene::AdjustShadowComplexity(SortObjectList& objs)
{
#if 1
    int totalComplexity = 0;

    int shadowFactorI = ShadowFactor(this);

    for (int i = 0; i < objs.Size(); i++)
    {
        SortObject* oi = objs[i];
        Object* obj = oi->object;
        LODShape* shape = oi->shape;
        if (!shape)
        {
            continue;
        }
        if (!(shape->Special() & NoShadow) && oi->distance2 < Square(_shadowFogMaxRange) && shadowFactorI >= 12 &&
            obj->CastShadow())
        {
            int level = LevelShadowFromDistance2(shape, oi->distance2, obj->Scale(), obj->Direction(),
                                                 _mainLight->ShadowDirection());
            if (level != LOD_INVISIBLE)
            {
                level = shape->FindNearestWithoutProperty(level, "lodnoshadow");
                if (level < 0)
                {
                    level = LOD_INVISIBLE;
                }
            }
            oi->shadowLOD = level;
        }
        else
        {
            oi->shadowLOD = LOD_INVISIBLE;
        }
        if (oi->shadowLOD != LOD_INVISIBLE)
        {
            // check number of faces in given level
            Shape* level = shape->Level(oi->shadowLOD);
            totalComplexity += level->NFaces();
        }
    }
    return totalComplexity;
#else

    float addLightsFactor = MainLight()->NightEffect();
    float skyCoef = floatMin(GetLandscape()->SkyThrough(), 0.6f);
    float shadowFactor = skyCoef + 0.1f;
    int shadowFactorI = toIntFloor(shadowFactor * (1 - addLightsFactor) * 255);

    for (int i = 0; i < objs.Size(); i++)
    {
        SortObject* oi = objs[i];
        Object* obj = oi->object;
        LODShape* shape = oi->shape;
        if (!(shape->Special() & NoShadow) && oi->distance2 < Square(_shadowFogMaxRange) && shadowFactorI >= 12)
        {
            int level = 0;
            if (level != LOD_INVISIBLE)
            {
                level = shape->FindNearestWithoutProperty(level, "lodnoshadow");
                if (level < 0)
                    level = LOD_INVISIBLE;
            }
            oi->shadowLOD = level;
        }
        else
        {
            oi->shadowLOD = LOD_INVISIBLE;
        }
    }
    return 1000;
#endif
}

inline void CheckMinMaxIter(Vector3& min, Vector3& max, Vector3Par val)
{
#if __ICL
    if (min[0] > val[0])
        min[0] = val[0];
    if (max[0] < val[0])
        max[0] = val[0];
    if (min[1] > val[1])
        min[1] = val[1];
    if (max[1] < val[1])
        max[1] = val[1];
    if (min[2] > val[2])
        min[2] = val[2];
    if (max[2] < val[2])
        max[2] = val[2];
#else
    if (min[0] > val[0])
    {
        min[0] = val[0];
    }
    else if (max[0] < val[0])
    {
        max[0] = val[0];
    }
    if (min[1] > val[1])
    {
        min[1] = val[1];
    }
    else if (max[1] < val[1])
    {
        max[1] = val[1];
    }
    if (min[2] > val[2])
    {
        min[2] = val[2];
    }
    else if (max[2] < val[2])
    {
        max[2] = val[2];
    }
#endif
}

static bool FarEnoughForOcclusion(const SortObject* oi)
{
    // do not occlude things that are very near
    // the test would be very slow and is very like to fail
    const float occNearest = 20;
    if (oi->distance2 > Square(occNearest + 50))
    {
        return true;
    }
    else
    {
        float distNear = oi->distance2 * InvSqrt(oi->distance2) - oi->radius;
        if (distNear > occNearest)
        {
            return true;
        }
    }
    return false;
}

// W3-1(b): the view geometry an object renders into the occlusion buffer when it
// is not hidden, or null when it renders nothing (not an occluder, no view
// geometry, or too small on screen). Pure: the occlusion pass asks it before the
// test for objects beyond the test range (OcclusionRange.hpp) and after it as
// before for the rest.
static Shape* OccluderViewGeometry(const SortObject* oi, float invLeft)
{
    LODShape* shape = oi->shape;
    if (!shape->CanOcclude() || !oi->object->OcclusionView())
    {
        return nullptr;
    }
    Shape* view = shape->ViewGeometryLevel();
    if (!view)
    {
        return nullptr;
    }
    // test if object is near enough to make a some occlusion
    float areaNom = Square(oi->radius * invLeft);
    float areaDenom = oi->distance2;
    return areaNom > 0.001 * areaDenom ? view : nullptr;
}

namespace
{
// POSEIDON_OCC_STATS=1: one event=occ log line per 256 occlusion passes.
struct OcclusionStatsWindow
{
    double sums[4] = {};
    int passes = 0;
};

void ReportOcclusionStats(const OcclusionRange::Stats& s, float range2)
{
    static const bool s_log = std::getenv("POSEIDON_OCC_STATS") != nullptr;
    if (!s_log)
    {
        return;
    }
    static OcclusionStatsWindow w;
    w.sums[0] += s.tested;
    w.sums[1] += s.culled;
    w.sums[2] += s.skipped;
    w.sums[3] += s.farTested;
    if (++w.passes < 256)
    {
        return;
    }
    const double n = w.passes;
    LOG_INFO(Graphics, "event=occ passes={} range={:.0f} tested={:.1f} culled={:.1f} skipped={:.1f} farTested={:.1f}",
             w.passes, range2 > 0.0f ? std::sqrt(range2) : 0.0f, w.sums[0] / n, w.sums[1] / n, w.sums[2] / n,
             w.sums[3] / n);
    w = OcclusionStatsWindow{};
}
} // namespace

// ---- Hill culling (TerrainOcclusion.hpp, the hillCull feature switch) ----

namespace
{
struct HillCull
{
    TerrainOcclusion::HeightGrid terrain;
    TerrainOcclusion::HorizonTable table;
    TerrainOcclusion::CullGovernor governor;
    // The landscape the terrain bounds were built from.
    const Landscape* land = nullptr;
    int range = 0;
    float grid = 0.0f;
    std::uint64_t sample = 0;
};

HillCull& GHillCull()
{
    static HillCull s_hillCull;
    return s_hillCull;
}

// A strided sample of the heights: notices a new island or an editor
// reshaping the ground without reading the whole grid every frame (the same
// detector TerrainLighting uses for the horizon map).
std::uint64_t SampleTerrain(const Landscape& land, int range, float grid)
{
    std::uint64_t h = 1469598103934665603ull;
    auto mix = [&h](std::uint32_t v)
    {
        h ^= v;
        h *= 1099511628211ull;
    };
    mix(static_cast<std::uint32_t>(range));
    std::uint32_t bits = 0;
    std::memcpy(&bits, &grid, sizeof(bits));
    mix(bits);
    const int step = range > 64 ? range / 64 : 1;
    for (int z = 0; z < range; z += step)
    {
        for (int x = (z / step) % step; x < range; x += step)
        {
            const float v = land.ClippedData(z, x);
            std::memcpy(&bits, &v, sizeof(bits));
            mix(bits);
        }
    }
    return h;
}

// The terrain as drawn, copied from `land` when the landscape changed. It is
// the full-detail grid: LandCache asks for segment LOD 0 (Landscape.cpp), so
// every segment draws every grid vertex. If distant segments ever draw
// coarser, their triangles no longer follow this grid and the hill cull must
// take the lowest vertex of each coarse cell instead.
bool EnsureHillTerrain(HillCull& hc, const Landscape& land)
{
    const int range = land.GetTerrainRange();
    const float grid = land.GetTerrainGrid();
    if (range < 2 || !(grid > 0.0f))
        return false;
    const std::uint64_t sample = SampleTerrain(land, range, grid);
    if (hc.terrain.Valid() && hc.land == &land && hc.range == range && hc.grid == grid && hc.sample == sample)
        return true;
    const auto t0 = std::chrono::steady_clock::now();
    hc.terrain.Build(range, grid, YOutsideMap, [&land](int x, int z) { return land.ClippedData(z, x); });
    hc.table.Invalidate();
    hc.governor.Reset();
    hc.land = &land;
    hc.range = range;
    hc.grid = grid;
    hc.sample = sample;
    LOG_INFO(Graphics, "Hill cull: terrain grid {}x{} at {} m copied in {:.1f} ms", range, range, grid,
             std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count());
    return hc.terrain.Valid();
}
} // namespace

#if !DENSITY_LOD

void Scene::AdjustComplexity()
{
    if (_lodPinFactor > 0.0f)
    {
        // Pinned (PinObjectDetail): the detail no longer follows the frame
        // time, so A/B captures of one view draw the same objects.
        _lodInvWidth = _minLodInvWidth * _lodPinFactor;
        saturate(_lodInvWidth, _minLodInvWidth, _maxLodInvWidth);
        AdjustComplexity(_drawObjects);
        AdjustShadowComplexity(_drawObjects);
        return;
    }
    float oldLodInvWidth = _lodInvWidth;

    // adjust lodInvWidth so we are on the line given by points
    // (MinTargetFrameDuration, minLodInvWidth), (MaxTargetFrameDuration, maxLodInvWidth):
    // l is _lodInvWidth, t is estimated time. Line equation lineL*l+lineT*t+lineC = 0.
    float lMin = _minLodInvWidth;
    float lMax = _maxLodInvWidth;

    float tMin = _minTargetFrameDuration;
    float tMax = _maxTargetFrameDuration;

    float lineL = tMin - tMax;
    float lineT = lMax - lMin;
    // normalize lineL +  lineT
    float invLineTLSize = InvSqrt(lineL * lineL + lineT * lineT);
    lineL *= invLineTLSize;
    lineT *= invLineTLSize;

    float lineC = -lineL * lMin - lineT * tMin;
#define DIAG_LOD 0
#if DIAG_LOD
    LOG_DEBUG(Graphics, "t range {:.1f}..{:.1f}", tMin, tMax);
    LOG_DEBUG(Graphics, "l range {:.3f}..{:.3f}", lMin, lMax);
#endif

    int targetGeom = ENGINE_CONFIG.maxObjects * 75;
    int complexity = 0;
    int treshold = 8;
    for (int iters = 5; --iters >= 0;)
    {
        // calculate complexity
        complexity = AdjustComplexity(_drawObjects);
        complexity += AdjustShadowComplexity(_drawObjects);
        // include landscape complexity
        complexity += toLargeInt(Square(ENGINE_CONFIG.horizontZ * (1.0f / 50)));

        float frameDuration = GEngine->GetAvgFrameDuration(4);

        // estimate duration of this frame
        float avgComplexity = 0;
        for (int i = 0; i < NStoreComplexities; i++)
        {
            avgComplexity += _lastComplexity[i];
        }
        avgComplexity *= 1.0f / NStoreComplexities;
        int estDuration = 200; // no estimate yet
        if (avgComplexity > 0)
        {
            estDuration = toLargeInt(frameDuration * complexity / avgComplexity);
        }
        // estimate some minimal complexity this computer is able to render
        float triCoef = 200; // good estimation for Voodoo2
        int maxDuration = toLargeInt(complexity / float(targetGeom) * triCoef);

        saturateMin(estDuration, maxDuration);

        // react to quick change in complexity
        // estimate frame rate based on previous complexity and time

        // calculate l and t
        // l is _lodInvWidth, t is estimated time
        // line normal is (lineL,lineT)
        float l = _lodInvWidth;
        float t = estDuration;
        float dist = lineL * l + lineT * t + lineC;
// check distance from line
// note: we might be in area out of tmin,tmax
// find nearest point on line
#if DIAG_LOD
        LOG_DEBUG(Graphics, "  t {:.1f}, l {:.3f}, dist {:.2f}", t, l, dist);
#endif
        float tDiff = dist * 200;

        if (tDiff > treshold)
        {
            if (_lodInvWidth >= _maxLodInvWidth)
            {
                break; // fast path - min reached
            }
            treshold = 4; // allow smoother change
            // scale - we are too slow
            float change = tDiff * -0.005f; // change is negative
            saturate(change, -0.2f, +0.2f);
            _lodInvWidth /= Square(1 + change);
            if (_lodInvWidth > _maxLodInvWidth)
            {
                // extreme reached - limit
                _lodInvWidth = _maxLodInvWidth;
                complexity = AdjustComplexity(_drawObjects);
                complexity += AdjustShadowComplexity(_drawObjects);
                complexity += toLargeInt(Square(ENGINE_CONFIG.horizontZ * (1.0f / 50)));
                break;
            }
        }
        else if (tDiff < -treshold)
        {
            if (_lodInvWidth <= _minLodInvWidth)
            {
                break; // fast path - max reached
            }
            treshold = 4; // allow smoother change
            // scale - we are too fast
            float change = tDiff * -0.005f; // change is possitive
            saturate(change, -0.2f, +0.2f);

            _lodInvWidth /= Square(1 + change);

            if (_lodInvWidth < _minLodInvWidth)
            {
                // extreme reached - limit
                _lodInvWidth = _minLodInvWidth;
                complexity = AdjustComplexity(_drawObjects);
                complexity += AdjustShadowComplexity(_drawObjects);
                complexity += toLargeInt(Square(ENGINE_CONFIG.horizontZ * (1.0f / 50)));
                break;
            }
        }
        else
        {
            // wanted state reached - terminate loop
            break;
        }
    }

    if (oldLodInvWidth > _lodInvWidth)
    {
        // going better - check for oscilation
        if (_lastScaleWorseTime > Glob.uiTime - 5)
        {
            // avoid change in this direction - restore state
            _lodInvWidth = oldLodInvWidth;
            complexity = AdjustComplexity(_drawObjects);
            complexity += AdjustShadowComplexity(_drawObjects);
        }
        _lastScaleBetterTime = Glob.uiTime;
    }
    if (oldLodInvWidth < _lodInvWidth)
    {
        // going worse - check for oscilation
        if (_lastScaleBetterTime > Glob.uiTime - 0.5)
        {
            // avoid change in this direction - restore state
            _lodInvWidth = oldLodInvWidth;
            complexity = AdjustComplexity(_drawObjects);
            complexity += AdjustShadowComplexity(_drawObjects);
        }
        _lastScaleWorseTime = Glob.uiTime;
    }

    // store complexity to complexity history
    for (int i = 1; i < NStoreComplexities; i++)
    {
        _lastComplexity[i - 1] = _lastComplexity[i];
    }
    _lastComplexity[NStoreComplexities - 1] = complexity;

    float avgComplexity = 0;
    for (int i = 0; i < NStoreComplexities; i++)
    {
        avgComplexity += _lastComplexity[i];
    }
    avgComplexity *= 1.0f / NStoreComplexities;
}
float Scene::GetSmokeGeneralization() const
{
    return _lodInvWidth * 0.1f;
}

#else

static inline float CoveredArea(Object* obj, float dist2, float oScale)
{
    LODShape* shape = obj->GetShape();
    if (!shape)
        return 0;
    float radius = shape->BoundingSphere() * oScale;

    Camera* cam = GScene->GetCamera();
    // return area in pixels
    float areaK = cam->InvLeft() * cam->InvTop() * GEngine->Width() * GEngine->Height();

    const float maxArea = 0.5f;
    // if (Square(radius)/dist2>maxArea)
    if (Square(radius) > dist2 * maxArea)
    {
        return maxArea * areaK;
    }
    return Square(radius) * areaK / dist2;
}

static int CmpCoveredAreaObj(const SortObjectItem* p1, const SortObjectItem* p0)
{
    // the first object is the nearest one
    const SortObject* o0 = *p0;
    const SortObject* o1 = *p1;
    float dif = (CoveredArea(o0->object, o0->distance2, o0->object->Scale()) -
                 CoveredArea(o1->object, o1->distance2, o1->object->Scale()));
    if (dif < 0)
        return -1;
    if (dif > 0)
        return +1;
    // make sure ordering is stable
    dif = o0->distance2 - o1->distance2;
    if (dif < 0)
        return +1;
    if (dif > 0)
        return -1;
    return CmpShapeObj(p1, p0);
}

static inline int Complexity(LODShape* lShape, int level)
{
    return lShape->Level(level)->NFaces();
}

static int FindLevelWithComplexity(LODShape* lShape, float complexity, float maxDif)
{
    PoseidonAssert(complexity >= 0);
    int bestI = -1;
    float bestDif = maxDif;
    for (int i = 0; i < lShape->NLevels(); i++)
    {
        float resol = lShape->Resolution(i);
        if (resol > 900)
            break;
        int lComplex = Complexity(lShape, i);
        float dif = fabs(complexity - lComplex);
        if (bestDif > dif)
        {
            bestDif = dif;
            bestI = i;
        }
    }
    return bestI;
}

static int FindShadowLevelWithComplexity(LODShape* lShape, float complexity, float maxDif)
{
    int i = FindLevelWithComplexity(lShape, complexity, maxDif);
    if (i < 0)
        return i;
    return lShape->FindNearestWithoutProperty(i, "lodnoshadow");
}

void Scene::AdjustComplexity()
{
#if _ENABLE_CHEATS
    static int displayComplexityLimit = 100000;
#endif

    // set _lodInvWidth in case any needs it
    // development only: it should not be used when Density Lod system is active
    _lodInvWidth = (_minLodInvWidth + _maxLodInvWidth) * 0.5f;

    int shadowFactorI = ShadowFactor(this);

#if _ENABLE_CHEATS
    static float maxDensity = 0.3; // max. 5 polygons per pixel wanted
    static float shadowAreaCoef = 1.0f / 32;
    // static float invShadowAreaCoef = 1/shadowAreaCoef;
    auto& input = InputSubsystem::Instance();
    if (input.GetCheat2ToDo(SDL_SCANCODE_LEFTBRACKET))
    {
        maxDensity /= 1.2f;
    }
    if (input.GetCheat2ToDo(SDL_SCANCODE_RIGHTBRACKET))
    {
        maxDensity *= 1.2f;
    }

    if (input.GetCheat1ToDo(SDL_SCANCODE_LEFTBRACKET))
    {
        shadowAreaCoef /= 1.5f;
        // invShadowAreaCoef = 1/shadowAreaCoef;
    }
    if (input.GetCheat1ToDo(SDL_SCANCODE_RIGHTBRACKET))
    {
        shadowAreaCoef *= 1.5f;
        // invShadowAreaCoef = 1/shadowAreaCoef;
    }
#else
    const float maxDensity = 0.3; // max. 5 polygons per pixel wanted
    const float shadowAreaCoef = 1.0f / 32;
// const float invShadowAreaCoef = 1/shadowAreaCoef;
#endif

    // calculate total covered area
    float totalArea = 0;
    // sum complexity of objects that are excluded from lod management
    int complexityUsed = 0;
    for (int i = 0; i < _drawObjects.Size(); i++)
    {
        SortObject* oi = _drawObjects[i];
        Object* obj = oi->object;
        if (oi->forceDrawLOD >= 0)
        {
            // count complexity as used
            if (oi->forceDrawLOD != LOD_INVISIBLE)
            {
                complexityUsed += obj->GetComplexity(oi->forceDrawLOD, *obj);
            }
        }
        else
        {
            float area = CoveredArea(obj, oi->distance2, obj->Scale());
            totalArea += area;
        }

        LODShape* shape = obj->GetShape();
        if (shape && !(shape->Special() & NoShadow) && oi->distance2 < Square(_shadowFogMaxRange) &&
            shadowFactorI >= 12 && obj->CastShadow())
        {
            float area = CoveredArea(obj, oi->distance2, obj->Scale());
            totalArea += area * shadowAreaCoef;
        }
        else
        {
            oi->shadowLOD = LOD_INVISIBLE;
        }
    }

    // if total area is zero, there are no controlled visible objects and density does not matter
    // we have total area, we know how much complexity we want - wa may calculate density now
    const int wantedComplexity = 30000;
    complexityUsed += toLargeInt(Square(ENGINE_CONFIG.horizontZ * (1.0f / 50)));

    int complexity = complexityUsed;

    float density = (wantedComplexity - complexity) / floatMax(totalArea, 1e-20);
    saturate(density, 0, maxDensity);

    float density0 = density;
    float totalArea0 = totalArea;
    // note: if too many polygons were already used, density may be negative

    float minArea = totalArea * 1e-4f;
    // first pass: select object which use lod 0
    // such objects often do not use available complexity
    for (int i = 0; i < _drawObjects.Size(); i++)
    {
        SortObject* oi = _drawObjects[i];
        Object* obj = oi->object;
        if (oi->forceDrawLOD < 0)
        {
            LODShape* lShape = obj->GetShape();
            float area = CoveredArea(obj, oi->distance2, obj->Scale());
            float oComplexity = area * density;
            int level = FindLevelWithComplexity(lShape, oComplexity, oComplexity * 1.5f + 200);
            if (level == 0)
            {
                int levelComplexity = Complexity(lShape, level);
#if _ENABLE_CHEATS
                if (abs(oComplexity - levelComplexity) > displayComplexityLimit)
                {
                    LOG_DEBUG(Graphics, "{} - complexity {}, wanted {:.0f}", (const char*)lShape->GetName(),
                              levelComplexity, oComplexity);
                }
#endif
                complexity += levelComplexity;
                oi->passNum = obj->PassNum(level);
                oi->drawLOD = level;

                totalArea -= area;
            }
        }
        else
        {
            if (oi->forceDrawLOD != LOD_INVISIBLE)
            {
                oi->passNum = obj->PassNum(oi->forceDrawLOD);
            }
            oi->drawLOD = oi->forceDrawLOD;
        }
        // similiar for shadows
        if (oi->shadowLOD < 0)
        {
            LODShape* lShape = obj->GetShape();
            float area = CoveredArea(obj, oi->distance2, obj->Scale());
            float oComplexity = area * density * shadowAreaCoef;
            int level = FindShadowLevelWithComplexity(lShape, oComplexity, oComplexity * 1.5f + 200);
            if (level == 0) // lShape->_minShadow should be used here instead
            {
                int levelComplexity = Complexity(lShape, level);
                complexity += levelComplexity;
                oi->shadowLOD = level;

                totalArea -= area * shadowAreaCoef;
            }
        }
    }

    // when we rendered almost everything, there is no need to update density any more
    if (totalArea > minArea)
    {
        density = (wantedComplexity - complexity) / floatMax(totalArea, 1e-20);
        saturate(density, 0, maxDensity);
    }

    // selects lods for all other objects
    for (int i = 0; i < _drawObjects.Size(); i++)
    {
        SortObject* oi = _drawObjects[i];
        Object* obj = oi->object;
        if (oi->drawLOD < 0)
        {
            LODShape* lShape = obj->GetShape();
            float area = CoveredArea(obj, oi->distance2, obj->Scale());
            float oComplexity = area * density;
            int level = FindLevelWithComplexity(lShape, oComplexity, oComplexity * 1.5f + 200);
            if (level < 0)
            {
                oi->drawLOD = LOD_INVISIBLE;
            }
            else
            {
                int levelComplexity = Complexity(lShape, level);
#if _ENABLE_CHEATS
                if (abs(oComplexity - levelComplexity) > displayComplexityLimit)
                {
                    LOG_DEBUG(Graphics, "{} - complexity {}, wanted {:.0f}", (const char*)lShape->GetName(),
                              levelComplexity, oComplexity);
                }
#endif
                complexity += levelComplexity;
                oi->passNum = obj->PassNum(level);
                oi->drawLOD = level;
            }
        }
        if (oi->shadowLOD < 0)
        {
            LODShape* lShape = obj->GetShape();
            float area = CoveredArea(obj, oi->distance2, obj->Scale());
            float oComplexity = area * density * shadowAreaCoef;
            int level = FindShadowLevelWithComplexity(lShape, oComplexity, oComplexity * 1.5f + 200);
            if (level < 0)
            {
                oi->shadowLOD = LOD_INVISIBLE;
            }
            else
            {
                int levelComplexity = Complexity(lShape, level);
#if _ENABLE_CHEATS
                if (abs(oComplexity - levelComplexity) > displayComplexityLimit)
                {
                    LOG_DEBUG(Graphics, "{} - complexity {}, wanted {:.0f}", (const char*)lShape->GetName(),
                              levelComplexity, oComplexity);
                }
#endif
                complexity += levelComplexity;
                oi->shadowLOD = level;
            }
        }
    }

    // store complexity to complexity history
    for (int i = 1; i < NStoreComplexities; i++)
    {
        _lastComplexity[i - 1] = _lastComplexity[i];
    }
    _lastComplexity[NStoreComplexities - 1] = complexity;

    float avgComplexity = 0;
    for (int i = 0; i < NStoreComplexities; i++)
    {
        avgComplexity += _lastComplexity[i];
    }
    avgComplexity *= 1.0f / NStoreComplexities;
#if 1
    GlobalShowMessage(
        500,
        "Complex %8d, ~ %8.0f, density %0.5f (%0.5f) < %0.5f, totArea %0.f (%0.f), check %0.1f, cUsed %d, shadC %.2f",
        complexity, avgComplexity, density, density0, maxDensity, totalArea, totalArea0, density0 * totalArea0,
        complexityUsed, 1 / shadowAreaCoef);
#endif
}
float Scene::GetSmokeGeneralization() const
{
    return (_minLodInvWidth + _maxLodInvWidth) * 0.5f * 0.1f;
}
#endif

// Cockpit pass routing — the one policy site.  The camera vehicle is
// queued at its inside-view LOD (World::Draw); only that queue ever
// selects an object's InsideLOD level, so equality here means "this is
// the first-person interior" — soldier hands/body, vehicle cockpit.
// Those draws carry PassKindHint::Cockpit so the descriptor build picks
// the Cockpit* pass family explicitly instead of inferring it from the
// shape's NoDropdown faces.
// POSEIDON_XP_LODBAND: the objects in a band this pass, and their draw by
// sample mask: for each split k of the samples, every object on that split
// draws its own level on the first k samples, then its other level on the
// rest. 2 x (samples - 1) mask changes a pass, whatever the object count.
static std::vector<SortObject*> s_bandDeferred;

static void DrawBandDeferred(const std::vector<SortObject*>& objs)
{
    const int samples = GEngine->GetMsaaSamples();
    const unsigned all = (1u << samples) - 1u;
    GSectionFilter = SectionClassFilter::OpaqueAndCutout;
    for (int k = 1; k < samples; k++)
    {
        const unsigned now = (1u << k) - 1u;
        bool any = false;
        for (SortObject* oi : objs)
        {
            if (oi->fadeOut || LodPop::FadeSampleMasks(oi->fadeT, samples).now != now)
            {
                continue;
            }
            if (!any)
            {
                GEngine->SetDrawSampleMask(now);
                any = true;
            }
            oi->object->Draw(oi->drawLOD, oi->orClip, *oi->object);
        }
        any = false;
        for (SortObject* oi : objs)
        {
            if (oi->fadeFrom == LOD_INVISIBLE || LodPop::FadeSampleMasks(oi->fadeT, samples).now != now)
            {
                continue;
            }
            if (!any)
            {
                GEngine->SetDrawSampleMask(all & ~now);
                any = true;
            }
            oi->object->Draw(oi->fadeFrom, oi->orClip, *oi->object);
        }
    }
    GEngine->SetDrawSampleMask(0);
    GSectionFilter = SectionClassFilter::All;
}

static void DrawSortObject(SortObject* oi)
{
    Object* obj = oi->object;
    if (oi->fadeFrom != LodPop::kNoLevel)
    {
        // Static objects only (UpdateLodFades), so never the cockpit below.
        const LodPop::FadeMasks masks = LodPop::FadeSampleMasks(oi->fadeT, GEngine->GetMsaaSamples());
        if (!oi->fadeOut)
        {
            GEngine->SetDrawSampleMask(masks.now);
            obj->Draw(oi->drawLOD, oi->orClip, *obj);
        }
        if (oi->fadeFrom != LOD_INVISIBLE)
        {
            GEngine->SetDrawSampleMask(masks.was);
            obj->Draw(oi->fadeFrom, oi->orClip, *obj);
        }
        GEngine->SetDrawSampleMask(0);
        return;
    }
    const bool cockpit =
        GWorld && oi->drawLOD != LOD_INVISIBLE && oi->drawLOD == obj->InsideLOD(GWorld->GetCameraType());
    if (!cockpit)
    {
        obj->Draw(oi->drawLOD, oi->orClip, *obj);
        return;
    }
    const render::PassKindHint savedHint = GEngine->GetPassKindHint();
    GEngine->SetPassKindHint(render::PassKindHint::Cockpit);
    obj->Draw(oi->drawLOD, oi->orClip, *obj);
    GEngine->SetPassKindHint(savedHint);
}

// The instanced-run facts of one queued object (InstancedRunRules.hpp).
// `headLevel` is the head's draw level: an object joins a run only with the
// head's shape and LOD, so that level is its own whenever the answer matters.
// IsAnimated and SelectLights index the object's shape by its draw LOD, which
// for a later object may be LOD_INVISIBLE, so they are asked only for the
// head's shape and LOD. `scene` is set when drawGrouping decides per object
// (some light slot exists): the object's own light list, as its draw would
// select it (Object::Draw), then sets localLit.
static InstancedRun::RunEntry InstancedRunEntry(const SortObject* so, const LODShape* headShape, int headLOD,
                                                const Shape* headLevel, Scene* scene)
{
    const Object* obj = so->object;
    InstancedRun::RunEntry entry;
    entry.shape = obj->GetShape();
    entry.drawLOD = so->drawLOD;
    entry.passNum = so->passNum;
    entry.special = headLevel->Special() | obj->GetObjSpecial();
    entry.distance2 = so->distance2;
    entry.isStatic = obj->Static();
    const bool headLevelToo = obj->GetShape() == headShape && so->drawLOD == headLOD;
    // A fading object draws alone, on its own samples.
    entry.ownVertices = (headLevelToo && obj->IsAnimated(so->drawLOD)) || so->fadeFrom != LodPop::kNoLevel;
    if (scene && headLevelToo)
    {
        LightList work;
        entry.localLit = scene->SelectLights(obj->Transform(), obj, so->drawLOD, work).Size() > 0;
    }
    return entry;
}

// EX2-POP counters (LodPop.hpp): what changed on screen this frame and why,
// read after the occlusion tests. A row per frame while a triCamPath runs.
namespace
{
struct LodPopLog
{
    FILE* rows = nullptr;
    FILE* events = nullptr;
    bool tried = false;
    int segment = 0;
    float lastPathT = -1.0f;
    bool wasActive = false;
    int shIn = 0, shOut = 0, casters = 0; // shadow casters started, stopped, in the maps
};
LodPopLog GLodPopLog;

const char* LodPopPath()
{
    static const char* s_path = std::getenv("POSEIDON_XP_LODLOG");
    return s_path && *s_path ? s_path : nullptr;
}
} // namespace

// Where a world point is on screen: 0..1 from the top left; -9 behind the camera.
struct PopScreen
{
    float x, y;
};

static PopScreen PopScreenOf(const Scene& scene, const Camera& camera, Vector3Par p)
{
    const Vector3 v = scene.ScaledInvTransform() * p;
    if (v[2] < 0.01f || !GEngine)
    {
        return PopScreen{-9.0f, -9.0f};
    }
    Matrix4Val project = camera.Projection();
    const float invW = 1.0f / v[2];
    return PopScreen{(project(0, 2) + project(0, 0) * v[0] * invW) / GEngine->Width(),
                     (project(1, 2) + project(1, 1) * v[1] * invW) / GEngine->Height()};
}

// One line of <path>.events: segment, path time, kind, shape, level before and
// after, distance, size on screen (px), where the object is on screen and where
// the shadow of its middle lands (0..1), the reason, static, draw pass.
static void PopEvent(const Scene& scene, const Camera& camera, const char* kind, const SortObject* oi, int before,
                     int shown, const char* why)
{
    LodPopLog& log = GLodPopLog;
    if (!log.events || !oi->object || !oi->shape)
    {
        return;
    }
    const Object* obj = oi->object;
    const Vector3 pos = obj->Position();
    const float dist = std::sqrt(floatMax(pos.Distance2(camera.Position()), 1.0f));
    const float halfWidth = GEngine ? GEngine->Width() * 0.5f : 960.0f;
    const float px = oi->radius * 2.0f * camera.InvLeft() * halfWidth / dist;
    const PopScreen at = PopScreenOf(scene, camera, pos);
    const Vector3 sun = scene.ShadowKeepSun();
    const PopScreen shadow = PopScreenOf(scene, camera, pos + sun * LodPop::ShadowLength(oi->radius, -sun.Y()));
    fprintf(log.events, "%d,%.3f,%s,%s,%d,%d,%.1f,%.1f,%.4f,%.4f,%.4f,%.4f,%s,%d,%d\n", log.segment, log.lastPathT,
            kind, (const char*)oi->shape->GetName(), before, shown, dist, px, at.x, at.y, shadow.x, shadow.y, why,
            obj->Static() ? 1 : 0, static_cast<int>(oi->passNum));
}

// An object leaves the draw list (it went off screen or out of range) while it
// still cast into the shadow maps: its shadow goes with it.
static void LogListRemoval(const SortObject* oi, const Camera* camera)
{
    if (!camera || !GScene || !tricam::CamPathActive() || oi->smCastPrev < 0 || oi->smCastPrev == LOD_INVISIBLE)
    {
        return;
    }
    PopEvent(*GScene, *camera, "shGone", oi, oi->smCastPrev, LOD_INVISIBLE, "list");
    ++GLodPopLog.shOut;
}

static void RecordLodPop(const Scene& scene, SortObjectList& objs, const Camera& camera, float lodInvWidth,
                         perf::FrameCounters& counters)
{
    LodPopLog& log = GLodPopLog;
    if (!log.tried)
    {
        log.tried = true;
        const std::string path = LodPopPath();
        log.rows = fopen(path.c_str(), "ab");
        log.events = fopen((path + ".events").c_str(), "ab");
        if (log.rows)
        {
            fputs("seg,t,list,drawn,lod,flip,cullOut,cullIn,occOut,occIn,hillOut,hillIn,entered,lodPx,flipPx,occPx,"
                  "lodBig,occBig,lodw,camx,camy,camz,fades,hard,farOut,farIn,offOut,offIn,shIn,shOut,casters\n",
                  log.rows);
        }
    }
    const bool active = tricam::CamPathActive();
    const float pathT = active ? tricam::CamPathTime() : 0.0f;
    if (active && (!log.wasActive || pathT < log.lastPathT))
    {
        ++log.segment;
    }
    log.wasActive = active;
    log.lastPathT = pathT;
    // The first frames of a path are the cut to its start, not pops.
    const bool logEvents = active && log.events && pathT > 0.1f;

    const float invLeft = camera.InvLeft();
    const float halfWidth = GEngine ? GEngine->Width() * 0.5f : 960.0f;
    const Vector3 eye = camera.Position();
    const float objectsZ2 = Square(ENGINE_CONFIG.objectsZ);
    int drawn = 0, lod = 0, flip = 0, cullOut = 0, cullIn = 0, occOut = 0, occIn = 0, hillOut = 0, hillIn = 0,
        entered = 0, lodBig = 0, occBig = 0, fades = 0, hard = 0, farOut = 0, farIn = 0, offOut = 0, offIn = 0;
    double lodPx = 0, flipPx = 0, occPx = 0;
    static const char* const kOut[] = {"", "cullOut", "occOut", "hillOut", "offOut", "farOut"};
    static const char* const kIn[] = {"", "cullIn", "occIn", "hillIn", "offIn", "farIn"};
    for (int i = 0; i < objs.Size(); i++)
    {
        SortObject* oi = objs[i];
        Object* obj = oi->object;
        if (!obj || !oi->shape)
        {
            continue;
        }
        fades += oi->fadeFrom != LodPop::kNoLevel ? 1 : 0;
        const int adjusted = oi->heldLOD;
        const bool hidden = oi->drawLOD == LOD_INVISIBLE || oi->hillHidden;
        const int shown = hidden ? LOD_INVISIBLE : oi->drawLOD;
        // why hidden: 1 too small, 2 behind an object, 3 behind a hill, 4 off
        // screen (kept for its shadow), 5 past the object distance
        const int why = !hidden                             ? 0
                        : oi->forceDrawLOD == LOD_INVISIBLE ? 4
                        : adjusted == LOD_INVISIBLE         ? (oi->distance2 > objectsZ2 ? 5 : 1)
                        : oi->hillHidden                    ? 3
                                                            : 2;
        const int before = oi->popShown;
        if (shown != LOD_INVISIBLE)
        {
            ++drawn;
            perf::CountLod(counters, shown);
        }
        if (oi->popAge < 65535)
        {
            ++oi->popAge;
        }
        if (before == shown)
        {
            oi->popWhy = static_cast<unsigned char>(why);
            continue;
        }
        const float dist = std::sqrt(floatMax(obj->Position().Distance2(eye), 1.0f));
        const float px = oi->radius * 2.0f * invLeft * halfWidth / dist;
        const char* kind = nullptr;
        if (before == LodPop::kNoLevel)
        {
            if (shown != LOD_INVISIBLE)
            {
                ++entered;
                kind = "enter";
            }
        }
        else if (before != LOD_INVISIBLE && shown != LOD_INVISIBLE)
        {
            ++lod;
            lodPx += px;
            lodBig += px >= 8.0f ? 1 : 0;
            hard += oi->fadeFrom == LodPop::kNoLevel ? 1 : 0;
            kind = "lod";
            if (shown == oi->popPrev && oi->popAge <= 60)
            {
                ++flip;
                flipPx += px;
                kind = "flip";
            }
        }
        else if (shown == LOD_INVISIBLE)
        {
            (why == 1 ? cullOut : why == 2 ? occOut : why == 3 ? hillOut : why == 4 ? offOut : farOut) += 1;
            if (why == 2)
            {
                occPx += px;
                occBig += px >= 8.0f ? 1 : 0;
            }
            kind = why == 4 ? nullptr : kOut[why];
        }
        else
        {
            const int was = oi->popWhy;
            (was == 1 ? cullIn : was == 2 ? occIn : was == 3 ? hillIn : was == 4 ? offIn : farIn) += 1;
            if (was == 2)
            {
                occPx += px;
                occBig += px >= 8.0f ? 1 : 0;
            }
            kind = was == 4 || was > 5 ? nullptr : was == 0 ? "in" : kIn[was];
        }
        if (kind && logEvents && px >= 2.0f)
        {
            PopEvent(scene, camera, kind, oi, before, shown, oi->fadeFrom != LodPop::kNoLevel ? "fade" : "");
        }
        oi->popPrev = static_cast<signed char>(before);
        oi->popShown = static_cast<signed char>(shown);
        oi->popWhy = static_cast<unsigned char>(why);
        oi->popAge = 0;
    }
    counters.lodSwitches = lod;
    if (active && log.rows)
    {
        // shIn, shOut and casters are the shadow pass's of the frame before.
        fprintf(log.rows,
                "%d,%.4f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.1f,%.1f,%.1f,%d,%d,%.6f,%.2f,%.2f,%.2f,%d,%d,%d,%d,%d,%d,"
                "%d,%d,%d\n",
                log.segment, pathT, objs.Size(), drawn, lod, flip, cullOut, cullIn, occOut, occIn, hillOut, hillIn,
                entered, lodPx, flipPx, occPx, lodBig, occBig, lodInvWidth, eye.X(), eye.Y(), eye.Z(), fades, hard,
                farOut, farIn, offOut, offIn, log.shIn, log.shOut, log.casters);
        fflush(log.rows);
        if (log.events)
        {
            fflush(log.events);
        }
    }
    log.shIn = log.shOut = 0;
}

// After the shadow-map depth pass: which objects started or stopped casting
// into the maps this frame, and why (LodPop.hpp, POSEIDON_XP_LODLOG).
static void RecordShadowPop(const Scene& scene, SortObjectList& objs, const Camera& camera, bool passRan)
{
    LodPopLog& log = GLodPopLog;
    const bool logEvents = tricam::CamPathActive() && log.events && log.lastPathT > 0.1f;
    remaster::FeatureGates gates;
    const bool treeShadows = GEngine->LightingRemasterActive() && GEngine->GetFeatureGates(gates) &&
                             gates[remaster::Feature::TreeShadows].on;
    const bool keepOn = scene.ShadowKeepOn();
    const float far2 = Square(scene.ShadowMapCasterFar());
    static const char* const kWhy[] = {"", "reach", "offscreen", "small", "occluded", "nolevel", "never", "nopass"};
    int casters = 0;
    for (int i = 0; i < objs.Size(); i++)
    {
        SortObject* oi = objs[i];
        Object* obj = oi->object;
        if (!obj || !oi->shape)
        {
            continue;
        }
        const int now = oi->smCast >= 0 && oi->smCast != LOD_INVISIBLE ? oi->smCast : LOD_INVISIBLE;
        const int prev = oi->smCastPrev;
        int why = 0;
        if (now == LOD_INVISIBLE)
        {
            const bool beyond = keepOn ? !scene.ShadowKeepInReach(obj->Position(), oi->radius) : oi->distance2 > far2;
            why = !passRan ? 7
                  : (oi->shape->Special() & NoShadow) ||
                          !remaster::CastsIntoSunShadowMap(obj->CastShadow(), obj->Static(), treeShadows)
                      ? 6
                  : beyond                             ? 1
                  : oi->forceDrawLOD == LOD_INVISIBLE  ? 2
                  : oi->popWhy == 1 || oi->popWhy == 5 ? 3
                  : oi->popWhy == 2                    ? 4
                                                       : 5;
        }
        else
        {
            ++casters;
        }
        if (prev != now)
        {
            const char* kind = nullptr;
            const char* reason = "";
            if (prev == LodPop::kNoLevel)
            {
                if (now != LOD_INVISIBLE)
                {
                    kind = "shEnter";
                    reason = "list";
                    ++log.shIn;
                }
            }
            else if (prev == LOD_INVISIBLE)
            {
                kind = "shIn";
                reason = kWhy[oi->smWhy < 8 ? oi->smWhy : 0];
                ++log.shIn;
            }
            else if (now == LOD_INVISIBLE)
            {
                kind = "shOut";
                reason = kWhy[why];
                ++log.shOut;
            }
            else
            {
                kind = "shLod";
            }
            if (kind && logEvents)
            {
                PopEvent(scene, camera, kind, oi, prev == LodPop::kNoLevel ? LOD_INVISIBLE : prev, now, reason);
            }
        }
        oi->smCastPrev = static_cast<signed char>(now);
        oi->smCast = LOD_INVISIBLE;
        oi->smWhy = static_cast<unsigned char>(why);
    }
    log.casters = casters;
}

// Starts and steps the LOD cross-fades (LodPop.hpp), between AdjustComplexity
// and the occlusion tests: heldLOD is still last frame's level. Static objects
// only, in the opaque passes; the rest switch as they always did.
static void UpdateLodFades(SortObjectList& objs, float dt, const Object* cameraOn)
{
    const bool msaa = GEngine->GetMsaaSamples() >= 2;
    for (int i = 0; i < objs.Size(); i++)
    {
        SortObject* oi = objs[i];
        Object* obj = oi->object;
        if (!obj || !oi->shape)
        {
            continue;
        }
        if (oi->fadeFrom != LodPop::kNoLevel)
        {
            oi->fadeT += dt / s_lodFade;
            if (oi->fadeT >= 1.0f)
            {
                oi->fadeFrom = LodPop::kNoLevel;
                oi->fadeOut = false;
            }
        }
        const int now = oi->drawLOD;
        const int held = oi->heldLOD;
        oi->heldLOD = static_cast<signed char>(now);
        if (msaa && held != LodPop::kNoLevel && held != now && oi->forceDrawLOD < 0 && obj->Static() &&
            obj != cameraOn && (now == LOD_INVISIBLE || oi->passNum <= 1))
        {
            // Turning back in mid-fade: the same fade, run the other way.
            const bool back = oi->fadeFrom == now && !oi->fadeOut;
            oi->fadeT = back ? 1.0f - oi->fadeT : 0.0f;
            oi->fadeFrom = static_cast<signed char>(held);
            oi->fadeOut = now == LOD_INVISIBLE;
        }
        if (oi->fadeOut && oi->fadeFrom != LodPop::kNoLevel)
        {
            if (now != LOD_INVISIBLE)
            {
                oi->fadeOut = false; // came back: fade in from what it was
            }
            else
            {
                // Going away: it stays on the list, on the level that fades.
                oi->drawLOD = oi->fadeFrom;
                oi->passNum = obj->PassNum(oi->fadeFrom);
            }
        }
    }
}

// POSEIDON_XP_LODBAND (LodPop.hpp): each static object's place in a level
// switch, and near the object distance and the smallest drawn size its share
// of the picture, read from its distance this frame. Both levels draw on
// complementary MSAA samples (DrawSortObject), so the switch is spread over
// metres of approach instead of a fraction of a second. Stateless.
static void UpdateLodBands(Scene& scene, SortObjectList& objs, const Object* cameraOn)
{
    const int samples = GEngine->GetMsaaSamples();
    const float objectsZ = ENGINE_CONFIG.objectsZ;
    const float scale = scene.GetCamera()->Left();
    for (int i = 0; i < objs.Size(); i++)
    {
        SortObject* oi = objs[i];
        oi->heldLOD = oi->drawLOD;
        oi->fadeFrom = LodPop::kNoLevel;
        oi->fadeOut = false;
        Object* obj = oi->object;
        LODShape* shape = oi->shape;
        if (samples < 2 || !obj || !shape || oi->drawLOD == LOD_INVISIBLE || oi->forceDrawLOD >= 0 || !obj->Static() ||
            obj == cameraOn || oi->passNum > 1)
        {
            continue;
        }
        const int level = oi->drawLOD;
        // Share of the picture near the edges where it used to vanish at once.
        const float distance = std::sqrt(oi->distance2);
        const float detail = distance * scene.GetLodInvWidth();
        const float size = detail > 0.0f ? shape->BoundingSphere() * 2.0f * obj->Scale() / detail : 1e9f;
        // Size on screen in pixels (the 0.125 limit is the 1.25 px cut). A
        // switch or an arrival under kBandMinPx cannot be seen, and blending
        // it costs a draw of its own: those switch at once, as they always did.
        const float px = size / (0.125f * scale) * 1.25f;
        if (px < LodPop::kBandMinPx)
        {
            continue;
        }
        const float vis = LodPop::EdgeShare(distance, objectsZ, s_lodDistBand, false);
        if (vis < 1.0f)
        {
            const int k = LodPop::ShareSamples(vis, samples);
            if (k <= 0)
            {
                oi->drawLOD = LOD_INVISIBLE;
            }
            else if (k < samples)
            {
                oi->fadeFrom = LOD_INVISIBLE;
                oi->fadeT = static_cast<float>(k) / static_cast<float>(samples);
            }
            continue;
        }
        if (shape->NLevels() < 2)
        {
            continue;
        }
        const LodPop::LevelBlend b = LodPop::BandBlend(scene.LodResolution2(oi->distance2), s_lodDistBand,
                                                       [shape](float r2) { return shape->FindSqrtLevel(r2, true); });
        // The band lies before the switch: the object draws its own (finer)
        // level on a share of the samples and the coarser one on the rest.
        if (b.fine == b.coarse || level != b.fine || obj->PassNum(b.coarse) > 1)
        {
            continue;
        }
        const int k = LodPop::ShareSamples(1.0f - b.t, samples);
        if (k <= 0)
        {
            // the last stretch before the switch: the coarser level alone
            oi->drawLOD = static_cast<signed char>(b.coarse);
            oi->passNum = obj->PassNum(b.coarse);
        }
        else if (k < samples)
        {
            oi->fadeFrom = static_cast<signed char>(b.coarse);
            oi->fadeT = static_cast<float>(k) / static_cast<float>(samples);
        }
    }
}

void Scene::DrawObjectsAndShadowsPass1()
{
    // select first objects - those with highest visual priority

    int s = 0, t = 0;
    for (; s < _drawObjects.Size(); s++)
    {
        SortObject* sObj = _drawObjects[s];
        if (!sObj->notUsed)
        {
            if (s != t)
            {
                _drawObjects[t] = sObj;
            }
            t++;
        }
        else
        {
            if (LodPopPath())
            {
                LogListRemoval(sObj, _camera);
            }
            sObj->object->SetInList(nullptr); // removed from the list
        }
    }
    _drawObjects.Resize(t);

#if DO_STAT
    Alpha.Clear();
    Opaque.Clear();
    Shadow.Clear();
#endif

    // remove all objects that should not be used

    using Clock = std::chrono::steady_clock;
    auto msSince = [](Clock::time_point t0)
    { return std::chrono::duration<float, std::milli>(Clock::now() - t0).count(); };
    perf::FrameCounters& counters = perf::GPerfCounters().Frame();
    // The frame-cost switches, as this frame's gates hold them (computed at the
    // world's first 3D pass; off with the classic lighting, so Classic draws as
    // it always did).
    remaster::FeatureGates gates;
    const bool haveGates = GEngine && GEngine->GetFeatureGates(gates) && gates.computed;
    const bool drawGrouping = haveGates && gates[remaster::Feature::DrawGrouping].on;
    const bool hillCull = haveGates && gates[remaster::Feature::HillCull].on;

    const Clock::time_point lodT0 = Clock::now();
    // A cut: the levels held for the old place mean nothing at the new one.
    const bool lodKeep = s_lodBand > 0.0f || s_lodFade > 0.0f || s_lodDistBand > 0.0f || LodPopPath();
    if (lodKeep && _camera)
    {
        static Vector3 s_lastEye = VZero;
        static int s_stillFrames = 0;
        const Vector3 eye = _camera->Position();
        const float moved2 = eye.Distance2(s_lastEye);
        if (moved2 > Square(LodPop::kCutDistance))
        {
            for (int i = 0; i < _drawObjects.Size(); i++)
            {
                SortObject* oi = _drawObjects[i];
                oi->heldLOD = LodPop::kNoLevel;
                oi->fadeFrom = LodPop::kNoLevel;
                oi->fadeOut = false;
            }
        }
        s_stillFrames = moved2 < Square(0.001f) ? s_stillFrames + 1 : 0;
        s_lastEye = eye;
        s_lodBandNow = s_lodBand > 0.0f && s_lodFade > 0.0f && s_stillFrames >= LodPop::kRestFrames ? 1.0f : s_lodBand;
    }
    AdjustComplexity();
    if (_shadowKeep.on)
    {
        // POSEIDON_XP_SHADOWKEEP: every object whose shadow can reach ground
        // the shadow maps shade gets a caster level, whatever the camera sees
        // of the object itself. The legacy shadow level stops at the legacy
        // object-shadow toggle and is several levels coarser than the draw
        // level, so a tree's shadow changed shape the moment the tree left the
        // screen or went behind another; the level here is the draw level
        // (the stateless one for an object the camera does not draw), so an
        // object casts the same shape in view, hidden and off screen.
        remaster::FeatureGates keepGates;
        const bool treeShadows = GEngine->LightingRemasterActive() && GEngine->GetFeatureGates(keepGates) &&
                                 keepGates[remaster::Feature::TreeShadows].on;
        const float keepReach2 = Square(_shadowKeep.mapFar * _shadowKeep.corner + LodPop::kKeepMaxShadow);
        for (int i = 0; i < _drawObjects.Size(); i++)
        {
            SortObject* oi = _drawObjects[i];
            if (oi->distance2 > keepReach2 || !oi->object || !oi->shape)
            {
                continue;
            }
            Object* obj = oi->object;
            if ((oi->shape->Special() & NoShadow) ||
                !remaster::CastsIntoSunShadowMap(obj->CastShadow(), obj->Static(), treeShadows) ||
                !ShadowKeepInReach(obj->Position(), oi->radius))
            {
                continue;
            }
            oi->shadowLOD = oi->drawLOD != LOD_INVISIBLE
                                ? oi->drawLOD
                                : static_cast<signed char>(LevelFromDistance2(oi->shape, oi->distance2, obj->Scale(),
                                                                              obj->Direction(), _camera->Direction()));
        }
    }
    if (s_lodDistBand > 0.0f)
    {
        UpdateLodBands(*this, _drawObjects, GWorld ? GWorld->CameraOn() : nullptr);
    }
    else if (s_lodFade > 0.0f)
    {
        static Foundation::Time s_lastTime = Glob.time;
        float dt = Glob.time - s_lastTime;
        s_lastTime = Glob.time;
        saturate(dt, 0.0f, 0.1f);
        UpdateLodFades(_drawObjects, dt, GWorld ? GWorld->CameraOn() : nullptr);
    }
    else if (lodKeep)
    {
        for (int i = 0; i < _drawObjects.Size(); i++)
        {
            _drawObjects[i]->heldLOD = _drawObjects[i]->drawLOD;
        }
    }
    counters.lodMs += msSince(lodT0);

    // copy objects to working list (mergers)
    // do not copy objects that are not drawn
    {
        // make smaller only when really necessary
        int objNeed = _drawObjects.Size();
        int objHave = _drawMergers.MaxSize();
        if (objNeed >= objHave)
        {
            _drawMergers.Reserve(objNeed, objNeed);
        }
        else if (objNeed * 2 < objHave && objHave > 1024)
        {
            // no need to keep it big now - make it smaller
            _drawMergers.Realloc(objNeed);
        }
        _drawMergers.Resize(0);
        _drawnObjects = 0;
        for (int s = 0; s < _drawObjects.Size(); s++)
        {
            SortObject* sObj = _drawObjects[s];
            if (sObj->drawLOD == LOD_INVISIBLE && sObj->shadowLOD == LOD_INVISIBLE)
            {
                continue;
            }
            if (!sObj->object || !sObj->shape)
            {
                continue;
            }
            _drawMergers.Add(sObj);
            if (sObj->drawLOD != LOD_INVISIBLE)
            {
                ++_drawnObjects;
            }
        }
    }
    // One-shot shape census (perf campaign): triPerfDumpShapes arms this;
    // the next Pass1 logs the top repeated shapes — the instancing-candidate
    // histogram (how many draws are N copies of the same tree/fence/house).
    extern bool gPerfDumpShapesOnce;
    if (gPerfDumpShapesOnce)
    {
        gPerfDumpShapesOnce = false;
        struct Bucket
        {
            const char* name;
            int count;
        };
        std::map<std::string, int> hist;
        for (int s = 0; s < _drawMergers.Size(); s++)
        {
            SortObject* so = _drawMergers[s];
            if (so && so->shape)
                hist[std::string((const char*)so->shape->GetName())]++;
        }
        std::vector<std::pair<int, std::string>> top;
        for (auto& kv : hist)
            top.push_back({kv.second, kv.first});
        std::sort(top.rbegin(), top.rend());
        LOG_INFO(Graphics, "PERF shapes: {} objects, {} unique shapes", _drawMergers.Size(), (int)hist.size());
        for (size_t i = 0; i < top.size() && i < 20; i++)
            LOG_INFO(Graphics, "PERF shape[{}]: {} x{}", (int)i, top[i].second.c_str(), top[i].first);
    }

    // if we sort by shape, we group object with similiar textures
    // sort by distance

#if _ENABLE_CHEATS
    if (InputSubsystem::Instance().GetCheat2ToDo(SDL_SCANCODE_H))
    {
        EnableObjOcc = !EnableObjOcc;
        GlobalShowMessage(500, "Object occlusions %s", EnableObjOcc ? "On" : "Off");
    }
#endif

    const Clock::time_point occT0 = Clock::now();
    if (EnableObjOcc)
    {
        // sort only what needs to checked/drawn for occlusion
        // this will remove especially cloudlets from occlusion testing
        // it also helps to maintain _drawMergers sorted
        // because sort is performed in different array
        // therefore _drawMergers sort may be performed incrementally
        AUTO_STATIC_ARRAY(Ref<SortObject>, occSort, 2048);
        for (int i = 0; i < _drawMergers.Size(); i++)
        {
            SortObject* oi = _drawMergers[i];
            if (oi->drawLOD == LOD_INVISIBLE)
            {
                continue;
            }
            Object* obj = oi->object;
            if (!obj)
            {
                continue;
            }
            LODShape* shape = oi->shape;
            if (!shape)
            {
                continue;
            }

            if (shape->CanBeOccluded() || shape->CanOcclude() && obj->OcclusionView())
            {
                occSort.Add(_drawMergers[i]);
            }
        }
        SortNearestFirst(occSort.Data(), occSort.Size());
// before drawing anything draw cockpit occlusion
// check if we are in internal view
#if 1
        if (GWorld->GetCameraType() == CamInternal && !GWorld->GetCameraEffect())
        {
            Object* obj = GWorld->CameraOn();
            if (obj)
            {
                int view = obj->InsideViewGeomLOD(CamInternal);
                if (view != LOD_INVISIBLE && view >= 0)
                {
                    // draw that particular view geometry
                    obj->AnimateComponentLevel(view);
                    // if object can occlude something, render its components

                    LODShape* lShape = obj->GetShape();

                    Shape* shape = lShape->Level(view);
                    const ConvexComponents* cc = lShape->GetConvexComponents(view);
                    if (cc)
                    {
                        GetOcclusions()->RenderShape(*obj, shape, *cc, ClipAll);
                    }
                    else
                    {
                        LOG_DEBUG(Graphics, "Inconsistent view geom in {}", (const char*)obj->GetDebugName());
                    }
                    obj->DeanimateComponentLevel(view);
                }
            }
        }
#endif

        // W3-1(b): objects beyond the test range that are not occluders are not
        // tested (OcclusionRange.hpp). The range never falls short of the shadow
        // caster reach (SceneShadowPass.cpp: near + distanceCoef * (shadowsZ -
        // near); the projected shadows reach shadowsZ).
        static const float s_occRange = OcclusionRange::ParseRange(std::getenv("POSEIDON_OCC_RANGE"));
        float occRange2 = 0.0f;
        float occInvLeft = 0.0f;
        if (_camera)
        {
            const float occNear = _camera->ClipNear();
            // The shadow maps' far distance exactly as SceneShadowPass.cpp
            // computes smMapFar: a High shadow tier reaches past shadowsZ, and
            // an untested hidden object inside that reach would cast from the
            // wrong LOD.
            const Engine::ShadowMapTuning occSmTune = GEngine->GetShadowMapTuning();
            const float occShadowsZ = Poseidon::shadow::xp::MapFar(ENGINE_CONFIG.shadowsZ, ENGINE_CONFIG.tacticalZ,
                                                                   ENGINE_CONFIG.objectsZ, occSmTune.reach) +
                                      Poseidon::shadow::xp::RadialFadeBand();
            const float occCasterReach = occNear + occSmTune.distanceCoef * (occShadowsZ - occNear);
            occRange2 = OcclusionRange::TestRange2(s_occRange, std::max(occShadowsZ, occCasterReach));
            occInvLeft = _camera->InvLeft();
        }
        OcclusionRange::Stats occStats;

        // draw occlusions front to back
        for (int i = occSort.Size(); --i >= 0;)
        {
            SortObject* oi = occSort[i];
            Object* obj = oi->object;
            LODShape* shape = oi->shape;
            if (oi->drawLOD == LOD_INVISIBLE)
            {
                continue;
            }
            // select view geometry
            // no occlusions for or by camera vehicle
            if (obj == GWorld->CameraOn())
            {
                continue;
            }
            // if object is small and simple if should not be tested nor rendered
            bool occluded = false;
            // check object distance
            // if the objects is very near, do not check occlusion

            if (shape->CanBeOccluded())
            {
                // do not occlude things that are very near
                // the test would be very slow and is very like to fail
                if (FarEnoughForOcclusion(oi))
                {
                    const bool beyond = occRange2 > 0.0f && oi->distance2 >= occRange2;
                    if (beyond && OcclusionRange::SkipTest(oi->distance2, occRange2,
                                                           OccluderViewGeometry(oi, occInvLeft) != nullptr))
                    {
                        ++occStats.skipped;
                    }
                    else
                    {
                        Vector3 minMax[2];
                        obj->AnimatedMinMax(oi->drawLOD, minMax);
                        ++occStats.tested;
                        occStats.farTested += beyond ? 1 : 0;
                        // check if object is occluded by objects
                        if (!GetOcclusions()->TestBBox(*oi->object, minMax, oi->orClip))
                        {
                            occluded = true;
                            ++occStats.culled;
                        }
                    }
                }
            }
            if (!occluded)
            {
                if (Shape* view = OccluderViewGeometry(oi, occInvLeft))
                {
                    obj->AnimateViewGeometry();
                    // if object can occlude something, render its components
                    GetOcclusions()->RenderShape(*oi->object, view, shape->GetViewComponents(), oi->orClip);
                    obj->DeanimateViewGeometry();
                }
            }
            else
            {
                // occluded - do not draw, do not render into occlusion buffer
                oi->drawLOD = LOD_INVISIBLE;
            }
        }
        ReportOcclusionStats(occStats, occRange2);
    }
    counters.occlusionMs += msSince(occT0);

    // Hill culling: objects the terrain hides from the camera are not drawn.
    // After the object occlusion pass, so its buffer and its results are the
    // same with the cull on or off. A hidden object is only flagged
    // (hillHidden): it keeps its draw LOD, so the sorts below and in Pass2
    // order the list exactly as with the cull off (the shadow-map pass emits
    // its casters in that order, and overlapping leaf cards resolve equal
    // depths by it), the shadow-map pass casts its shadow from its draw LOD as
    // before (behind a ridge from the camera, its shadow can still reach
    // ground in view), and the draw loops skip it. Its draw's texture mipmap
    // hints are given without the draw (PrepareDrawTextures): the shadow pass
    // loads the alpha-tested caster textures from them, and a texture it
    // shares with an object in view keeps its mipmaps. Only static objects are
    // tested: their draw is Object::Draw's, which PrepareDrawTextures mirrors
    // (vehicles and men draw through their own overrides).
    //
    // Hiding any draw is safe for the shadow-map pass: it sets its own
    // sampler and polygon offset, so the depth its casters store does not
    // depend on which draw came last before it.
    _hillHiddenObjects = 0;
    if (hillCull && _camera && GetLandscape())
    {
        HillCull& hc = GHillCull();
        if (!hc.governor.ShouldRun())
        {
            counters.hillIdle = 1;
        }
        else if (EnsureHillTerrain(hc, *GetLandscape()))
        {
            const Clock::time_point hillT0 = Clock::now();
            const Vector3 eye = _camera->Position();
            const float range = GetFogMaxRange();
            if (!hc.table.Matches(eye.X(), eye.Y(), eye.Z(), range))
            {
                hc.table.Build(hc.terrain, eye.X(), eye.Y(), eye.Z(), range, TerrainOcclusion::Params());
                ++counters.hillBuilds;
            }
            const int wedgesBefore = hc.table.WedgesBuilt();
            const Object* cameraOn = GWorld ? GWorld->CameraOn() : nullptr;
            int tested = 0;
            for (int i = 0; i < _drawMergers.Size(); i++)
            {
                SortObject* oi = _drawMergers[i];
                Object* obj = oi->object;
                if (oi->drawLOD == LOD_INVISIBLE || !obj || !obj->Static() || obj == cameraOn || oi->passNum == 3)
                {
                    continue;
                }
                const Vector3 pos = obj->Position();
                const TerrainOcclusion::Result r = hc.table.Test(pos.X(), pos.Y(), pos.Z(), obj->GetRadius());
                if (r == TerrainOcclusion::Result::NotTested)
                {
                    continue;
                }
                ++tested;
                if (r == TerrainOcclusion::Result::Hidden)
                {
                    oi->hillHidden = true;
                    obj->PrepareDrawTextures(oi->drawLOD, *obj);
                    ++_hillHiddenObjects;
                }
            }
            hc.governor.Report(tested, _hillHiddenObjects);
            counters.hillRan = 1;
            counters.hillTested = tested;
            counters.hillHidden = _hillHiddenObjects;
            counters.hillWedges += hc.table.WedgesBuilt() - wedgesBefore;
            counters.hillMs += msSince(hillT0);
        }
    }

    if (LodPopPath() && _camera)
    {
        RecordLodPop(*this, _drawObjects, *_camera, _lodInvWidth, counters);
    }

    SortByShape(_drawMergers.Data(), _drawMergers.Size());
    // first of all draw non-alpha objects

#if DRAW_OBJS
    {
        // Instanced runs (perf effort 08): _drawMergers is shape-sorted, so
        // identical static shapes arrive contiguously. A batchable run draws
        // the head once inside Begin/EndInstancedRun, and every TL section then
        // renders all K instances from the WorldInstances matrix array. The
        // predicate keeps per-object state out of batches: static, proxy-free,
        // not OnSurface/IsColored, equal obj-special, no local light, a tight
        // distance band so the head's constant-fog value is representative for
        // the whole batch, and no object whose draw rewrites its own vertices
        // (a land-following bush fitted to its own patch of slope, a damaged
        // house): the run draws every member from the head's vertex buffer
        // (InstancedRunRules.hpp). Local lights: with drawGrouping off, any
        // light slot in the scene turns runs off for every object (Abel's
        // street lamps hold slots by day too); with it on, an object is kept
        // out only when its own light list is not empty, unless the frame's
        // tiled light list lights every draw (lightTiles): then no draw's own
        // list matters and lit objects join runs too.
        const bool noLocalLights = NLights() == 0;
        const bool runsAllowed = noLocalLights || drawGrouping;
        Scene* lightsPerObject = drawGrouping && !noLocalLights && !GEngine->LocalLightsFromTiles() ? this : nullptr;
        counters.lightSlots = NLights();
        counters.activeLights = _aLights.Size();
        for (int i = 0; i < _drawMergers.Size();)
        {
            SortObject* oi = _drawMergers[i];
            LODShape* shape = oi->object->GetShape();
            if (oi->drawLOD == LOD_INVISIBLE || oi->hillHidden)
            {
                i++;
                continue;
            }
            PoseidonAssert(oi->drawLOD >= 0);
            Shape* sShape = shape->LevelOpaque(oi->drawLOD);
            if (!sShape)
            {
                i++;
                continue;
            }
            if (oi->passNum > 1)
            {
                i++;
                continue;
            }
            if (s_lodDistBand > 0.0f && oi->fadeFrom != LodPop::kNoLevel)
            {
                // POSEIDON_XP_LODBAND: drawn after the loop, grouped by sample
                // mask, so the mask changes (each a flush) stay a handful a
                // frame instead of two or three per object in a band.
                s_bandDeferred.push_back(oi);
                i++;
                continue;
            }

            int runEnd = i + 1;
            const InstancedRun::RunEntry head = InstancedRunEntry(oi, shape, oi->drawLOD, sShape, lightsPerObject);
            const render::LegacySpec headSpec = render::SplitLegacy(head.special);
            const bool headBatchable = runsAllowed && InstancedRun::CanHead(head) && sShape->NProxies() == 0 &&
                                       !render::Has(headSpec.routing, render::Routing::OnSurface) &&
                                       !render::Has(headSpec.routing, render::Routing::IsColored) &&
                                       oi->object != GWorld->CameraOn();
            if (head.localLit)
            {
                ++counters.litObjects;
            }
            if (headBatchable)
            {
                GEngine->InstancedRunReset();
                if (GEngine->InstancedRunAdd(oi->object->Transform()))
                {
                    while (runEnd < _drawMergers.Size())
                    {
                        SortObject* oj = _drawMergers[runEnd];
                        if (oj->hillHidden)
                        {
                            runEnd++; // in the run's span, never drawn
                            continue;
                        }
                        if (!InstancedRun::CanJoin(head,
                                                   InstancedRunEntry(oj, shape, oi->drawLOD, sShape, lightsPerObject)))
                        {
                            break;
                        }
                        if (!GEngine->InstancedRunAdd(oj->object->Transform()))
                        {
                            break;
                        }
                        runEnd++;
                    }
                }
            }

            // The run's span may hold hidden objects (hillHidden): they joined no
            // instance list and are never drawn.
            int runLen = 0;
            for (int k = i; k < runEnd; k++)
            {
                runLen += _drawMergers[k]->hillHidden ? 0 : 1;
            }
            GSectionFilter = SectionClassFilter::OpaqueAndCutout;
            counters.opaqueObjects += runLen;
            if (headBatchable && runLen >= 4)
            {
                ++counters.runs;
                counters.runObjects += runLen;
                GEngine->BeginInstancedRunUpload();
                DrawSortObject(oi);
                if (!GEngine->EndInstancedRun())
                {
                    // Vertex-soup sections can't instance — those drew only for the
                    // head; redraw the rest scalar (TL overdraw is z-equal opaque).
                    for (int k = i + 1; k < runEnd; k++)
                    {
                        if (!_drawMergers[k]->hillHidden)
                        {
                            DrawSortObject(_drawMergers[k]);
                        }
                    }
                }
            }
            else
            {
                for (int k = i; k < runEnd; k++)
                {
                    if (!_drawMergers[k]->hillHidden)
                    {
                        DrawSortObject(_drawMergers[k]);
                    }
                }
            }
            GSectionFilter = SectionClassFilter::All;
#if DO_STAT
            Opaque.Count(shape->Name());
#endif
            i = runEnd;
        }
        if (!s_bandDeferred.empty())
        {
            counters.opaqueObjects += static_cast<int>(s_bandDeferred.size());
            DrawBandDeferred(s_bandDeferred);
            s_bandDeferred.clear();
        }
    }
#endif
}

// A surface overlay (road / decal): OnSurface-routed geometry drawn as a
// polygon-offset decal over the terrain, detected from the draw-LOD level
// spec exactly as Object::PassNum classifies surface objects.
static bool IsSurfaceSortObject(const SortObject* oi)
{
    if (!oi->object)
        return false;
    LODShape* lShape = oi->object->GetShape();
    if (!lShape)
        return false;
    Shape* s = lShape->Level(oi->drawLOD);
    if (!s)
        return false;
    const int spec = s->Special() | oi->object->GetObjSpecial();
    return render::IsOnSurfaceSpec(spec);
}

// Does Pass2's pre-shadow loop draw this object's blend sections (a road or
// decal surface)? It mirrors exactly the post-shadow blend branch that skips
// these objects (passNum<=1 && HasBlendSections && surface). With grass
// enabled a surface object is passNum==2 (whole-alpha) and is drawn by the
// passNum==2 branch; drawing it in the pre-shadow loop too would double-draw
// it and re-overwrite the shadow. The cheap passNum gate also avoids the
// per-object Special() probe for non-surface objects.
static bool IsSurfaceBlendDraw(const SortObject* oi)
{
    if (oi->drawLOD == LOD_INVISIBLE || oi->passNum > 1 || !IsSurfaceSortObject(oi))
        return false;
    LODShape* shp = oi->object->GetShape();
    Shape* lvl = shp ? shp->LevelOpaque(oi->drawLOD) : nullptr;
    return lvl && lvl->HasBlendSections();
}

// On-surface ordering: PassOrder (roads before decals), then shape, then
// back-to-front within a shape — see Scene/SurfaceDrawOrder.hpp. The key, with
// its virtual PassOrder call, is read once per entry (W3-11; was CmpSurfaceObj).
static void SortBySurfaceOrder(SortObjectItem* items, int n)
{
    static DrawListSort::Scratch<Poseidon::SurfaceDraw::SurfaceDrawKey, SortObject> scratch;
    DrawListSort::SortByKeys(
        items, n,
        [](const SortObject* o)
        {
            return Poseidon::SurfaceDraw::SurfaceDrawKey{
                o->object ? o->object->PassOrder(o->drawLOD) : 0,
                o->object ? static_cast<const void*>(o->object->GetShape()) : nullptr, o->distance2};
        },
        [](const Poseidon::SurfaceDraw::SurfaceDrawKey& a, const Poseidon::SurfaceDraw::SurfaceDrawKey& b)
        { return DrawListSort::CompareSurfaceKeys(a, b); }, scratch);
}

void Scene::DrawObjectsAndShadowsPass2()
{
    // must be sorted by distance
    // then draw shadows
    // note: some polygons can not be ordered
    int nDraw = _drawMergers.Size();
#if DRAW_OBJS
    if (GetLandscape())
    {
        // BeginShadowPass / EndShadowPass bracket the projected shadow
        // loop; on GL33 they flush the triangle queues so the per-poly
        // shadows commit between the world geometry and the alpha pass.

        // Surface-overlay blend sections (roads/decals) commit BEFORE the
        // shadow pass so the projected shadows darken on top of the road
        // instead of the road's blend section repainting over the shadowed
        // road pixels.  Their opaque sections already drew in pass 1; only
        // the OnSurface blend section (the visible asphalt) is drawn here.
        GEngine->FlushQueues();
        GEngine->EnableReorderQueues(false);
        // PassOrder, not distance: a fresh decal carries distance2~=0
        // (SetAutoCenter(false)) and would tie the road tile under the vehicle,
        // letting the road repaint over it.  Re-sorted by distance below.
        SortBySurfaceOrder(_drawMergers.Data(), nDraw);
        for (int i = 0; i < nDraw; i++)
        {
            SortObject* oi = _drawMergers[i];
            if (!IsSurfaceBlendDraw(oi) || oi->hillHidden)
                continue;
            GSectionFilter = SectionClassFilter::BlendOnly;
            DrawSortObject(oi);
            GSectionFilter = SectionClassFilter::All;
        }
        GEngine->FlushQueues();
        GEngine->EnableReorderQueues(true);

        // Shadow-map depth pass (durable shadow fix; off by default).  Render the
        // visible casters' geometry from the sun into the cascade depth maps.
        // Invisible on its own — the lit shaders sample it once enabled; the
        // projected accumulator still runs.  Body in SceneShadowPass.cpp (keeps
        // Scene.cpp under the file-size limit).
        if (GEngine->ShadowMapsEnabled())
        {
            // Sun shadows fade out at dusk and vanish at night (no sun above the
            // horizon = no sun shadow), as the projected path and OFP/ArmA/FP do.
            // NightEffect is 0 in daylight, ramps through twilight, 1 at night. The
            // lit shaders fade the darkness by this factor; skip the depth pass once
            // it is fully dark (nothing would be visible, and a stale map is hidden
            // because the darkness factor is 0).
            float sunFactor = 1.0f - _mainLight->NightEffect();
            sunFactor = floatMax(0.0f, floatMin(1.0f, sunFactor));
            GEngine->SetShadowMapSunFactor(sunFactor);
            // The remastered sun keeps lighting the scene until it is about 1.5
            // degrees below the horizon: keep its shadows live that long (a
            // skipped pass would leave a frozen map in view).
            const bool remasterSunUp = GEngine->LightingRemasterActive() && _mainLight->SunDirection().Y() < 0.026f;
            // At night the remastered moon key (moonShadows) casts the shadows:
            // the pass renders from the moon (SceneShadowPass.cpp).
            bool smPass = false;
            if (sunFactor > 0.01f || remasterSunUp || GEngine->RemasterMoonKey())
            {
                RenderShadowMapDepthPass(nDraw);
                smPass = true;
            }
            if (LodPopPath() && _camera)
            {
                RecordShadowPop(*this, _drawObjects, *_camera, smPass);
            }
        }

        // Projected accumulator shadows — the fallback path.  When shadow maps are
        // on, the lit shaders darken receivers from the depth map instead, so skip
        // this to avoid two overlapping shadows.
        if (!GEngine->ShadowMapsEnabled())
        {
            // Frozen-pose caster accounting for the projected-shadow cache: Object::PrepareShadow
            // bumps gShadowFrozenRouted each time a settled corpse / stopped vehicle is served from
            // the cache instead of re-projected.  Reset per pass, then publish the count the
            // triShadowAssertFrozenCasts verb reads.
            extern int gShadowFrozenCasters;
            extern int gShadowFrozenRouted;
            GEngine->BeginShadowPass();
            gShadowFrozenRouted = 0;
            for (int i = 0; i < nDraw; i++)
            {
                SortObject* oi = _drawMergers[i];
                if (oi->shadowLOD == LOD_INVISIBLE)
                {
                    continue;
                }
                DrawExShadow(oi);
            }
            GEngine->EndShadowPass();
            gShadowFrozenCasters = gShadowFrozenRouted;
        }
    }
#endif
    // draw alpha parts of roads
    GEngine->FlushQueues();
    // The opaque world is complete: the remastered TAA keeps a copy for its
    // particle mask before the translucent objects go on top.
    GEngine->BeginTranslucentObjects();
    GEngine->EnableReorderQueues(false);
    SortAlphaFarToNear(_drawMergers.Data(), _drawMergers.Size());
    // last draw alpha objects (not roads - they are already drawn)
    for (int i = 0; i < nDraw; i++)
    {
        SortObject* oi = _drawMergers[i];
        LODShape* shape = oi->object->GetShape();
        if (oi->drawLOD == LOD_INVISIBLE || oi->hillHidden)
        {
            continue;
        }
        PoseidonAssert(oi->drawLOD >= 0);
        Shape* sShape = shape->LevelOpaque(oi->drawLOD);
        if (!sShape)
        {
            continue;
        }
        if (oi->passNum == 2)
        {
            // whole-alpha object (cloudlets, <0xd0 objects): draw all of it here
            DrawSortObject(oi);
#if DO_STAT
            Alpha.Count(shape->Name());
#endif
        }
        else if (oi->passNum <= 1 && sShape->HasBlendSections())
        {
            // Surface overlays (roads/decals: asfaltka/cesta) own their visible
            // surface as a blend section; those drew before the shadow pass so
            // the shadow darkens on top of them — skip here (see the pre-shadow
            // surface-blend pass above).
            if (IsSurfaceSortObject(oi))
            {
                continue;
            }
            // opaque object that owns translucent (blend) sections — e.g. a vehicle's
            // glass. Its opaque+cutout sections drew in pass 1; revisit it here, in
            // back-to-front order, drawing only the blend sections so they blend over
            // the already-drawn scene behind them (depth-write stays on — original parity).
            GSectionFilter = SectionClassFilter::BlendOnly;
            DrawSortObject(oi);
            GSectionFilter = SectionClassFilter::All;
#if DO_STAT
            Alpha.Count(shape->Name());
#endif
        }
    }
    GEngine->EnableReorderQueues(true);
}

#include <Poseidon/Foundation/Common/Filenames.hpp>

void Scene::DrawObjectsAndShadowsPass3()
{
#if DRAW_OBJS
    int i, nDraw = _drawMergers.Size();
    // pass 3 - draw cockpits after all external alpha effects
    for (i = 0; i < nDraw; i++)
    {
        SortObject* oi = _drawMergers[i];
        LODShape* shape = oi->object->GetShape();
        if (oi->drawLOD != LOD_INVISIBLE && !oi->hillHidden)
        {
            PoseidonAssert(oi->drawLOD >= 0);
            Shape* sShape = shape->LevelOpaque(oi->drawLOD);
            if (!sShape)
            {
                continue;
            }
            if (oi->passNum == 3)
            {
                {
                    DrawSortObject(oi);
                }
            }
        }
    }
#endif

#if DO_STAT
    LOG_DEBUG(Graphics, "Alpha objects");
    Alpha.Report();
    LOG_DEBUG(Graphics, "Opaque objects");
    Opaque.Report();
    LOG_DEBUG(Graphics, "Shadow objects");
    Shadow.Report();
#endif
}

void Scene::DrawReflections(const WaterLevel& water) {}

void Scene::DrawCollisionStar(Vector3Par pos, float size, PackedColor color)
{
    if (_collisionStar.NotNull() && size >= 0.01)
    {
        Ref<Object> star = new ObjectColored(_collisionStar->GetShape(), -1);
        star->SetTransform(M4Identity);
        star->SetPosition(pos);
        star->SetScale(size * 4);
        star->SetConstantColor(color);
        GetLandscape()->ShowObject(star);
    }
}

void Scene::DrawDiagModel(Vector3Par pos, LODShapeWithShadow* shape, float size, PackedColor color)
{
    Ref<Object> star = new ObjectColored(shape, -1);
    star->SetTransform(M4Identity);
    star->SetPosition(pos);
    star->SetScale(size / shape->BoundingSphere());
    star->SetConstantColor(color);
    GetLandscape()->ShowObject(star);
}

void Scene::DrawVolumeLight(LODShapeWithShadow* shape, PackedColor color, const Frame& pos, float size)
{
    if (!shape)
    {
        static bool warned = false;
        if (!warned)
        {
            LOG_ERROR(Graphics, "Skipping visible light volume because its shape failed to load");
            warned = true;
        }
        return;
    }

    Ref<Object> object = new ObjectColored(shape, -1);
    // draw light shape
    object->SetTransform(pos.Transform());
    object->SetPosition(object->PositionModelToWorld(shape->BoundingCenter()));
    // ObjectForDrawing(object,-1,(ENGINE_CONFIG.reflections&REFL_OBJECT)!=0);

    object->SetConstantColor(color);
    object->SetScale(size);
    ObjectForDrawing(object);
}

void Scene::DrawExShadow(SortObject* oi)
{
    // calculate position of the shadow of the object top
    Object* obj = oi->object;
    if (!obj)
    {
        return;
    }
    int level = oi->shadowLOD;
    LODShapeWithShadow* shape = oi->shape;
    Vector3Val objPos = obj->Position();
    Point3 shadowPos = objPos;
    const FrameBase& pos = *obj;

    if (!ShadowPos(objPos, shadowPos, _mainLight))
    {
        return;
    }

    Ref<Shape> sShape = obj->PrepareShadow(level, shadowPos, pos);
    // check bbox clipping and occlusion

    // per object clipping possible - bounding sphere was recalculated
    if (!sShape)
    {
        return; // no shadow
    }
    if (sShape->NPos() <= 0)
    {
        return;
    }

    float bRadius = sShape->BSphereRadius() * pos.Scale();
    Vector3Val bCenterM = sShape->BSphereCenter();
    Vector3Val bCenter = pos.PositionModelToWorld(bCenterM);
    // try to clip bounding sphere
    // if whole bounding sphere is out, object is already skipped
    if (GScene->GetCamera()->IsClipped(bCenter, bRadius, 1))
    {
        return;
    }
    ClipFlags clipFlags = GScene->GetCamera()->MayBeClipped(bCenter, bRadius, 1);
    if (EnableObjOcc)
    {
        bool occluded = false;
        if (shape->CanBeOccluded())
        {
            if (FarEnoughForOcclusion(oi))
            {
                // check occlusion - bbox recalculation
                Vector3 minMax[2];
                Vector3Val pos0 = sShape->Pos(0);
                minMax[0] = pos0;
                minMax[1] = pos0;
                for (int i = 0; i < sShape->NPos(); i++)
                {
                    Vector3Val posI = sShape->Pos(i);
                    CheckMinMaxIter(minMax[0], minMax[1], posI);
                }

                // check if object is occluded by objects
                if (!GetOcclusions()->TestBBox(pos, minMax, clipFlags))
                {
                    occluded = true;
                }
            }
        }
        if (occluded)
        {
            return;
        }
    }

    // first draw shadows of some proxies?
    for (int i = 0; i < obj->GetProxyCount(level); i++)
    {
        if (!obj->CastProxyShadow(level, i))
        {
            continue;
        }
        Matrix4 trans = obj->Transform(), invTrans = obj->GetInvTransform();

        LODShapeWithShadow* pshape = nullptr;
        Object* proxy = obj->GetProxy(pshape, level, trans, invTrans, *obj, i);
        if (!proxy)
        {
            continue;
        }
        // note: it is not sure we need to draw shadows of all proxies
        // proxy shadow must be always calculated dynamically
        // we do not want to draw proxies of complex objects like vehicle crews

        if (!pshape)
        {
            continue;
        }
        float dist2 = trans.Position().Distance2(GetCamera()->Position());
        int plevel =
            LevelShadowFromDistance2(pshape, dist2, trans.Scale(), trans.Direction(), GetCamera()->Direction());
        if (plevel != LOD_INVISIBLE)
        {
            plevel = pshape->FindNearestWithoutProperty(plevel, "lodnoshadow");
            if (plevel < 0)
            {
                plevel = LOD_INVISIBLE;
            }
        }
        if (plevel == LOD_INVISIBLE)
        {
            continue;
        }

        proxy->Animate(plevel);

        FrameWithInverse pframe(trans, invTrans);
        if (proxy->RecalcShadow(plevel, pframe, true, pshape))
        {
            LODShape* shadowShape = pshape->Shadow();
            Ref<Shape> ret = shadowShape->LevelOpaque(plevel);
            // we can draw shape ret now as shadow
            proxy->DrawShadow(ret, shadowPos, ClipAll, pframe);
        }

        proxy->Deanimate(plevel);
    }
    /**/

    // obj->DrawShadow(shadow,shadowPos,ClipAll,pos);
    obj->DrawShadow(sShape, shadowPos, clipFlags, pos);

#if DO_STAT
    Shadow.Count(oi->shape->Name());
#endif
}

// different global variables

void ManCleanUp();

void ClearShapes()
{
    VehicleTypes.Clear();
    // WeaponInfos.Clear();
    RoadTypes.Clear();
    // log any shape still resident in the cache at shutdown (refs would
    // be from cache itself plus any holdouts on the entity side).
    Shapes.ForEach([](LODShapeWithShadow& shape)
                   { Log("Shape %s not released (%d refs).", shape.Name(), shape.RefCounter()); });
    Shapes.Clear();

    MagazineTypes.Clear();
    WeaponTypes.Clear();
}

bool Scene::ShadowPos(Vector3Par pos, Vector3& aprox, LightSun* light)
{ //
    Vector3Val dir = light->ShadowDirection();
    if (dir.Y() > 0)
    {
        return false; // shadow casted up - no real shadow
    }
    const float maxShadow = 300;
    float t = GetLandscape()->IntersectWithGround(&aprox, pos, dir, 0, maxShadow * 1.1);
    if (t > maxShadow)
    {
        // no intersection
        return false;
    }
    if (t <= 0.01f)
    {
        // up -> on surface required
        aprox[1] = GLandscape->SurfaceY(aprox[0], aprox[2]);
    }
    return true;
}
