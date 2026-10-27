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
//
// Local lights: the batched draw lights every member with the head's light
// list. With drawGrouping off, any light slot in the scene keeps every object
// out of runs (Scene::NLights() != 0; Abel's street lamps hold slots by day
// too). With it on, the rule is per object: an object may head or join a run
// only when the light list its own draw would select is empty (localLit
// false), so the head's empty list is right for every member. While the
// frame's tiled light list lights every draw (lightTiles,
// Engine::LocalLightsFromTiles), no draw's own list is read and the caller
// leaves localLit false for every object.

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
    // Scene::SelectLights for the object's own draw returns a light: its draw
    // would be lit by a local light (a lamp, a headlight, a fire). False when
    // the scene has no light slot, by day (SelectLights selects none then) and
    // while the tiled light list lights every draw.
    bool localLit = false;
};

// Members stay within this factor of the head's squared distance (about 5% in
// distance), so the head's per-object constant fog stands for all of them.
constexpr float kFogBandLo = 0.90f;
constexpr float kFogBandHi = 1.10f;

// May this object head a run?  Not when its draw rewrites its own vertices:
// the instanced draw would repeat that object's fit at every member.  Not
// when a local light lights it: the run draws with the head's light list, and
// only an empty one is right for every member.
constexpr bool CanHead(const RunEntry& head)
{
    return head.isStatic && !head.ownVertices && !head.localLit;
}

// May `member` join the run `head` leads?  Same shape, level, pass and render
// flags, inside the fog band, drawn from the shape's own vertices and lit by
// no local light.  A member whose draw rewrites them (a bush fitted to the
// slope it stands on) would otherwise be drawn with the head's fit instead of
// its own; a lit member would lose its light (the head's list is empty).
constexpr bool CanJoin(const RunEntry& head, const RunEntry& member)
{
    return member.shape == head.shape && member.drawLOD == head.drawLOD && member.passNum == head.passNum &&
           member.isStatic && member.special == head.special && member.distance2 >= head.distance2 * kFogBandLo &&
           member.distance2 <= head.distance2 * kFogBandHi && !member.ownVertices && !member.localLit;
}

} // namespace Poseidon::InstancedRun
