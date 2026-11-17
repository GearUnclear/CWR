// Far draw: a unit with the farDraw trait stays visible past the object
// distance (objectsZ) and past the view distance, fogged down to a silhouette.
//
// Two cases, decided per frame in FarBeginFrame:
//  - the unit is inside the camera far plane: it goes through the normal
//    object pass, exempt from the objectsZ cut, with a fog floor so the stock
//    fog cannot erase it;
//  - the unit reaches past the far plane (the far plane is the view distance,
//    so normal geometry is clipped there): it is drawn by FarDrawPass with a
//    projection that brackets it, into the reserved far slice of the depth
//    range, after the terrain and the horizon and before the object passes.
//    Everything the main scene draws is therefore in front of it, it covers
//    the sky and the horizon band, and it keeps exact depth inside itself.

#include <Poseidon/Game/Kaiju/KaijuService.hpp>
#include <Poseidon/Game/Kaiju/KaijuMath.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Scene/Object.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/World/World.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <Poseidon/Foundation/platform.hpp>
#include <algorithm>

namespace Poseidon::Kaiju
{

// Development switch for the far pass (kjFarDebug n), 0 in normal use.
// bit 0: leave the depth range alone; bit 1: draw unfogged; bit 2: clear
// depth before the far pass; bit 3: log each far draw; bit 4: no ground haze.
int GFarDebug = 0;

namespace
{
bool s_sliceReserved = false;
}

void FarBeginFrame(Scene& scene)
{
    if (!GFarCount)
    {
        return;
    }
    const Camera* cam = scene.GetCamera();
    if (!cam)
    {
        return;
    }
    const float fogEnd = scene.GetFogMaxRange();
    const float night = scene.MainLight() ? scene.MainLight()->NightEffect() : 0.0f;
    for (auto& up : Units())
    {
        Unit& u = *up;
        u.farMode = FarOff;
        u.farFloor = 0;
        Object* obj = u.obj.GetLink();
        if (!obj || u.traits.farDraw <= 0 || !obj->GetShape())
        {
            continue;
        }
        Vector3Val pos = obj->Position();
        const float radius = obj->GetRadius();
        u.farDist = cam->Position().Distance(pos);
        u.farPlanarZ = (scene.ScaledInvTransform() * pos).Z();
        if (u.farDist > u.traits.farDraw)
        {
            continue; // past the limit: stock rules apply
        }
        u.farFloor = FarFogFloor(u.farDist, fogEnd, u.traits.farDraw, u.traits.farContrast);
        if (night > 0 && u.traits.farGlow > 0)
        {
            // at night what carries is his own light, not his silhouette
            u.farFloor = FarNightFloor(u.farFloor, u.farDist, u.traits.farDraw, u.traits.farGlow, night);
        }
        if (!FarNeedsOwnPass(u.farPlanarZ, radius, cam->ClipFar()))
        {
            u.farMode = FarNormal;
            u.farFramesNormal++;
            continue;
        }
        // off screen to the side, above, below or behind: nothing to draw
        if (cam->IsClipped(pos, radius, 1) & ~ClipBack)
        {
            u.farMode = FarHidden;
            continue;
        }
        u.farMode = FarOwnPass;
    }
}

bool FarOwnsDraw(const Object* obj)
{
    const Unit* u = FindUnit(obj);
    return u && (u->farMode == FarOwnPass || u->farMode == FarHidden);
}

bool FarExemptFromObjectsZ(const Object* obj)
{
    const Unit* u = FindUnit(obj);
    return u && u->farMode == FarNormal;
}

float FarFogFloorFor(const Object* obj)
{
    const Unit* u = FindUnit(obj);
    return u && u->farMode == FarNormal ? u->farFloor : 0.0f;
}

void NoteDrawLevel(const Object* obj, int level)
{
    if (Unit* u = FindUnit(obj))
    {
        u->lastLevel = level;
    }
}

bool FarKeepsSimulating(const Object* obj, float dist2)
{
    const Unit* u = FindUnit(obj);
    return u && u->traits.farDraw > 0 && dist2 < Square(u->traits.farDraw);
}

void FarBeginScene(Scene& scene)
{
    s_sliceReserved = false;
    if (!GFarCount || !GEngine)
    {
        return;
    }
    for (auto& up : Units())
    {
        if (up->farMode == FarOwnPass)
        {
            s_sliceReserved = true;
            break;
        }
    }
    if (s_sliceReserved && !(GFarDebug & 1))
    {
        GEngine->SetDepthRange(0.0f, kFarDepthSplit);
    }
}

void FarEndScene()
{
    if (s_sliceReserved && GEngine && !(GFarDebug & 1))
    {
        GEngine->SetDepthRange(0.0f, 1.0f);
    }
    s_sliceReserved = false;
}

void FarDrawPass(Scene& scene)
{
    if (!s_sliceReserved)
    {
        return;
    }
    Camera* cam = scene.GetCamera();
    std::vector<Unit*> order;
    for (auto& up : Units())
    {
        if (up->farMode == FarOwnPass && up->obj.GetLink())
        {
            order.push_back(up.get());
        }
    }
    if (order.empty())
    {
        return;
    }
    // back to front, each cleared out of the next one's way by its own bracket
    std::sort(order.begin(), order.end(), [](const Unit* a, const Unit* b) { return a->farPlanarZ > b->farPlanarZ; });

    const float oldNear = cam->ClipNear();
    const float oldFar = cam->ClipFar();
    GEngine->FlushQueues();
    if (GFarDebug & 4)
    {
        GEngine->Clear(true, false);
    }
    for (Unit* u : order)
    {
        Object* obj = u->obj.GetLink();
        LODShapeWithShadow* shape = obj->GetShapeOnPos(obj->Position());
        if (!shape)
        {
            continue;
        }
        int level = u->traits.lodPin ? scene.LevelFromDistance2Pinned(shape, Square(u->farDist), obj->Scale(),
                                                                      obj->Direction(), cam->Direction(), true)
                                     : scene.LevelFromDistance2(shape, Square(u->farDist), obj->Scale(),
                                                                obj->Direction(), cam->Direction(), true);
        u->lastLevel = level;
        if (level == LOD_INVISIBLE)
        {
            continue;
        }
        ClipBracket b = FarClipBracket(u->farPlanarZ, obj->GetRadius());
        cam->SetClipRange(b.zNear, b.zFar);
        GEngine->UpdateProjection();
        if (!(GFarDebug & 1))
        {
            GEngine->SetDepthRange(kFarDepthSplit, 1.0f);
        }
        // both T&L paths: a skinned Man goes through the software one
        const float floor = (GFarDebug & 2) ? 1.0f : u->farFloor;
        GEngine->SetFogFloor(floor);
        scene.SetFogFloor(floor);
        const int drawsBefore = gPerfDrawCalls;
        obj->Draw(level, ClipAll & ~ClipBack, *obj);
        GEngine->FlushQueues();
        // the ground haze its feet stand in (stock fog, so the floors come off first)
        GEngine->SetFogFloor(0.0f);
        scene.SetFogFloor(0.0f);
        if (GLandscape && !(GFarDebug & 16))
        {
            const float feetY = obj->PositionModelToWorld(Vector3(0, shape->Min().Y(), 0)).Y();
            const float height = shape->Max().Y() - shape->Min().Y();
            const HazeBand haze =
                FarHazeBand(u->farPlanarZ, obj->GetRadius(), feetY - cam->Position().Y(), height, u->farDist, oldFar);
            if (haze.topY > haze.midY + 0.01f)
            {
                GLandscape->DrawHorizont(scene, haze.z, haze.midY, haze.topY);
            }
        }
        if (GFarDebug & 8)
        {
            Shape* s = shape->LevelOpaque(level);
            LOG_INFO(World,
                     "kaiju far pass: level {} of {} faces {} dist {:.1f} planarZ {:.1f} radius {:.1f} bracket "
                     "{:.1f}..{:.1f} "
                     "cam clip {:.1f}..{:.1f} floor {:.3f} gl draws {}",
                     level, shape->NLevels(), s ? s->NFaces() : -1, u->farDist, u->farPlanarZ, obj->GetRadius(),
                     b.zNear, b.zFar, cam->ClipNear(), cam->ClipFar(), u->farFloor, gPerfDrawCalls - drawsBefore);
        }
        u->farFramesOwnPass++;
    }
    GEngine->SetFogFloor(0.0f);
    scene.SetFogFloor(0.0f);
    cam->SetClipRange(oldNear, oldFar);
    GEngine->UpdateProjection();
    if (!(GFarDebug & 1))
    {
        GEngine->SetDepthRange(0.0f, kFarDepthSplit);
    }
}

} // namespace Poseidon::Kaiju
