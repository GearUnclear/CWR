// Batched decals (perf W4-1).
//
// Every decal (Object::DrawDecal: smoke, cloud and fire puffs, a light's
// flare; EngineGL33::DrawDecal) is one screen quad with its own texture. The
// alpha queue took one texture at a time, so a texture change ended a draw:
// with the animated puffs (the fire's 48 frames, the smoke's 11) nearly every
// puff was its own draw, about 5 to 7 us of GPU time each (perf survey M2
// section 6; with one texture for every puff, EXP-P3, the draws merged and
// the town in rain lost 2.2 ms of GPU time, the fires 3.1).
//
// Here each decal texture is copied once (glCopyImageSubData, the GPU's own
// texels, no decode) into a layer of a texture array shared by every texture
// of its surface shape (size, mips, format: DecalBatchRules.hpp), the layer
// rides in the quad's second texture coordinate, and the alpha queue keeps one
// batch open across texture changes: consecutive decals of one spec and one
// array are one draw, which the remastered object program (psNormalRemaster,
// decalLayerMode) samples from the array with the same sampler object the
// texture had on unit 0. Nothing is reordered: the batch takes decals only in
// the order they come, and one draw blends its triangles in index order, as
// the separate draws did. A program that cannot read the array (the classic
// lighting set) draws the batch as its runs of one texture, the old draws.
//
// What still ends a batch: another spec, another array (a texture of another
// shape), a texture that cannot join (no surface yet, interpolated or updated
// in place, a full array), a different wet-weather or interior surface class,
// and anything that ended the old queue (a 3D draw, a full vertex buffer).
//
// POSEIDON_DECAL_BATCH=0 (read once, at the first decal) keeps the old path.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <PoseidonGL33/TextureGL33.hpp>
#include <Poseidon/Graphics/Core/FanDecompose.hpp>
#include <Poseidon/Graphics/Rendering/RenderFlags.hpp>

#include <glad/gl.h>

#include <cstdint>
#include <cstdlib>

extern void InitGLPixelFormat(TextureDescGL33& desc, Poseidon::PacFormat format, bool enableDXT);

namespace db = Poseidon::render::decalbatch;

namespace
{
// Indices of one decal quad (a fan of four).
constexpr int kDecalIndices = Poseidon::render::geom::FanTriangleIndexCount(4);

int MipExtent(int size, int level)
{
    const int s = size >> level;
    return s > 0 ? s : 1;
}
} // namespace

bool EngineGL33::DecalBatchOn()
{
    if (_decalBatchGate < 0)
    {
        const char* env = std::getenv("POSEIDON_DECAL_BATCH");
        const bool envOff = env && env[0] == '0' && env[1] == '\0';
        const bool copyImage = (GLAD_GL_ARB_copy_image || GLAD_GL_VERSION_4_3) && glCopyImageSubData &&
                               (GLAD_GL_ARB_texture_storage || GLAD_GL_VERSION_4_2) && glTexStorage3D;
        _decalBatchGate = (!envOff && copyImage) ? 1 : 0;
        LOG_INFO(Graphics, "GL33: decal batching {}{}", _decalBatchGate ? "on" : "off",
                 envOff ? " (POSEIDON_DECAL_BATCH=0)"
                        : (copyImage ? "" : " (no ARB_copy_image or ARB_texture_storage)"));
    }
    return _decalBatchGate == 1;
}

int EngineGL33::DecalStateClass(TextureGL33* tex, int specFlags)
{
    // The texture-derived state ApplyPassState sets from the queue's texture:
    // the wet weather's surface class (SetLitSurface) and a vehicle interior's
    // glass mark (InteriorGlassStencil). Outside both every texture is equal.
    if (!((_lightingActive && _wetShading) || FogInteriorOpen()))
        return -1;
    EnsureSurfaceTable();
    return static_cast<int>(tex->SurfaceClassFor(_surfaceTable, _surfaceTableGen, GetDrawObjectKind(),
                                                 Poseidon::render::IsOnSurfaceSpec(specFlags)));
}

EngineGL33::DecalLayerSlot EngineGL33::PrepareDecalLayer(TextureGL33* tex)
{
    DecalLayerSlot slot;
    if (!tex || !_glContext || !DecalBatchOn())
        return slot;
    // Only the world's draws: inside the sky's own pass the object program
    // reads the material texture beyond the fetch it batches (the dome's
    // cubic filter, the cloud sheets' march), which a layer would not feed.
    if (LitSkyMode() != 0)
        return slot;
    // Only what the alpha queue takes (it draws in submission order); the
    // opaque reorder queues keep their own path.
    if (!(tex->IsAlpha() || !_enableReorder))
        return slot;
    // A texture whose surface can change without becoming another surface
    // (interpolated, updated in place) never joins: its layer could go stale.
    if (!tex->_src || tex->_interpolate || tex->_dynamicMipmapped)
        return slot;
    const SurfaceInfoGL33& surf = tex->GetSurface();
    const unsigned int handle = surf.GetTexture();
    if (!handle)
        return slot;
    TextureDescGL33 fmt = {};
    InitGLPixelFormat(fmt, surf._format, true);
    db::SurfaceShape shape;
    shape.w = surf._w;
    shape.h = surf._h;
    shape.levels = surf._nMipmaps;
    shape.internalFormat = fmt.internalFormat;
    shape.swizzleAI = surf._format == Poseidon::PacAI88;
    shape.bytes = surf._totalSize;
    db::SurfaceStamp stamp;
    stamp.handle = handle;
    stamp.creationId = surf.GetCreationID();

    const db::LayerRef ref = _decalLayers.Acquire(reinterpret_cast<std::uintptr_t>(tex), shape, stamp);
    if (!ref.Valid())
        return slot;
    if (ref.created || ref.growFrom > 0 || ref.copy)
    {
        // The open batch may draw this array's layers: it draws first, with
        // what they hold now.
        const int qi = MaxTriQueues - 1;
        if (_decalBatch.active && _decalBatch.key.array == ref.array && _queueNo._triUsed[qi] &&
            _queueNo._tri[qi]._triangleQueue.Size() > 0)
            FlushQueue(_queueNo, qi);
        if (!EnsureDecalArray(ref.array, ref.capacity, ref.growFrom) ||
            (ref.copy && !CopyDecalLayer(ref.array, ref.layer, tex)))
        {
            _decalLayers.Forget(reinterpret_cast<std::uintptr_t>(tex));
            return slot;
        }
    }
    slot.array = ref.array;
    slot.layer = ref.layer;
    return slot;
}

bool EngineGL33::EnsureDecalArray(int array, int capacity, int growFrom)
{
    if (array < 0 || array >= _decalLayers.ArrayCount() || capacity <= 0)
        return false;
    if (array >= static_cast<int>(_decalArrayTex.size()))
    {
        _decalArrayTex.resize(array + 1, 0u);
        _decalArrayCap.resize(array + 1, 0);
    }
    const GLuint old = _decalArrayTex[array];
    const int oldCap = _decalArrayCap[array];
    if (old && oldCap >= capacity)
        return true;
    const db::SurfaceShape& shape = _decalLayers.ArrayAt(array).shape;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    if (!tex)
        return false;
    // Created on the upload unit (its 2D binding is the loaders'; the array
    // target beside it is ours), as the texture surfaces are.
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, shape.levels, shape.internalFormat, shape.w, shape.h, capacity);
    if (glGetError() != GL_NO_ERROR)
    {
        LOG_ERROR(Graphics, "GL33: decal array {}x{} mips={} fmt=0x{:04X} x{} layers: storage failed", shape.w, shape.h,
                  shape.levels, shape.internalFormat, capacity);
        glDeleteTextures(1, &tex); // also unbinds it from the upload unit
        GL33Bind::ActiveUnit(0);
        return false;
    }
    // The texture surfaces' own parameters (SurfaceInfoGL33::CreateSurface);
    // the sampler object bound beside it overrides the filter and wrap, as it
    // does for the textures on unit 0.
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, shape.levels - 1);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (shape.swizzleAI)
    {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_R, GL_RED);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_B, GL_RED);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_A, GL_GREEN);
    }
    // A grown array keeps the layers it had.
    const int keep = growFrom > 0 && old ? (oldCap < capacity ? oldCap : capacity) : 0;
    for (int level = 0; keep > 0 && level < shape.levels; ++level)
        glCopyImageSubData(old, GL_TEXTURE_2D_ARRAY, level, 0, 0, 0, tex, GL_TEXTURE_2D_ARRAY, level, 0, 0, 0,
                           MipExtent(shape.w, level), MipExtent(shape.h, level), keep);
    // The array stays bound on the upload unit's array target, as a loaded
    // surface stays on its 2D target there (no program samples unit 7), and
    // the texture-array target is unbound only by the terrain horizon upload
    // (test_gl_state_cache_audit).
    if (old)
    {
        // Deleting the grown array unbinds it from unit 16 if a batch left it
        // there: the new one takes its place, complete.
        glDeleteTextures(1, &old);
        GL33Bind::ActiveUnit(Poseidon::gl33lighting::kDecalLayerUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    }
    GL33Bind::ActiveUnit(0);
    _decalArrayTex[array] = tex;
    _decalArrayCap[array] = capacity;
    return true;
}

bool EngineGL33::CopyDecalLayer(int array, int layer, const TextureGL33* tex)
{
    if (array < 0 || array >= static_cast<int>(_decalArrayTex.size()) || !_decalArrayTex[array])
        return false;
    const SurfaceInfoGL33& surf = tex->GetSurface();
    const db::SurfaceShape& shape = _decalLayers.ArrayAt(array).shape;
    for (int level = 0; level < shape.levels; ++level)
        glCopyImageSubData(surf.GetTexture(), GL_TEXTURE_2D, level, 0, 0, 0, _decalArrayTex[array], GL_TEXTURE_2D_ARRAY,
                           level, 0, 0, layer, MipExtent(shape.w, level), MipExtent(shape.h, level), 1);
    // Once per texture (and again only when it changes surface): the check
    // costs nothing that matters here.
    if (glGetError() != GL_NO_ERROR)
    {
        LOG_ERROR(Graphics, "GL33: decal layer copy failed ({} into array {} layer {})",
                  static_cast<const char*>(tex->GetName()), array, layer);
        return false;
    }
    return true;
}

void EngineGL33::QueuePrepareDecalBatch(const Poseidon::MipInfo& mip, int specFlags, const DecalLayerSlot& slot)
{
    TextureGL33* tex = reinterpret_cast<TextureGL33*>(mip._texture);
    const int qi = MaxTriQueues - 1;
    // As AllocateQueue for an alpha texture: the reorder queues draw first.
    FlushAllQueues(_queueNo, qi);
    db::BatchKey key;
    key.spec = specFlags;
    key.array = slot.array;
    key.stateClass = DecalStateClass(tex, specFlags);
    if (_queueNo._triUsed[qi])
    {
        TriQueue& triq = _queueNo._tri[qi];
        if (_decalBatch.active && db::Joins(_decalBatch.key, key))
        {
            // A full index queue draws what it holds and the batch goes on.
            if (triq._triangleQueue.Size() + kDecalIndices > TriQueueSize)
                FlushQueue(_queueNo, qi);
            _queueNo._usedCounter++;
            triq._lastUsed = _queueNo._usedCounter;
            saturateMin(triq._level, mip._level);
            _decalBatch.runs.Add(reinterpret_cast<std::uintptr_t>(tex), triq._triangleQueue.Size(), kDecalIndices);
            _queueNo._actTri = qi;
            return;
        }
        // Another key ends the batch (or a queue of another alpha texture is
        // open, which draws as before).
        FlushAndFreeQueue(_queueNo, qi);
    }
    const int index = _queueNo.Allocate(tex, mip._level, specFlags, qi, MaxTriQueues, qi);
    PoseidonAssert(index == qi);
    _decalBatch.active = true;
    _decalBatch.key = key;
    _decalBatch.runs.Clear();
    _decalBatch.runs.Add(reinterpret_cast<std::uintptr_t>(tex), 0, kDecalIndices);
    _queueNo._actTri = index;
}

void EngineGL33::DrawDecalBatch(const TriQueue& triq, int n, int indexOffset)
{
    // The program ApplyPassState bound: the remastered object program reads
    // the array while decalLayerMode is 1 (set for this draw only).
    int loc = -1;
    if (_surfBoundProgram && _pixelShaderSel == PSNormal)
    {
        if (_surfBoundProgram >= _decalModeLoc.size())
            _decalModeLoc.resize(_surfBoundProgram + 64, -2);
        int& cached = _decalModeLoc[_surfBoundProgram];
        if (cached == -2)
            cached = glGetUniformLocation(_surfBoundProgram, "decalLayerMode");
        loc = cached;
    }
    const int array = _decalBatch.key.array;
    const GLuint arrayTex = array >= 0 && array < static_cast<int>(_decalArrayTex.size()) ? _decalArrayTex[array] : 0u;
    if (loc >= 0 && arrayTex)
    {
        // The sampler object ApplySamplerState bound on unit 0 for this spec.
        const int bits = (_pointSampling ? 4 : 0) | (_lastClampU ? 1 : 0) | (_lastClampV ? 2 : 0);
        GL33Bind::ActiveUnit(Poseidon::gl33lighting::kDecalLayerUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, arrayTex);
        glBindSampler(Poseidon::gl33lighting::kDecalLayerUnit, MaterialSampler(bits));
        GL33Bind::ActiveUnit(0);
        glUniform1f(loc, 1.0f);
        glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_SHORT, (void*)(intptr_t)(indexOffset * sizeof(WORD)));
        glUniform1f(loc, 0.0f);
        ++Poseidon::gPerfDrawCalls;
        return;
    }
    // No array reader: one draw per run of one texture, the draws of before.
    const Poseidon::render::LegacySpec spec = Poseidon::render::SplitLegacy(triq._special);
    int drawn = 0;
    for (int r = 0; r < _decalBatch.runs.Size(); ++r)
    {
        const db::RunList::Run& run = _decalBatch.runs[r];
        if (run.first + run.count > n)
            break;
        TextureGL33* tex = reinterpret_cast<TextureGL33*>(run.tex);
        const unsigned int handle = tex ? tex->GetHandle() : 0;
        if (!GL33Bind::IsTexBound(0, handle))
            SetTexture(tex, spec);
        glDrawElements(GL_TRIANGLES, run.count, GL_UNSIGNED_SHORT,
                       (void*)(intptr_t)((indexOffset + run.first) * sizeof(WORD)));
        ++Poseidon::gPerfDrawCalls;
        drawn = run.first + run.count;
    }
    if (drawn < n)
    {
        glDrawElements(GL_TRIANGLES, n - drawn, GL_UNSIGNED_SHORT,
                       (void*)(intptr_t)((indexOffset + drawn) * sizeof(WORD)));
        ++Poseidon::gPerfDrawCalls;
    }
}

void EngineGL33::DestroyDecalArrays()
{
    for (unsigned int& tex : _decalArrayTex)
        if (tex)
        {
            glDeleteTextures(1, &tex);
            tex = 0;
        }
    _decalArrayTex.clear();
    _decalArrayCap.clear();
    // Unit 16 gets its stand-in back (InitLightingResources), if it still lives.
    if (_dummyHorizonTex)
    {
        GL33Bind::ActiveUnit(Poseidon::gl33lighting::kDecalLayerUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, _dummyHorizonTex);
        GL33Bind::ActiveUnit(0);
    }
    _decalLayers.Clear();
    _decalBatch.active = false;
    _decalBatch.runs.Clear();
    _decalModeLoc.clear();
}
