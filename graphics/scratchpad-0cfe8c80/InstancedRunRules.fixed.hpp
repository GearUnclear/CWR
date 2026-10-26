#pragma once

namespace Poseidon::InstancedRun
{

// The opaque pass (Scene::DrawObjectsAndShadowsPass1) draws a run of identical
// static shapes as one instanced draw: the run's first object (the head) is
// drawn once and every TL section repeats it with each member's world matrix.
// Everything else in that draw (vertex buffer, textures, constant fog, light
// list) is the head's, so an object may join only where the head's draw is
// also right for it. These are the per-object rules; the caller adds the scene
// and routing gates (local lights, proxies, OnSurface, IsColored, the camera
// object).

// What the grouping reads from one queued object.  POD; no engine dependencies.
struct RunEntry
{
    const void* shape; // LODShape identity
    int drawLOD;       // draw level
    int passNum;       // Object::PassNum
    int special;       // draw level Special() | object GetObjSpecial()
    float distance2;   // squared camera distance
    bool isStatic;     // Object::Static()
    // Object::IsAnimated(drawLOD): the object's draw rewrites the shared level's
    // vertices for this object alone (land-following ClipLandKeep / ClipLandOn
    // points fitted to the terrain under it, damage, destruction, entity
    // animation) and restores them afterwards.
    bool ownVertices;
};

// Members stay within this factor of the head's squared distance (about 5% in
// distance), so the head's per-object constant fog stands for all of them.
constexpr float kFogBandLo = 0.90f;
constexpr float kFogBandHi = 1.10f;

// May this object head a run?  Not when its draw rewrites its own vertices:
// the instanced draw would repeat that object's fit at every member.
constexpr bool CanHead(const RunEntry& head)
{
    return head.isStatic && !head.ownVertices;
}

// May `member` join the run `head` leads?  Same shape, level, pass and render
// flags, inside the fog band, and drawn from the shape's own vertices.  A
// member whose draw rewrites them (a bush fitted to the slope it stands on)
// would otherwise be drawn with the head's fit instead of its own.
constexpr bool CanJoin(const RunEntry& head, const RunEntry& member)
{
    return member.shape == head.shape && member.drawLOD == head.drawLOD && member.passNum == head.passNum &&
           member.isStatic && member.special == head.special && member.distance2 >= head.distance2 * kFogBandLo &&
           member.distance2 <= head.distance2 * kFogBandHi && !member.ownVertices;
}

} // namespace Poseidon::InstancedRun
