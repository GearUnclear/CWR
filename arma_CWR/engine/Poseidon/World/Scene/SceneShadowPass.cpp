#include <Poseidon/Core/Config/EngineConfig.hpp>
#include <Poseidon/Core/Global.hpp>

#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp>
#include <Poseidon/World/Scene/Object.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Simulation/FrameInv.hpp>
#include <Poseidon/Graphics/Rendering/Primitives/Poly.hpp>
#include <Poseidon/Graphics/Shadow/ShadowMath.hpp>
#include <Poseidon/Graphics/Shadow/ShadowXp.hpp>
#include <Poseidon/Graphics/Remaster/SunShadows.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <array>
#include <utility>
#include <Poseidon/Foundation/Containers/StreamArray.hpp>
#include <Poseidon/Foundation/Math/Math3D.hpp>
#include <Poseidon/Foundation/Math/Math3DP.hpp>
#include <Poseidon/Foundation/Math/MathOpt.hpp>
#include <Poseidon/Foundation/Types/Pointers.hpp>

// Test/debug instrumentation: count of solid shadow-caster vertices contributed by
// PROXIES (weapon, crew) on the last depth pass. >0 proves the proxy actually cast.
// Read by the triShadowProxyVerts verb.

namespace Poseidon
{
static int g_lastShadowProxyVerts = 0;
} // namespace Poseidon
int LastShadowProxyVertCount()
{
    using namespace Poseidon;
    return g_lastShadowProxyVerts;
}
namespace Poseidon
{

// World-space caster-vert cache for STATIC casters. Re-walking every visible
// caster's faces and re-transforming ~450k verts to camera-relative each frame
// cost 4.1 ms CPU in a battle scene; statics never change pose, so their world
// verts are collected once and emitted with a bulk camera subtract (~10× less
// work). Dynamic casters (units) still collect live each frame.
namespace
{
struct StaticCasterCache
{
    OLink<Object> obj; // nulls on delete; also catches address reuse
    int geomLOD = -1;
    Vector3 pos = VZero; // transform fingerprint — destruction rotates/moves
    Vector3 dir = VZero;
    std::vector<float> solidWorld; // xyz world
    struct AlphaChunk
    {
        Texture* tex;
        int first; // vertex index into alphaWorld (5 floats per vertex)
        int count;
    };
    std::vector<AlphaChunk> alphaChunks;
    std::vector<float> alphaWorld; // xyzuv world
    // XP-shadows casterCull: world AABB of every vertex above (only filled
    // while the cull is on).
    bool hasBounds = false;
    float bmin[3] = {0.0f, 0.0f, 0.0f};
    float bmax[3] = {0.0f, 0.0f, 0.0f};
};
std::unordered_map<Object*, StaticCasterCache> s_staticCasterCache;
// Bound the cache footprint: typical shadow range holds a few hundred statics;
// long camera travel accumulates stale entries — drop everything and let the
// next frame rebuild what it actually uses.
constexpr size_t kStaticCasterCacheCap = 1024;
} // namespace

// Shadow-map depth pass — extracted from Scene::DrawObjectsAndShadowsPass2 so
// Scene.cpp stays under the file-size limit.  Collects the visible casters
// (solid + alpha-cutout per texture, camera-relative) plus their proxies, builds
// the tiered omni+frustum cascades, and renders them.  Off by default.
void Scene::RenderShadowMapDepthPass(int nDraw)
{
    namespace sm = Poseidon::shadow;
    extern bool gPerfDumpShadowsOnce;
    extern int gSmDepthCachedCasters;
    const auto depthT0 = std::chrono::steady_clock::now();
    int censusCasters = 0, censusRebuilds = 0, censusCached = 0;
    // Solid casters (opaque) → xyz; alpha-cutout casters (foliage) → xyz+uv
    // grouped per texture so the depth pass can alpha-discard on the caster
    // texture and cast a leaf silhouette instead of a solid quad blob.
    static std::vector<float> smVerts;                                    // solid: 3 floats/vert
    static std::vector<float> smAlphaVerts;                               // alpha: 5 floats/vert (xyz+uv)
    static std::vector<Engine::ShadowCasterBatch> smAlphaBatches;         // per-texture ranges
    static std::unordered_map<Texture*, std::vector<float>> s_alphaByTex; // scratch, reused
    // XP-shadows casterCull: per caster a camera-relative bounding sphere, the
    // solid runs and, per alpha bucket, its runs (first vertex in the bucket,
    // count, caster) for RenderShadowDepthScene to cull per cascade.
    static std::vector<float> smSpheres;
    static std::vector<int> smSolidRuns;
    static std::vector<int> smAlphaRuns;
    static std::vector<int> smAlphaRunStart;
    static std::unordered_map<Texture*, std::vector<int>> s_alphaRunsByTex;
    smVerts.clear();
    smAlphaVerts.clear();
    smAlphaBatches.clear();
    smSpheres.clear();
    smSolidRuns.clear();
    smAlphaRuns.clear();
    smAlphaRunStart.clear();
    g_lastShadowProxyVerts = 0;
    for (auto& kv : s_alphaByTex)
    {
        kv.second.clear();
    }
    for (auto& kv : s_alphaRunsByTex)
    {
        kv.second.clear();
    }
    const Engine::ShadowMapTuning xpTune = GEngine->GetShadowMapTuning();
    const bool xpCull = xpTune.casterCull;
    // Shadow-map far distance: the shadow distance as today, or (XP reach) the
    // view distance's OFP share up to the reach cap and the object distance.
    const float smMapFar = Poseidon::shadow::xp::MapFar(ENGINE_CONFIG.shadowsZ, ENGINE_CONFIG.tacticalZ,
                                                        ENGINE_CONFIG.objectsZ, xpTune.reach);
    const size_t smCacheCap =
        xpTune.reach > 0.0f ? Poseidon::shadow::xp::CasterCacheCap(smMapFar) : kStaticCasterCacheCap;
    if (s_staticCasterCache.size() > smCacheCap)
    {
        s_staticCasterCache.clear();
    }
    static const int s_xpLogEvery = Poseidon::shadow::xp::ReadOverrides().logEvery;
    static int s_xpLogFrame = 0;
    if (s_xpLogEvery > 0 && ++s_xpLogFrame >= s_xpLogEvery)
    {
        s_xpLogFrame = 0;
        gPerfDumpShadowsOnce = true;
    }
    // The current caster's index and its camera-relative AABB (cull only).
    int xpCaster = 0;
    float xpMin[3] = {0.0f, 0.0f, 0.0f};
    float xpMax[3] = {0.0f, 0.0f, 0.0f};
    auto xpGrow = [&](float x, float y, float z)
    {
        xpMin[0] = std::min(xpMin[0], x);
        xpMin[1] = std::min(xpMin[1], y);
        xpMin[2] = std::min(xpMin[2], z);
        xpMax[0] = std::max(xpMax[0], x);
        xpMax[1] = std::max(xpMax[1], y);
        xpMax[2] = std::max(xpMax[2], z);
    };
    // Extend (or open) the current caster's run in an alpha bucket.
    auto xpAlphaRun = [&](Texture* tex, int firstInBucket, int count)
    {
        std::vector<int>& runs = s_alphaRunsByTex[tex];
        const size_t n = runs.size();
        if (n >= 3 && runs[n - 1] == xpCaster && runs[n - 3] + runs[n - 2] == firstInBucket)
        {
            runs[n - 2] += count;
        }
        else
        {
            runs.push_back(firstInBucket);
            runs.push_back(count);
            runs.push_back(xpCaster);
        }
    };
    const int kSkipMask = NoShadow | ShadowDisabled | IsHidden | IsHiddenProxy;
    const int kAlphaMask = IsAlpha | IsTransparent;
    const Vector3 camPos = GetCamera()->Position(); // camera-relative render (PrepareMeshTLImpl)
    // Only collect casters within the shadow-map far distance (= distanceCoef *
    // SHADOW visibility distance); nothing beyond it samples them, so distanceCoef
    // doubles as the caster-collection budget. Pair the reach with
    // ENGINE_CONFIG.shadowsZ — the Game-menu Shadows slider, already clamped to the
    // object distance (no casters live past where objects draw).
    const Engine::ShadowMapTuning smCap = GEngine->GetShadowMapTuning();
    const float smCamNear = GetCamera()->ClipNear();
    const float smShadowsZ = smMapFar;
    const float smShadowFar = smCamNear + smCap.distanceCoef * (smShadowsZ - smCamNear);
    const float smCasterMaxDist2 = Square(smShadowFar);
    // XP-shadows casterMinPx: a caster of radius r at distance d spans
    // r / (d * Top) * 540 pixels (half of 1080 rows); below the threshold it
    // is left out. kMinPx2 = (minPx * Top / 540)^2 compared with r^2 / d^2.
    const float xpMinPxK = xpTune.casterMinPx > 0.0f ? xpTune.casterMinPx * GetCamera()->Top() / 540.0f : 0.0f;
    const float xpMinPxK2 = xpMinPxK * xpMinPxK;
    int xpSmallSkipped = 0;
    // Remastered sun shadows (Graphics/Remaster/SunShadows.hpp): with the
    // treeShadows gate on, every static object casts whatever the legacy
    // object-shadow toggle says.
    remaster::FeatureGates smGates;
    const bool smTreeShadows = GEngine->LightingRemasterActive() && GEngine->GetFeatureGates(smGates) &&
                               smGates[remaster::Feature::TreeShadows].on;

    // Walk one shape's faces in WORLD space, dispatching triangles to the emit
    // callbacks. Used for live (camera-relative stream) and cache (world-space
    // store) collection so both share the classification + triangulation.
    auto walkShape = [&](Shape* s, const FrameBase& frame, auto&& emitSolid, auto&& emitAlpha)
    {
        for (Offset o = s->BeginFaces(); o < s->EndFaces(); s->NextFace(o))
        {
            const Poly& f = s->Face(o);
            sm::CasterMode mode = sm::ClassifyShadowCaster(f.Special(), kSkipMask, kAlphaMask);
            if (mode == sm::CasterMode::Skip)
            {
                continue;
            }
            Texture* tex = f.GetTexture();
            if (mode == sm::CasterMode::AlphaTest && !tex)
            {
                mode = sm::CasterMode::Solid; // no texture to test — cast solid
            }
            int fn = f.N();
            for (int k = 1; k + 1 < fn; k++)
            {
                int i0 = f.GetVertex(0), ia = f.GetVertex(k), ib = f.GetVertex(k + 1);
                Vector3 w0 = frame.PositionModelToWorld(s->Pos(i0));
                Vector3 wa = frame.PositionModelToWorld(s->Pos(ia));
                Vector3 wb = frame.PositionModelToWorld(s->Pos(ib));
                if (mode == sm::CasterMode::Solid)
                {
                    emitSolid(w0, wa, wb);
                }
                else
                {
                    emitAlpha(tex, s, w0, i0, wa, ia, wb, ib);
                }
            }
        }
    };

    // Live (uncached) collection — camera-relative into the stream buffers.
    auto liveSolid = [&](const Vector3& w0, const Vector3& wa, const Vector3& wb)
    {
        auto push = [&](const Vector3& w)
        {
            smVerts.push_back(w.X() - camPos.X());
            smVerts.push_back(w.Y() - camPos.Y());
            smVerts.push_back(w.Z() - camPos.Z());
            if (xpCull)
            {
                xpGrow(w.X() - camPos.X(), w.Y() - camPos.Y(), w.Z() - camPos.Z());
            }
        };
        push(w0);
        push(wa);
        push(wb);
    };
    auto liveAlpha =
        [&](Texture* tex, Shape* s, const Vector3& w0, int i0, const Vector3& wa, int ia, const Vector3& wb, int ib)
    {
        std::vector<float>& buf = s_alphaByTex[tex];
        if (xpCull)
        {
            xpAlphaRun(tex, static_cast<int>(buf.size() / 5), 3);
        }
        auto push = [&](const Vector3& w, int vi)
        {
            buf.push_back(w.X() - camPos.X());
            buf.push_back(w.Y() - camPos.Y());
            buf.push_back(w.Z() - camPos.Z());
            buf.push_back(tex->UToLogical(s->U(vi)));
            buf.push_back(tex->VToLogical(s->V(vi)));
            if (xpCull)
            {
                xpGrow(w.X() - camPos.X(), w.Y() - camPos.Y(), w.Z() - camPos.Z());
            }
        };
        push(w0, i0);
        push(wa, ia);
        push(wb, ib);
    };
    auto collectShape = [&](Shape* s, const FrameBase& frame) { walkShape(s, frame, liveSolid, liveAlpha); };

    // Cached static collection: build the entry's world-space buffers once,
    // emit per frame with a bulk camera-relative subtract.
    auto buildStaticEntry = [&](StaticCasterCache& e, Object* obj, int geomLOD, Shape* s)
    {
        e.obj = obj;
        e.geomLOD = geomLOD;
        e.pos = obj->Position();
        e.dir = obj->Direction();
        e.solidWorld.clear();
        e.alphaChunks.clear();
        e.alphaWorld.clear();
        auto cacheSolid = [&](const Vector3& w0, const Vector3& wa, const Vector3& wb)
        {
            auto push = [&](const Vector3& w)
            {
                e.solidWorld.push_back(w.X());
                e.solidWorld.push_back(w.Y());
                e.solidWorld.push_back(w.Z());
            };
            push(w0);
            push(wa);
            push(wb);
        };
        auto cacheAlpha = [&](Texture* tex, Shape* sh, const Vector3& w0, int i0, const Vector3& wa, int ia,
                              const Vector3& wb, int ib)
        {
            if (e.alphaChunks.empty() || e.alphaChunks.back().tex != tex)
            {
                e.alphaChunks.push_back({tex, static_cast<int>(e.alphaWorld.size() / 5), 0});
            }
            auto push = [&](const Vector3& w, int vi)
            {
                e.alphaWorld.push_back(w.X());
                e.alphaWorld.push_back(w.Y());
                e.alphaWorld.push_back(w.Z());
                e.alphaWorld.push_back(tex->UToLogical(sh->U(vi)));
                e.alphaWorld.push_back(tex->VToLogical(sh->V(vi)));
            };
            push(w0, i0);
            push(wa, ia);
            push(wb, ib);
            e.alphaChunks.back().count += 3;
        };
        walkShape(s, *obj, cacheSolid, cacheAlpha);
        e.hasBounds = false;
        if (xpCull)
        {
            bool first = true;
            auto grow = [&](const float* q)
            {
                for (int a = 0; a < 3; a++)
                {
                    e.bmin[a] = first ? q[a] : std::min(e.bmin[a], q[a]);
                    e.bmax[a] = first ? q[a] : std::max(e.bmax[a], q[a]);
                }
                first = false;
            };
            for (size_t i = 0; i + 2 < e.solidWorld.size(); i += 3)
            {
                grow(e.solidWorld.data() + i);
            }
            for (size_t i = 0; i + 4 < e.alphaWorld.size(); i += 5)
            {
                grow(e.alphaWorld.data() + i);
            }
            e.hasBounds = !first;
        }
    };
    const float camX = camPos.X(), camY = camPos.Y(), camZ = camPos.Z();
    auto emitCachedEntry = [&](const StaticCasterCache& e)
    {
        const size_t base = smVerts.size();
        smVerts.resize(base + e.solidWorld.size());
        const float* src = e.solidWorld.data();
        float* dst = smVerts.data() + base;
        for (size_t i = 0; i < e.solidWorld.size(); i += 3)
        {
            dst[i] = src[i] - camX;
            dst[i + 1] = src[i + 1] - camY;
            dst[i + 2] = src[i + 2] - camZ;
        }
        if (xpCull && e.hasBounds)
        {
            xpGrow(e.bmin[0] - camX, e.bmin[1] - camY, e.bmin[2] - camZ);
            xpGrow(e.bmax[0] - camX, e.bmax[1] - camY, e.bmax[2] - camZ);
        }
        for (const StaticCasterCache::AlphaChunk& chunk : e.alphaChunks)
        {
            std::vector<float>& buf = s_alphaByTex[chunk.tex];
            const size_t b2 = buf.size();
            if (xpCull)
            {
                xpAlphaRun(chunk.tex, static_cast<int>(b2 / 5), chunk.count);
            }
            buf.resize(b2 + static_cast<size_t>(chunk.count) * 5);
            const float* asrc = e.alphaWorld.data() + static_cast<size_t>(chunk.first) * 5;
            float* adst = buf.data() + b2;
            for (int i = 0; i < chunk.count; i++, asrc += 5, adst += 5)
            {
                adst[0] = asrc[0] - camX;
                adst[1] = asrc[1] - camY;
                adst[2] = asrc[2] - camZ;
                adst[3] = asrc[3];
                adst[4] = asrc[4];
            }
        }
    };

    for (int i = 0; i < nDraw; i++)
    {
        SortObject* oi = _drawMergers[i];
        if (!oi->object || !oi->shape)
        {
            continue;
        }
        // Beyond the shadow range nothing samples this caster's depth.
        if (oi->distance2 > smCasterMaxDist2)
        {
            continue;
        }
        if (xpMinPxK2 > 0.0f)
        {
            const float r = oi->shape->BoundingSphere() * oi->object->Scale();
            if (r * r < xpMinPxK2 * oi->distance2)
            {
                xpSmallSkipped++;
                continue;
            }
        }
        // Cast gate INDEPENDENT of the engine's shadow DISTANCE cull. The engine sets
        // shadowLOD = INVISIBLE beyond _shadowFogMaxRange (a near range tuned for the
        // projected path), which would leave the full-VD frustum tiers with no
        // casters. Shadow maps want every real caster the camera can see, so gate
        // only on "this never casts" (model NoShadow flag / CastShadow opt-out);
        // per-face NoShadow / alpha is still handled by ClassifyShadowCaster below.
        if ((oi->shape->Special() & NoShadow) ||
            !remaster::CastsIntoSunShadowMap(oi->object->CastShadow(), oi->object->Static(), smTreeShadows))
        {
            continue;
        }
        // Visible objects cast from their DRAW LOD (silhouette matches the screen,
        // faithful at any distance); an object that is itself off-screen (drawLOD
        // invisible) but whose shadow falls into view — e.g. someone behind the
        // camera — casts from its shadow LOD so the omni tiers still pick it up.
        int geomLOD = (oi->drawLOD != LOD_INVISIBLE) ? oi->drawLOD : oi->shadowLOD;
        if (geomLOD == LOD_INVISIBLE)
        {
            continue;
        }
        // Caster LOD is re-selected at a biased distance: a depth-map texel covers
        // decimetres, so a caster needs far fewer vertices than its screen draw —
        // 250 battle soldiers at the 744-vert LOD dominated the collection (4 ms
        // CPU/frame). Never select finer than the draw LOD, and keep the screen
        // LOD untouched near the camera where the silhouette is texel-resolved.
        if (smCap.casterLodBias > 1.0f)
        {
            const float biased2 = oi->distance2 * Square(smCap.casterLodBias);
            int coarse = LevelFromDistance2(oi->shape, biased2, oi->object->Scale(), oi->object->Direction(),
                                            GetCamera()->Direction());
            if (coarse != LOD_INVISIBLE && coarse > geomLOD)
            {
                geomLOD = coarse;
            }
        }
        const FrameBase& fb = *oi->object;
        Shape* s = oi->shape->LevelOpaque(geomLOD);
        if (!s || s->NPos() <= 0)
        {
            continue;
        }
        censusCasters++;
        const size_t xpSolidStart = smVerts.size();
        if (xpCull)
        {
            xpMin[0] = xpMin[1] = xpMin[2] = 3.0e38f;
            xpMax[0] = xpMax[1] = xpMax[2] = -3.0e38f;
        }
        if (oi->object->Static())
        {
            StaticCasterCache& e = s_staticCasterCache[oi->object];
            if (e.obj != oi->object || e.geomLOD != geomLOD || (e.pos - oi->object->Position()).SquareSize() != 0 ||
                (e.dir - oi->object->Direction()).SquareSize() != 0)
            {
                buildStaticEntry(e, oi->object, geomLOD, s);
                censusRebuilds++;
            }
            else
            {
                censusCached++;
            }
            emitCachedEntry(e);
        }
        else
        {
            collectShape(s, fb);
        }

        // Proxies (the soldier's weapon, vehicle crew, ...) cast from their
        // substituted model — the parent holds only an IsHiddenProxy placeholder
        // (skipped above; that placeholder was the "proxy arrow"). Mirror the
        // projected path's proxy loop (Scene.cpp DrawExShadow): gate on
        // CastProxyShadow, compose the proxy world transform, cast its visible LOD.
        Object* obj = oi->object;
        const size_t solidBeforeProxy = smVerts.size();
        const int proxyCount = obj->GetProxyCount(geomLOD);
        for (int pi = 0; pi < proxyCount; pi++)
        {
            if (!obj->CastProxyShadow(geomLOD, pi))
            {
                continue;
            }
            Matrix4 ptrans = obj->Transform(), pinv = obj->GetInvTransform();
            LODShapeWithShadow* pshape = nullptr;
            Object* proxy = obj->GetProxy(pshape, geomLOD, ptrans, pinv, *obj, pi);
            if (!proxy || !pshape)
            {
                continue;
            }
            const float pdist2 = ptrans.Position().Distance2(camPos);
            int plevel =
                LevelFromDistance2(pshape, pdist2, ptrans.Scale(), ptrans.Direction(), GetCamera()->Direction());
            if (plevel == LOD_INVISIBLE)
            {
                continue;
            }
            Shape* ps = pshape->LevelOpaque(plevel);
            if (!ps || ps->NPos() <= 0)
            {
                continue;
            }
            FrameWithInverse pframe(ptrans, pinv);
            collectShape(ps, pframe);
        }
        g_lastShadowProxyVerts += static_cast<int>((smVerts.size() - solidBeforeProxy) / 3);
        if (xpCull)
        {
            // One sphere per caster (radius 0 at the origin when it emitted
            // nothing), then its solid run.
            if (xpMax[0] >= xpMin[0])
            {
                const float hx = 0.5f * (xpMax[0] - xpMin[0]);
                const float hy = 0.5f * (xpMax[1] - xpMin[1]);
                const float hz = 0.5f * (xpMax[2] - xpMin[2]);
                smSpheres.push_back(xpMin[0] + hx);
                smSpheres.push_back(xpMin[1] + hy);
                smSpheres.push_back(xpMin[2] + hz);
                smSpheres.push_back(std::sqrt(hx * hx + hy * hy + hz * hz) * 1.001f + 0.01f);
            }
            else
            {
                smSpheres.insert(smSpheres.end(), {0.0f, 0.0f, 0.0f, 0.0f});
            }
            const int solidFirst = static_cast<int>(xpSolidStart / 3);
            const int solidCount = static_cast<int>((smVerts.size() - xpSolidStart) / 3);
            if (solidCount > 0)
            {
                smSolidRuns.push_back(solidFirst);
                smSolidRuns.push_back(solidCount);
                smSolidRuns.push_back(xpCaster);
            }
            xpCaster++;
        }
    }
    gSmDepthCachedCasters = censusCached;
    // Flatten the per-texture alpha buckets into one buffer + batch ranges.
    for (auto& kv : s_alphaByTex)
    {
        if (kv.second.empty())
        {
            continue;
        }
        Engine::ShadowCasterBatch batch;
        batch.texture = kv.first;
        batch.firstVertex = static_cast<int>(smAlphaVerts.size() / 5);
        batch.vertexCount = static_cast<int>(kv.second.size() / 5);
        smAlphaVerts.insert(smAlphaVerts.end(), kv.second.begin(), kv.second.end());
        smAlphaBatches.push_back(batch);
        if (xpCull)
        {
            smAlphaRunStart.push_back(static_cast<int>(smAlphaRuns.size() / 3));
            const std::vector<int>& runs = s_alphaRunsByTex[kv.first];
            smAlphaRuns.insert(smAlphaRuns.end(), runs.begin(), runs.end());
        }
    }
    if (xpCull)
    {
        smAlphaRunStart.push_back(static_cast<int>(smAlphaRuns.size() / 3));
    }
    // Cascaded shadow maps fit in camera-relative space, snapped to a world grid
    // (ShadowMath::BuildShadowCascadesTiered, proven by [ShadowMath] unit tests), so
    // the shadow stays world-anchored (no crawl/jump) while the casters stay
    // camera-relative.
    const Engine::ShadowMapTuning tune = GEngine->GetShadowMapTuning();
    const int smRes = tune.resolution;
    const Camera* cam = GetCamera();
    Vector3Val cF = cam->Direction();
    Vector3Val cR = cam->DirectionAside();
    Vector3Val cU = cam->DirectionUp();
    // Classic keeps the shadow direction at least 11.5 degrees above the
    // horizon (LightSun::Recalculate). The remastered lighting shades with the
    // true sun, so its shadows follow it down to 4 degrees: full-length evening
    // shadows that line up with the lit and shaded sides of every object. At
    // night the remastered moon key (moonShadows) casts them from the moon.
    Vector3 sd = _mainLight->ShadowDirection();
    if (GEngine->LightingRemasterActive())
    {
        constexpr float kMinShadowSinElevation = 0.0698f; // sin(4 deg)
        sd = GEngine->RemasterMoonKey() ? _mainLight->MoonDirection() : _mainLight->SunDirection();
        if (sd.Y() > -kMinShadowSinElevation)
        {
            const float horiz = std::sqrt(std::max(sd.X() * sd.X() + sd.Z() * sd.Z(), 1e-8f));
            const float cosE = std::sqrt(1.0f - kMinShadowSinElevation * kMinShadowSinElevation);
            sd = Vector3(sd.X() / horiz * cosE, -kMinShadowSinElevation, sd.Z() / horiz * cosE);
        }
    }

    sm::CascadeBuildParams bp;
    bp.camPos = {camPos.X(), camPos.Y(), camPos.Z()};
    bp.forward = {cF.X(), cF.Y(), cF.Z()};
    bp.right = {cR.X(), cR.Y(), cR.Z()};
    bp.up = {cU.X(), cU.Y(), cU.Z()};
    bp.tanHalfX = cam->Left(); // tan(hfov/2)
    bp.tanHalfY = cam->Top();  // tan(vfov/2)
    bp.nearD = cam->ClipNear();
    // Pair the cascade reach with the shadow visibility distance (Game-menu Shadows
    // slider, clamped to the object distance), not the camera far-clip: the frustum
    // tiers cover exactly as far as shadows are meant to render.
    bp.farD = smMapFar;
    bp.sunDir = {sd.X(), sd.Y(), sd.Z()};
    bp.count = tune.cascadeCount;
    bp.distanceCoef = tune.distanceCoef;
    bp.splitCoef = tune.splitCoef;
    bp.resolution = smRes;
    bp.zPad = 50.0f;
    bp.omniCount = tune.omniCount;
    bp.omniCoef[0] = tune.omniCoef0;
    bp.omniCoef[1] = tune.omniCoef1;
    bp.tightSphere = tune.tightSphere;
    const sm::CascadeSet cs = sm::BuildShadowCascadesTiered(bp);
    if (gPerfDumpShadowsOnce)
    {
        const double collectMs =
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - depthT0).count();
        LOG_INFO(Graphics,
                 "PERF smdepth: {} casters ({} cached, {} rebuilt), {} solid + {} alpha verts collected in {:.2f} "
                 "ms CPU; map far {:.0f} m, res {}, cascades {} (omni {}), cache {} entries, cull {}, small skipped {}",
                 censusCasters, censusCached, censusRebuilds, (int)(smVerts.size() / 3), (int)(smAlphaVerts.size() / 5),
                 collectMs, smMapFar, smRes, cs.count, cs.omniCount, s_staticCasterCache.size(), xpCull ? 1 : 0,
                 xpSmallSkipped);
        gPerfDumpShadowsOnce = false;
    }

    float vps[4 * 16];
    for (int c = 0; c < cs.count; c++)
    {
        memcpy(vps + c * 16, cs.camRelVP[c].m.data(), sizeof(float) * 16);
    }
    const float camFwd[3] = {cF.X(), cF.Y(), cF.Z()};
    Engine::ShadowCasterSet casterSet;
    casterSet.solidXYZ = smVerts.data();
    casterSet.solidVertexCount = static_cast<int>(smVerts.size() / 3);
    casterSet.alphaXYZUV = smAlphaVerts.data();
    casterSet.alphaVertexCount = static_cast<int>(smAlphaVerts.size() / 5);
    casterSet.alphaBatches = smAlphaBatches.data();
    casterSet.alphaBatchCount = static_cast<int>(smAlphaBatches.size());
    if (xpCull)
    {
        casterSet.casterSpheres = smSpheres.data();
        casterSet.casterCount = static_cast<int>(smSpheres.size() / 4);
        casterSet.solidRuns = smSolidRuns.data();
        casterSet.solidRunCount = static_cast<int>(smSolidRuns.size() / 3);
        casterSet.alphaRuns = smAlphaRuns.data();
        casterSet.alphaRunStart = smAlphaRunStart.data();
    }
    GEngine->RenderShadowDepthScene(vps, cs.splitViewDist, camFwd, cs.count, cs.omniCount, smRes, casterSet);
}

} // namespace Poseidon
