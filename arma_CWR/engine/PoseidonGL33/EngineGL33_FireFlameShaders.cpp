// The fire spread's flame sheets (POSEIDON_XP_FIRESPREAD): their shaders.
// EngineGL33_FireFlames.cpp draws them; kept in a file of their own (no
// EngineGL33 members here) so the shader tests can read them alone.
//
// Two variants: the flipbook (simulated flames in four styles, physical
// fields from tools/firespread/gen_flame_flipbook.py, lit here as black
// bodies and a soot medium, played per sheet) and the procedural flame, the
// fallback when no flipbook file loads. The vertex shader takes the variant's
// quad shape from FLAME_FLIPBOOK; the fragment shaders share their inputs,
// the soft fade against the scene's depth and the output (linear light into
// the flame target, or coded into the frame without it). The merge's shader
// lays that target onto the decoded frame in the post pass.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <algorithm>
#include <cmath>
#include <string>

namespace Poseidon::gl33lighting
{

namespace
{

// The vertex shader: one instance per sheet, kFlameRows rows of two vertices
// as a triangle strip (EngineGL33_FireFlames.cpp kFlameStripVertices).
const char kFireFlameVertexGLSL[] = R"(
// The fire spread's flames (EngineGL33_FireFlames.cpp). The flipbook's: one
// flame tongue per FlameSheet, an axial billboard: a ribbon along the
// tongue's bent axis (foot sunk kFlameSink into the ground, the axis
// standing up from the fuel and bending over downwind), as wide as its frame,
// turned about that axis to face the eye, so it never shows a plane from any
// side. The procedural fallback's: an upright quad turned about the vertical.
uniform mat4 flameView; // the world's view (rotation only: positions are camera-relative)
uniform mat4 flameProj; // the world's projection (no z bias)
uniform vec4 flameCam;  // xy the TAA jitter (NDC), z fog start (m), w fog 1 / range (0 = no fog)

layout(location = 0) in vec4 aFoot;  // xyz the foot on the ground, camera-relative (m), w the width (m)
layout(location = 1) in vec4 aShape; // x height (m), yz the top's lean downwind (m), w heat (0..1)
layout(location = 2) in vec4 aSeed;  // x the sheet's seed (0..1), y the flipbook's frame (0..128), z its style (0..3),
                                     // w the fine detail's climb (its noise cells, 0..289)
layout(location = 3) in vec4 aOptic; // x the luminous zone's emissivity at its densest, seen side on (0..1), y its
                                     // axis' bend (the power, 0: kFlameBend), z its foot's fade (m, 0:
                                     // kFlipBaseFade), w its light's temperature offset (K)
layout(location = 4) in vec4 aVolume; // a big tongue drawn as a volume (FlameSheet::volumeK): x its visible
                                      // absorption (1/m; 0: a ribbon), y its radius' scale, z its tips' soot (0..1),
                                      // w its puff's bulge (VolumePuffCode; 0: none)
// Far flames (fix 08): x the level of detail's gain (FireSpreadWorld.hpp
// FlameSheet::gain: the light of the tongues it stands for over its own),
// yzw spare. Location 5 (fix 6's aVolume is 4).
layout(location = 5) in vec4 aFar;
// x half the flame target's height (pixels: with flameProj[1][1] and a
// distance, a metre's size there), y the near flipbook frame's height in
// texels, z its width, w spare.
uniform vec4 flameScreen;

out vec3 vSheet;       // x across the quad (-1..1), y up the flame from the ground (m; the flipbook's along its axis), z from the sunk foot (m)
flat out vec4 vFlame;  // x half the quad's width (m), y the flame's height (m; the flipbook's its length, foot to tip), z heat, w seed
flat out float vPhase; // the flipbook's frame (the backend plays it per sheet)
flat out float vStyle; // its style (FireSpreadWorld.hpp FlameStyle)
flat out float vScroll; // the fine detail's climb (the backend plays it per sheet)
flat out float vEmis;   // the luminous zone's emissivity at its densest, seen side on
flat out float vBaseFade; // its foot's fade (m; 0: the default)
flat out vec2 vTemp;      // x its light's temperature offset (K), y 1 for a flame standing on its fuel (0 one carried on its axis)
out float vGain;       // the optical depth along the sight line over the side-on one (1 side on, more toward end on)
out float vFogKeep;    // the share of the flame the classic fog keeps (1 = no fog)
out vec3 vRel;         // the point, camera-relative (m): the flame's near fade is worked out per pixel
// A volume tongue (flameVolumeQuad; 0 in vVolLat.w: a ribbon): its foot (camera-relative) and its frame's
// length along its axis, its axis and its length to the tip, the frame's across (square to the axis, facing
// the eye) and its absorption, and x its reach round the axis, y its depth's half thickness at the foot, z
// its tips' soot (all m but z).
flat out vec4 vVolFoot;
flat out vec4 vVolAxis;
flat out vec4 vVolLat;
flat out vec4 vVolDims;
flat out vec4 vFar;    // x the level of detail's gain, y the splat's share (0 the ribbon .. 1 the splat), z the far
                       // book's share (0 the near book .. 1 the far), w the ribbon's area (flame-target pixels^2)
out vec2 vSplat;       // across and up the splat (-1..1 each way: its tent)

#ifdef FLAME_FLIPBOOK
// The flipbook's frame holds the flame's tip at kFlipFlameShare of its height
// (per style, as measured: FireSpreadWorld.hpp kFlameTipShare; psFireFlames'
// kFlipFlameShare is the same) and its pockets and soot above.
const vec4 kFlipFlameShare = vec4(0.551, 0.566, 0.523, 0.543);
const float kFlameSink = 0.35; // the foot goes this far into the ground (m): no gap on a slope
const float kPlumeLean = 0.35; // above the flame's tip the axis leans at this share of its slope there
// Seen along its axis (the eye down a leaning tongue's lean) a tongue is a
// round body one tongue wide, not a sliver: from kEndOnFrom (the sine of the
// angle between the sight line and the axis, 20 degrees) the ribbon squashes
// toward a blob kBlobTall of its width tall on screen, all of it by kEndOnTo
// (10 degrees); the wildfire consult, 2026-11-16, third round.
const float kEndOnFrom = 0.342;
const float kEndOnTo = 0.174;
const float kBlobTall = 0.6;
// The sight line crosses a tongue's thickness over sin(angle), so its optical
// depth grows toward end on, at most kFlameGainMax times the side-on one.
const float kFlameGainMax = 2.0;
// ---- Far flames (fix 08; the realism audit's C13) ----
// The near flipbook's mips average its kelvin with the air round the tongue
// before the luminous zone's threshold, so a far tongue lost its light (87%
// kept at mip 2, 66 to 71% at 3, 15 to 44% at 4, none from 5: fix-08
// tools\miploss.py): from kFarFromLod to kFarToLod of the near book's mip a
// tongue goes over to the far book, whose fields are linear in the light, so
// its mips keep it exactly. Under kSplatFullPx (its ribbon's size, the
// square root of its area in flame-target pixels) a tongue is a splat (from
// kSplatNonePx): a square kSplatRadiusPx each way of the target's pixels,
// square to the eye, its light a tent that sums to the tongue's whole light
// over the pixel centres wherever it falls, so a far flame never crawls or
// vanishes as it moves. It stands at the tongue's light centroid
// (kFarCentroid of its frame up its axis, per style, as fix-08
// tools\splatconst.py measures it), raised in a level view until its bottom
// edge is no lower than the tongue's foot (the ground in front of a far
// flame would otherwise cut its lower half, and more or less of it as it
// moves).
const float kFarFromLod = 1.5;
const float kFarToLod = 2.5;
const float kSplatFullPx = 3.0;
const float kSplatNonePx = 6.0;
const float kSplatRadiusPx = 1.0;
const vec4 kFarCentroid = vec4(0.143, 0.161, 0.134, 0.149);

// The far flame's state (vFar): how much it is a splat and how much it reads
// the far book, from its ribbon's size in the flame target's pixels at its
// middle `mid` (camera-relative), frameH its frame's height and halfW its
// half width (m). (fix 6's volume path takes a tongue only while
// flameFarSplat is 0: a far one is a splat.)
float flameFarPxPerM(vec3 at) { return flameScreen.x * flameProj[1][1] / max(length(at), 0.1); }
// The ribbon's shown length is its frame's times the sine of its angle to
// the sight line, at least kSplatEndOnShare (end on it is a blob kBlobTall
// of its width, about that share of its frame): vFar.w, the splat's area,
// is the ribbon's as shown, so a splat seen from above carries what the
// ribbon would.
const float kSplatEndOnShare = 0.3;
vec2 flameFarState(vec3 mid, float frameH, float halfW, float shown) {
    float pxPerM = flameFarPxPerM(mid);
    float tallPx = frameH * shown * pxPerM, widePx = 2.0 * halfW * pxPerM;
    float lodNear = log2(max(max(flameScreen.y / max(tallPx, 1e-3), flameScreen.z / max(widePx, 1e-3)), 1.0));
    float splat = 1.0 - smoothstep(kSplatFullPx, kSplatNonePx, sqrt(tallPx * widePx));
    return vec2(splat, smoothstep(kFarFromLod, kFarToLod, lodNear));
}

// ---- Volume tongues (fix 6: big flames drawn as volumes) ----
// A big tongue (a torching crown's, a flaring plant's: FlameSheet::volumeK)
// is a body round its straight axis, not a ribbon: psFireFlamesBook's
// flameVolume integrates the flipbook along each sight line through it. Its
// quad only has to cover that body on screen from any side: a camera-facing
// trapezoid on the plane through the tongue's middle, from its foot's end to
// its frame's top, each end widened by the body's reach at its own depth
// (kVolCover of the frame's half width, times aVolume.y), never edge on.
const float kVolCover = 0.8;     // the body's reach round its axis, a share of its frame's half width
const float kVolDepthShare = 0.4; // its depth's half thickness at the foot, a share of its frame's half width
const float kVolNearM = 0.5;      // an end nearer the eye's plane than this is held there (the camera in a flame)
// Above its tip only the first of the frame's pockets and soot are read, up to kVolTopShare of the tongue's
// length (a converging column's tongues would cross above their tips; the rest is the smoke's).
const float kVolTopShare = 1.45;

vec3 flameVolumeQuad(float t, float side, float halfW, vec3 tip, float len, float frameH) {
    vec3 foot = aFoot.xyz;
    vec3 a = tip / len; // straight: a volume tongue's axis is its chord
    float topS = min(frameH, kVolTopShare * len);
    vec3 top = foot + a * topS;
    vec3 mid = foot + a * (0.5 * topS);
    vec3 n = normalize(mid); // the plane through its middle facing the eye
    float dMid = max(dot(mid, n), kVolNearM);
    float reach = kVolCover * halfW * max(aVolume.y, 0.1);
    // each end on the plane along its own sight line, its reach scaled by its depth
    float dFoot = max(dot(foot, n), kVolNearM), dTop = max(dot(top, n), kVolNearM);
    vec3 f0 = foot * (dMid / dFoot), f1 = top * (dMid / dTop);
    float r0 = reach * dMid / dFoot, r1 = reach * dMid / dTop;
    // the near side of the body bulges toward the eye: a margin
    r0 *= 1.0 + reach / max(dFoot - reach, kVolNearM);
    r1 *= 1.0 + reach / max(dTop - reach, kVolNearM);
    vec3 camUp = vec3(flameView[0][1], flameView[1][1], flameView[2][1]);
    vec3 along = f1 - f0;
    float la = length(along);
    vec3 u = la > 1e-3 * dMid ? along / la : normalize(camUp - n * dot(camUp, n));
    vec3 w = normalize(cross(n, u));
    // the frame's across: square to the axis and the sight line (the ribbon's rMid), the camera's right end on
    vec3 camRight = vec3(flameView[0][0], flameView[1][0], flameView[2][0]);
    vec3 rc = cross(a, -n);
    float sinA = length(rc);
    vec3 fb = normalize(camRight - a * dot(camRight, a));
    fb = dot(rc, fb) < 0.0 ? -fb : fb;
    vec3 lat = normalize(mix(fb, rc / max(sinA, 1e-5), smoothstep(0.02, 0.12, sinA)));
    vVolFoot = vec4(foot, frameH);
    vVolAxis = vec4(a, len);
    vVolLat = vec4(lat, aVolume.x);
    vVolDims = vec4(kVolCover * halfW * max(aVolume.y, 0.1), kVolDepthShare * halfW * max(aVolume.y, 0.1),
                    clamp(aVolume.z, 0.0, 1.0), max(aVolume.w, 0.0));
    float rr = mix(r0, r1, t);
    return mix(f0 - u * r0, f1 + u * r1, t) + w * (side * rr);
}
#else
const float kFlameSink = 0.35;  // the foot goes this far into the ground (m): no gap on a slope
const float kFlameWiden = 1.35; // the quad is this much wider than the sheet (its sides fade)
const float kFlameReach = 1.5;  // and this much taller than the flame (tongues break away above it)
#endif
const float kFlameBend = 1.4;   // the lean grows with height to this power (the flame bends over)
const int kFlameRows = 8;       // vertex rows up the quad

// The burning bed (FireSpreadWorld.hpp FlameStyle::Bed, style 4; the realism
// audit's fix 7): a straight ribbon lying on the ground from its foot
// (aFoot.xyz, lifted by the fire) along its axis (aShape.y, aShape.x,
// aShape.z: east, up, north), aFoot.w wide, turned about the axis to face the
// eye: from above a streak lying on the ground, from the side a low strip as
// tall as it is wide, seen along its axis (within kBedEndOnFrom to
// kBedEndOnTo, the sines of 20 and 10 degrees) a low blob kBedBlobTall of its
// width tall. Its sheet: x across (-1..1), y along the axis from the foot
// (m), z the sine of the eye's angle to the axis; vGain its optical path's
// share by how steeply the eye looks down on it (kBedTopGain from above).
const float kBedEndOnFrom = 0.342;
const float kBedEndOnTo = 0.174;
const float kBedBlobTall = 0.6;
// Its optical path seen from straight above against seen from the side (the
// bed is thin: from above the ground shows through it), vGain's.
const float kBedTopGain = 0.6;
void flameBedVertex(float t, float side) {
    vec3 ax = vec3(aShape.y, aShape.x, aShape.z);
    float len = max(length(ax), 0.05);
    vec3 dir = ax / len;
    float halfW = 0.5 * max(aFoot.w, 0.02);
    vec3 mid = aFoot.xyz + 0.5 * ax;
    vec3 eMid = normalize(-mid);
    vec3 rc = cross(dir, eMid);
    float sinA = length(rc);
    vec3 camRight = vec3(flameView[0][0], flameView[1][0], flameView[2][0]);
    vec3 fb = camRight - eMid * dot(camRight, eMid);
    fb = normalize(dot(rc, fb) < 0.0 ? -fb : fb);
    vec3 r = normalize(mix(fb, rc / max(sinA, 1e-5), smoothstep(0.02, 0.12, sinA)));
    vec3 u = normalize(cross(eMid, r));
    float blob = 1.0 - smoothstep(kBedEndOnTo, kBedEndOnFrom, sinA);
    vec3 c = mix(aFoot.xyz + ax * t, mid + u * ((t - 0.5) * kBedBlobTall * 2.0 * halfW), blob);
    vec3 p = c + r * (side * halfW);
#ifdef FLAME_FLIPBOOK
    // (fix 08) Far off a bed, flat on the ground and under a pixel tall from a
    // low eye, is a splat as a tongue is (kSplatFullPx to kSplatNonePx of its
    // strip as shown): a tent kSplatRadiusPx each way at its middle, lifted
    // in a level view until its bottom edge is at the middle, carrying its
    // light (psFireFlamesBook flameBedMain).
    float pxMid = flameFarPxPerM(mid);
    float shownPx = len * max(sinA, kSplatEndOnShare) * pxMid, widePx = 2.0 * halfW * pxMid;
    float splat = 1.0 - smoothstep(kSplatFullPx, kSplatNonePx, sqrt(shownPx * widePx));
    if (splat > 0.0) {
        float rad = kSplatRadiusPx / pxMid;
        vec3 camUp = vec3(flameView[0][1], flameView[1][1], flameView[2][1]);
        vec3 camFwd = -vec3(flameView[0][2], flameView[1][2], flameView[2][2]);
        float level = 1.0 - smoothstep(0.5, 0.9, abs(camFwd.y));
        vec3 cS = mid + camUp * (rad * level);
        p = mix(p, cS + (camRight * side + camUp * (2.0 * t - 1.0)) * rad, splat);
    }
    vFar = vec4(max(aFar.x, 0.0), splat, 0.0, shownPx * widePx);
    vSplat = vec2(side, 2.0 * t - 1.0);
#endif
    gl_Position = flameProj * (flameView * vec4(p, 1.0));
    gl_Position.xy += flameCam.xy * gl_Position.w;
    vSheet = vec3(side, t * len, sinA);
    vFlame = vec4(halfW, len, clamp(aShape.w, 0.0, 1.0), aSeed.x);
    vPhase = 0.0;
    vStyle = aSeed.z;
    vScroll = 0.0;
    vEmis = clamp(aOptic.x, 0.0, 1.0);
    vBaseFade = 0.0;
    vTemp = vec2(aOptic.w, 0.0);
    vGain = mix(1.0, kBedTopGain, abs(eMid.y));
    float dist = length(p);
    vFogKeep = flameCam.w > 0.0 ? clamp(1.0 - (dist - flameCam.z) * flameCam.w, 0.0, 1.0) : 1.0;
    vRel = p;
}

void main() {
    int row = gl_VertexID >> 1;
    float side = (gl_VertexID & 1) == 0 ? -1.0 : 1.0;
    float t = float(row) / float(kFlameRows - 1);
    vVolFoot = vec4(0.0);
    vVolAxis = vec4(0.0);
    vVolLat = vec4(0.0); // w 0: a ribbon
    vVolDims = vec4(0.0);
    // (fix 08) its level of detail's gain; no splat, no far book unless the
    // flipbook tongue's own size says so below
    vFar = vec4(max(aFar.x, 0.0), 0.0, 0.0, 0.0);
    vSplat = vec2(0.0);
    // (fix 7) the burning bed: a ribbon lying on the ground (its own quad)
    if (aSeed.z > 3.5) {
        flameBedVertex(t, side);
        return;
    }
#ifdef FLAME_FLIPBOOK
    // The tongue runs along its own axis, foot to tip (the tip aShape.x up and
    // (aShape.y, aShape.z) downwind): the frame's height is its length along
    // that axis, which stands up from the fuel and bends over (kFlameBend; a
    // tongue standing on a crown, aOptic.y 1, is straight). Above the tip the
    // frame holds the tongue's pockets and soot, which rise: there the axis
    // leans on at only kPlumeLean of its slope.
    float halfW = 0.5 * max(aFoot.w, 0.02);
    vec3 tip = vec3(aShape.y, max(aShape.x, 0.02), aShape.z);
    float height = max(length(tip), 0.05);
    float span = height / kFlipFlameShare[int(clamp(aSeed.z, 0.0, 3.0) + 0.5)] + kFlameSink;
    float up = t * span - kFlameSink; // metres along the flame (below 0: sunk into the ground)
    float s = max(up, 0.0) / height;  // the share of the way to the tip
    float bendK = aOptic.y > 0.0 ? aOptic.y : kFlameBend;
    float bend = s <= 1.0 ? pow(s, bendK) : 1.0 + kPlumeLean * bendK * (s - 1.0);
    float slope = s <= 1.0 ? bendK * pow(max(s, 1e-3), bendK - 1.0) : kPlumeLean * bendK;
    vec3 c = aFoot.xyz + vec3(tip.x * bend, tip.y * s + min(up, 0.0), tip.z * bend);
    vec3 axis = normalize(vec3(tip.x * slope, tip.y, tip.z * slope)); // the axis' direction here
    // The ribbon's width runs square to the axis and to the sight line, so it
    // turns about its axis to face the eye. Where the two nearly line up its
    // direction is the camera's right. Every row's width points the way the
    // whole tongue's does (its chord's, rMid), so a bent tongue seen down its
    // bend never twists over on itself.
    vec3 camRight = vec3(flameView[0][0], flameView[1][0], flameView[2][0]);
    vec3 e = normalize(-c);
    vec3 mid = aFoot.xyz + 0.5 * tip;
    vec3 eMid = normalize(-mid);
    vec3 rc = cross(tip / height, eMid);
    float sinA = length(rc); // the whole tongue's: the sine of its angle to the sight line
    vec3 fb = camRight - eMid * dot(camRight, eMid);
    fb = normalize(dot(rc, fb) < 0.0 ? -fb : fb);
    vec3 rMid = normalize(mix(fb, rc / max(sinA, 1e-5), smoothstep(0.02, 0.12, sinA)));
    vec3 rr = cross(axis, e);
    float lr = length(rr);
    rr = dot(rr, rMid) < 0.0 ? -rr : rr;
    vec3 fbRow = camRight - e * dot(camRight, e);
    fbRow = normalize(dot(rMid, fbRow) < 0.0 ? -fbRow : fbRow);
    vec3 r = normalize(mix(fbRow, rr / max(lr, 1e-5), smoothstep(0.02, 0.12, lr)));
    // End on: the rows close in on the tongue's middle and spread again on
    // screen (along the projected axis) to kBlobTall of its width: a round
    // body, foreshortened, never a sliver.
    vec3 uMid = normalize(cross(eMid, rMid));
    float blob = 1.0 - smoothstep(kEndOnTo, kEndOnFrom, sinA);
    vec3 cBlob = mid + uMid * ((up - 0.5 * height) / height * kBlobTall * 2.0 * halfW);
    vec3 p = mix(c, cBlob, blob) + mix(r, rMid, blob) * (side * halfW);
    vGain = min(kFlameGainMax, 1.0 / max(lr, 1e-3));
    // Far off (fix 08): the ribbon becomes a splat (see kSplatFullPx).
    int st = int(clamp(aSeed.z, 0.0, 3.0) + 0.5);
    float frameH = height / kFlipFlameShare[st];
    float shown = max(sinA, kSplatEndOnShare);
    vec2 farState = flameFarState(mid, frameH, halfW, shown);
    if (farState.x > 0.0) {
        float sC = kFarCentroid[st] * frameH / height;
        float bC = sC <= 1.0 ? pow(sC, bendK) : 1.0 + kPlumeLean * bendK * (sC - 1.0);
        vec3 cS = aFoot.xyz + vec3(tip.x * bC, tip.y * sC, tip.z * bC);
        float rad = kSplatRadiusPx / flameFarPxPerM(cS);
        vec3 camUp = vec3(flameView[0][1], flameView[1][1], flameView[2][1]);
        vec3 camFwd = -vec3(flameView[0][2], flameView[1][2], flameView[2][2]);
        float level = 1.0 - smoothstep(0.5, 0.9, abs(camFwd.y));
        cS += camUp * (max(rad - dot(cS - aFoot.xyz, camUp), 0.0) * level);
        vec3 pS = cS + (camRight * side + camUp * (2.0 * t - 1.0)) * rad;
        p = mix(p, pS, farState.x);
    }
    float pxMid = flameFarPxPerM(mid);
    vFar = vec4(max(aFar.x, 0.0), farState.x, farState.y, (frameH * shown * pxMid) * (2.0 * halfW * pxMid));
    vSplat = vec2(side, 2.0 * t - 1.0);
    // (fix 6) a volume tongue: no ribbon and no end-on squash, its body's quad
    // (fix 08: a far one, under its splat's size, stays a ribbon going over
    // to its splat: no volume there)
    if (aVolume.x > 0.0 && farState.x <= 0.0) {
        p = flameVolumeQuad(t, side, halfW, tip, height, span - kFlameSink);
        vGain = 1.0;
    }
#else
    vec2 toCam = -aFoot.xz;
    float len = length(toCam);
    vec2 d = len > 1e-3 ? toCam / len : vec2(0.0, 1.0);
    vec3 across = vec3(-d.y, 0.0, d.x);
    float halfW = 0.5 * max(aFoot.w, 0.2) * kFlameWiden;
    float height = max(aShape.x, 0.1);
    float span = height * kFlameReach + kFlameSink;
    float up = t * span - kFlameSink;
    float bend = pow(max(up, 0.0) / height, kFlameBend);
    vec3 p = aFoot.xyz + across * (side * halfW) + vec3(aShape.y * bend, up, aShape.z * bend);
    vGain = 1.0;
#endif
    gl_Position = flameProj * (flameView * vec4(p, 1.0));
    gl_Position.xy += flameCam.xy * gl_Position.w;
    vSheet = vec3(side, up, t * span);
    vFlame = vec4(halfW, height, clamp(aShape.w, 0.0, 1.0), aSeed.x);
    vPhase = aSeed.y;
    vStyle = aSeed.z;
    vScroll = aSeed.w;
    vEmis = clamp(aOptic.x, 0.0, 1.0);
    vBaseFade = max(aOptic.z, 0.0);
    vTemp = vec2(aOptic.w, aOptic.y > 0.0 ? 0.0 : 1.0);
    float dist = length(p);
    vFogKeep = flameCam.w > 0.0 ? clamp(1.0 - (dist - flameCam.z) * flameCam.w, 0.0, 1.0) : 1.0;
    vRel = p;
}
)";

// The tonemap the flames and their merge share: the lit prelude's
// tonemapNeutral (PBR Neutral without its toe), identity below 0.8, a soft
// shoulder above, hue kept, highlights whiten. With the classic display
// (no photoreal camera) the frame is coded through it.
const char kFireFlameTonemapGLSL[] = R"(
vec3 flameToLinear(vec3 c) { return pow(max(c, vec3(0.0)), vec3(2.2)); }
vec3 flameTonemap(vec3 c) {
    float peak = max(c.r, max(c.g, c.b));
    if (peak < 0.8) return c;
    const float d = 0.2;
    float newPeak = 1.0 - d * d / (peak + d - 0.8);
    c *= newPeak / peak;
    float g = 1.0 - 1.0 / (0.15 * (peak - newPeak) + 1.0);
    return mix(c, vec3(newPeak), g);
}
)";

// What both flames share, after the LitConstants block, the camera's encoding
// and the tonemap: the inputs and outputs, the soft fade against the scene's
// depth, the frame's light scale, and the output (fog, then the linear flame
// target or, without it, the frame itself).
const char kFireFlameCommonGLSL[] = R"(
uniform vec4 flameFog; // rgb the classic fog colour (display), w the fire's clock (s)
// Soft particles: the world's depth copied before the translucent objects
// (EngineGL33_FireFlames.cpp), read while flameDepthMap.y is not 0: x the
// projection's _33, y its _43 (view z = y / (ndc z - x)), ndc z = depth z + w.
uniform sampler2D flameDepth;
uniform vec4 flameDepthMap;
// x 0: into the linear flame target (rgb the light added at the frame's
// pre-exposure, a what of the frame stays; the post pass merges it, decoded),
// 1: straight into the frame, coded and premultiplied (no target this frame).
// y 1: the flipbook read at its full resolution only (FLIPDEBUG).
// z the depth copy's texels per flame pixel across (2: the linear target is
// half the frame's size).
uniform vec4 flameOut;
// x the flames' own exposure over the camera's, log2 (PhotoCamera.hpp
// FlameExposureLog2; 0 by day and without the photoreal camera): every light
// the flames make (their black body, flameBlackBody; the procedural flame's
// light) is exposed exp2(x) more than the scene.
uniform vec4 flameExpose;

in vec3 vSheet;
flat in vec4 vFlame;
flat in float vPhase;
flat in float vStyle;
flat in float vScroll;
flat in float vEmis;
flat in float vBaseFade;
flat in vec2 vTemp;
in float vGain;
in float vFogKeep;
in vec3 vRel;
flat in vec4 vVolFoot;
flat in vec4 vVolAxis;
flat in vec4 vVolLat;
flat in vec4 vVolDims;
flat in vec4 vFar;
in vec2 vSplat;

layout(location = 0) out vec4 fragColor;
// Straight into the frame, the surface target (the surfaces switch): like
// smoke, the flame lowers the share of the surface behind it the post pass's
// occlusion may darken. Into the linear target, its depth target: r the
// flame's view depth (km) times its weight, g the weight (how much of the
// pixel it makes: its cover and its light), summed over every sheet, so the
// merge knows how far the flames in a pixel are (the fog in front of them,
// what of the full-resolution scene stands in front of them).
layout(location = 1) out vec4 fragSurf;

const float kFlameSoftDepth = 0.4; // a flame fades out over this much in front of what is behind it (m)
// A flame this near the eye (m) fades out, gone at kFlameNearFrom: closer
// than that its flipbook is magnified past what it holds (a blurred blob
// over half the view), and the eye would be in the flame.
const float kFlameNearFrom = 3.5;
const float kFlameNearTo = 9.0;
// A small tongue's flipbook stays within what it holds much nearer: the fade
// scales with its frame's height (kFlameNearFromFrames to
// kFlameNearToFrames of it, never past the constants above), so a flamelet
// left on the burnt ground still shows a metre or two from the eye. The
// flipbook's program sets these per tongue; any other keeps the constants.
const float kFlameNearFromFrames = 1.2;
const float kFlameNearToFrames = 3.0;
float gFlameNearFrom = kFlameNearFrom;
float gFlameNearTo = kFlameNearTo;
// (fix 6) A volume tongue's view depth (m) for the depth target: its light-weighted samples' (-1: the quad's own).
float gFlameViewZ = -1.0;
const float kFlameCamGain = 1.0;   // straight into the frame under the photoreal camera: its light's code scale
const vec3 kFlameLuma = vec3(0.2126, 0.7152, 0.0722);
const float kFlameDepthLight = 4.0; // the depth weight: the cover plus this times the light (scene light), at most 1

float flameViewZ(float d) {
    return flameDepthMap.y / ((d * flameDepthMap.z + flameDepthMap.w) - flameDepthMap.x);
}

// 1 well in front of the scene, 0 where the flame meets it (the ground under
// its foot, a wall it stands against) and behind it (the linear target has no
// depth of its own: this is its depth test); 1 without the depth copy. At
// half resolution against the farthest of the four scene texels under the
// pixel: the flame stays wherever any of them shows it, and the merge cuts
// it at the full resolution where the scene stands in front.
// (fix 08: how far the scene at the pixel fragXY lies behind this fragment, m;
// a far splat asks at its middle, not at its own pixels)
float flameSceneGapAt(vec2 fragXY) {
    float d;
    if (flameOut.z > 1.5) {
        ivec2 p = ivec2(fragXY) * 2;
        ivec2 lim = textureSize(flameDepth, 0) - 1;
        d = max(max(texelFetch(flameDepth, min(p, lim), 0).r, texelFetch(flameDepth, min(p + ivec2(1, 0), lim), 0).r),
                max(texelFetch(flameDepth, min(p + ivec2(0, 1), lim), 0).r,
                    texelFetch(flameDepth, min(p + ivec2(1, 1), lim), 0).r));
    } else {
        d = texelFetch(flameDepth, clamp(ivec2(fragXY), ivec2(0), textureSize(flameDepth, 0) - 1), 0).r;
    }
    return flameViewZ(d) - flameViewZ(gl_FragCoord.z);
}
float flameSoftFadeAt(vec2 fragXY, float softM) {
    if (flameDepthMap.y == 0.0) return 1.0;
    return smoothstep(0.0, softM, flameSceneGapAt(fragXY));
}
float flameSoftFadeOver(float softM) { return flameSoftFadeAt(gl_FragCoord.xy, softM); }
float flameSoftFade() { return flameSoftFadeOver(kFlameSoftDepth); }

// Scene light (the rig's units: a white surface under the reference sun is 1)
// to what the frame stores: the rig's exposure (litZenith.w), and under the
// photoreal camera the frame's pre-exposure (litCamera.w).
float flameSceneScale() {
    return litZenith.w * (litCamera.x > 0.5 ? max(litCamera.w, 0.0) : 1.0);
}

// Writes a flame fragment: `light` the light it adds at the frame's scale (as
// lit surfaces and pre-lit draws store theirs), `trans` the share of what is
// behind it that stays. The classic fog (with the haze or the volumetric fog,
// only its far edge: litHaze.x) fades the light out, and what the flame hides
// takes the fog colour, so a fully fogged flame is the fog over what is behind.
void flameWrite(vec3 light, float trans) {
    // Its level of detail's gain (fix 08: the kept tongue carries the light
    // and the optical depth of those it stands for).
    light *= vFar.x;
    trans = pow(clamp(trans, 0.0, 1.0), vFar.x);
    // Per pixel (a big ribbon sweeping past the eye has all its corners far
    // off: a fade from them would let the near plane cut it along a line).
    float nearKeep = smoothstep(gFlameNearFrom, gFlameNearTo, length(vRel));
    light *= nearKeep;
    trans = 1.0 - (1.0 - trans) * nearKeep;
    float occ = 1.0 - trans;
    if (occ < 0.002 && max(light.r, max(light.g, light.b)) < 1e-5) discard;
    float fogKeep = clamp(min(vFogKeep * (1.0 + litHaze.x), 1.0), 0.0, 1.0);
    vec3 fogC = flameFog.rgb;
    if (litCamera.x > 0.5) fogC *= pow(max(litCamera.w, 0.0), 1.0 / 2.2);
    vec3 lit = light * fogKeep + flameToLinear(fogC) * (occ * (1.0 - fogKeep));
    if (flameOut.x > 0.5) {
        vec3 code = litCamera.x > 0.5 ? camFold(lit * kFlameCamGain) : pow(flameTonemap(lit), vec3(1.0 / 2.2));
        fragColor = vec4(code, occ);
        fragSurf = vec4(0.0, 0.0, 0.0, occ);
    } else {
        fragColor = vec4(lit, trans);
        float w = min(occ + kFlameDepthLight * dot(lit, kFlameLuma) / max(flameSceneScale(), 1e-6), 1.0);
        float z = flameDepthMap.y != 0.0 ? flameViewZ(gl_FragCoord.z) : 0.0;
        if (gFlameViewZ >= 0.0) z = gFlameViewZ; // (fix 6) a volume's own
        fragSurf = vec4(w * z * 0.001, w, 0.0, 1.0);
    }
}
)";

// The procedural flame's main (the fallback without the flipbook). The
// tunables are the constants at its top.
const char kFireFlameFragmentGLSL[] = R"(
// ---- Tunables ----
const vec2 kFlameCell = vec2(0.3, 0.6);   // the tongues' noise cells across and up, in flame heights ...
const vec2 kFlameCellMin = vec2(0.22, 0.4); // ... at least this (m) ...
const vec2 kFlameCellMax = vec2(2.5, 5.0);  // ... and at most this (m), before each sheet's scale
const float kFlameRise = 2.2;             // the flame's gas rises this fast (m/s) ...
const float kFlameRiseSpread = 0.3;       // ... give or take this share from sheet to sheet
const float kFlameWarp = 0.6;             // the sideways lick: the domain warp's reach (noise cells), at the top
const float kFlameTongue = 2.4;           // how far the noise moves the silhouette (in flame heights)
const float kFlameSolid = 0.2;            // below this share of its height the flame is one continuous band
const float kFlameSoft = 0.1;             // the silhouette's soft edge (in flame heights)
const float kFlameNarrow = 0.4;           // the body narrows by this share of its width toward the top
const float kFlameEdge = 0.55;            // the sides fade out from this share of the half width (neighbours blend)
const float kFlameLowHeat = 0.35;         // a dying flame (heat 0) reaches this share of its height ...
const float kFlameDim = 0.3;              // ... with this share of a full one's light
const float kFlameCore = 1.7;             // the white-yellow core's light (display-linear: above 1 it blooms)
const float kFlameTip = 0.45;             // the deep red tips' light
const float kFlameOpacity = 0.5;          // the share of what is behind that the body hides (it adds the rest)
const float kFlameSootAlpha = 0.3;        // the sooty fringe over the tips: its opacity at most ...
const float kFlameSootReach = 0.3;        // ... and its reach above the silhouette (in flame heights)
const float kFlameGroundFade = 0.6;       // the flame fades in over this much from its sunk foot (m)
const float kFlameFlicker = 0.15;         // its light flickers by this share
const float kFlameHueSpread = 0.1;        // sheets differ in hue by this share of their green
const vec3 kFlameRed = vec3(0.55, 0.05, 0.005); // the temperature ramp (display-linear, before the light)
const vec3 kFlameOrange = vec3(1.0, 0.3, 0.03);
const vec3 kFlameYellow = vec3(1.0, 0.68, 0.18);
const vec3 kFlameWhite = vec3(1.0, 0.92, 0.72);
// The noise lattice repeats every kFlamePeriod cells, and the rise wraps at
// two periods (the warp's field rises at half the gas's speed), so the scroll
// never jumps however long the fire burns. Not a tunable.
const float kFlamePeriod = 289.0;

float flameHash(vec2 i) {
    i = mod(i, kFlamePeriod);
    vec3 p3 = fract(vec3(i.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}
float flameNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = p - i;
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(flameHash(i), flameHash(i + vec2(1.0, 0.0)), u.x),
               mix(flameHash(i + vec2(0.0, 1.0)), flameHash(i + vec2(1.0, 1.0)), u.x), u.y);
}

const vec3 kFlameSootGrey = vec3(0.02); // soot's own colour (display-linear): it mostly darkens

// The procedural flame's light is display-linear at the frame's scale (as
// pre-lit draws store theirs): `light` over its `cover`, `alpha` of what is
// behind hidden by the body and `soot` of it by a translucent grey.
void flameFinish(vec3 light, float cover, float alpha, float soot) {
    float a = alpha + soot * (1.0 - alpha);
    flameWrite(light * cover + kFlameSootGrey * (soot * (1.0 - alpha)), 1.0 - a);
}

void main() {
    // (the burning bed, style 4, is the flipbook variant's: none in this fallback)
    if (vStyle > 3.5) discard;
    float halfW = vFlame.x;
    float height = vFlame.y;
    float heat = vFlame.z;
    float seed = vFlame.w;
    float time = flameFog.w;
    // On the sheet in metres: across from its middle, up from the ground.
    vec2 m = vec2(vSheet.x * halfW, vSheet.y);
    // The pixel's footprint on the sheet (m): each octave fades out where its
    // cells would fall under about two pixels.
    float foot = max(fwidth(m.x), fwidth(m.y));
    // Each sheet its own pattern, size, speed and hue (the seed is fixed for
    // its life, so nothing here jumps from frame to frame).
    vec2 cell = clamp(kFlameCell * height, kFlameCellMin, kFlameCellMax) * mix(0.8, 1.25, fract(seed * 7.13));
    float rise = kFlameRise * (1.0 + kFlameRiseSpread * (2.0 * fract(seed * 13.7) - 1.0));
    float scroll = mod(time * rise / cell.y, 2.0 * kFlamePeriod);
    vec2 q = m / cell + vec2(floor(seed * 97.0) + 0.37, floor(fract(seed * 3.31) * 53.0));
    float k0 = 1.0 - smoothstep(0.25, 0.5, foot / cell.x);
    float k1 = 1.0 - smoothstep(0.25, 0.5, 2.0 * foot / cell.x);
    float k2 = 1.0 - smoothstep(0.25, 0.5, 4.0 * foot / cell.x);
    // How high this flame reaches now (heat lowers a dying one), and the
    // fragment's height as a share of it.
    float reach = height * mix(kFlameLowHeat, 1.0, heat);
    float h = max(m.y, 0.0) / reach;
    // The warp: sideways licks and vertical breaks, on a field rising at half
    // the gas's speed, stronger toward the top.
    float amp = kFlameWarp * (0.25 + min(h, 1.5)) * k0;
    vec2 w = vec2(flameNoise(vec2(q.x + 31.7, q.y - 0.5 * scroll)), flameNoise(vec2(q.x + 7.3, q.y - 0.5 * scroll + 91.1))) - 0.5;
    vec2 qw = vec2(q.x + w.x * amp, q.y - scroll + w.y * amp * 0.6);
    float n = 0.55 * mix(0.5, flameNoise(qw), k0) +
              0.30 * mix(0.5, flameNoise(qw * 2.0 + vec2(17.0, 41.0)), k1) +
              0.15 * mix(0.5, flameNoise(qw * 4.0 + vec2(63.0, 7.0)), k2);
    // The silhouette: noise minus the height ramp, a continuous band at the
    // foot, the sides fading so neighbouring sheets run into one front.
    float ax = abs(vSheet.x);
    float edge = kFlameEdge * (1.0 - kFlameNarrow * clamp(h, 0.0, 1.0));
    float field = (1.0 - h) + (n - 0.5) * kFlameTongue * smoothstep(0.0, kFlameSolid, h) -
                  smoothstep(edge, 1.0, ax) * 1.5;
    float fw = max(fwidth(field), 1e-4);
    float body = smoothstep(-0.5 * fw, kFlameSoft + 0.5 * fw, field);
    float ground = smoothstep(0.0, kFlameGroundFade, vSheet.z) * flameSoftFade();
    // Temperature: hottest deep inside the body, coolest at its edge.
    float temp = clamp(field * 1.15, 0.0, 1.0) * mix(0.7, 1.0, heat);
    vec3 c = mix(kFlameRed, kFlameOrange, smoothstep(0.0, 0.4, temp));
    c = mix(c, kFlameYellow, smoothstep(0.45, 0.85, temp));
    c = mix(c, kFlameWhite, smoothstep(0.88, 1.0, temp));
    c.g *= 1.0 + kFlameHueSpread * (2.0 * fract(seed * 5.71) - 1.0);
    // Flicker: two sines a sheet (wrapped, so a long fire keeps its precision).
    float ft = mod(time, 628.3185);
    float flicker = 1.0 + kFlameFlicker * (0.6 * sin(ft * 7.3 + seed * 40.0) + 0.4 * sin(ft * 12.9 + seed * 17.0));
    vec3 light = c * (mix(kFlameTip, kFlameCore, temp * temp) * mix(kFlameDim, 1.0, heat) * flicker *
                      exp2(flameExpose.x));
    // The sooty fringe over the tips (fading out far away, where it would be
    // a dark halo over the whole front).
    float soot = smoothstep(-kFlameSootReach, 0.0, field) * (1.0 - body) * smoothstep(0.55, 1.0, h) *
                 (kFlameSootAlpha * heat) * k1 * ground;
    float cover = body * ground;
    flameFinish(light, cover, cover * kFlameOpacity * mix(0.55, 1.0, temp), soot);
}
)";

// The flipbook flame's main: one flame tongue a sheet (tools/firespread/
// gen_flame_flipbook.py, four styles: STYLE_ORDER front, tongues, creep,
// bushy) as physical fields ("Physical fields": R the soot's temperature, G
// its optical depth; uploaded as RG, the emissivity rebuilt here from the
// temperature as bake() makes it), a layer a frame, style-major (layer =
// style * kFlipFrames + frame), played per sheet (vPhase, the next frame
// blended in), each sheet with its own style (vStyle) and flip. A frame is
// the whole ribbon: the tongue in its middle, nothing at its sides. The
// light is the black body's at the temperature times the emissivity, in
// scene light (kFlameNitToScene); the soot is a medium (it lets exp(-tau)
// through and scatters the sun and the sky). The camera exposes it all;
// nothing here is a look.
// The tongue's optical depth is its side-on one (vEmis) times vGain, the
// sight line's path through it over its thickness (more toward end on).
// Close up, where a flipbook texel covers several pixels, fine detail the
// flipbook cannot hold is added: the frame is read through a two-octave warp
// that climbs with the gas (vScroll), the luminous zone's edge is broken up
// by finer noise and drawn crisper. It fades out by the pixel's footprint, so
// beyond a few tens of metres the flames are exactly the flipbook's.
const char kFireFlameFlipbookGLSL[] = R"(
uniform sampler2DArray flameBook; // the flipbook: kFlipFrames layers a style, one tongue a frame

// ---- Calibration ----
// Scene light of 1 cd/m2: the rig's scale makes a white surface under the
// reference sun (60 degrees, about 100 klx of sun and sky) 1.0 in scene light
// (LightRig.cpp ComputePhysicalRig), which is about 100000 / pi = 32000 cd/m2,
// so sunlit grass (albedo about 0.2) is about 6000 cd/m2.
const float kFlameNitToScene = 1.0 / 32000.0;
// A flame's luminance (cd/m2, at emissivity 1) by the soot's radiation
// temperature K: kFlameCoreNits at kFlameCoreK, falling as
// exp(kFlameSlopeK (1 / kFlameCoreK - 1 / K)). Anchored on luminance, not on
// Planck's slope at a measured temperature: a candle's luminous zone is about
// 1e4 cd/m2, and by day a grass fire's flames are clearly brighter than the
// sunlit ground round them (about 6000 cd/m2). So a 1650 K core at emissivity
// 0.6 is about 2.4e4 cd/m2 and a 1450 K body at 0.5 about 5000.
const float kFlameCoreK = 1650.0;
const float kFlameCoreNits = 4.0e4; // the flames' brightness: tune it here
const float kFlameSlopeK = 16500.0;

// ---- Tunables ----
// The flame's tip sits at this share of the frame's height, per style (x
// front, y tongues, z creep, w bushy; vsFireFlames' too, FireSpreadWorld.hpp
// kFlameTipShare as gen_flame_flipbook.py measures it).
const vec4 kFlipFlameShare = vec4(0.551, 0.566, 0.523, 0.543);
const float kFlipGlow = 1.0;         // the light's scale (1: as calibrated)
// Kirchhoff: the luminous gas absorbs what is behind it as much as it emits
// (its absorptivity is its emissivity); the soot's optical depth hides the rest.
const float kFlipGasOcclusion = 1.0;
const float kFlipDim = 0.3;          // a dying flame's (heat 0) share of that light ...
const float kFlipDimSoot = 0.1;      // ... and of its smoke's optical depth (a dying flame makes little soot)
// The foot fades in from the ground (the tongue's foot point) over
// kFlipBaseShare of its length (within kFlipBaseMinM..MaxM; a tongue standing
// on a crown over its own vBaseFade), the fade's top ragged across the ribbon
// (a two-octave noise kFlipBaseRagCell of its length across, the height
// between kFlipBaseRagLo and kFlipBaseRagHi of it): no straight line where a
// flat ribbon meets the ground (the scene's depth only softens the last few
// centimetres of a ribbon seen low over the ground).
const float kFlipBaseShare = 0.14;
const float kFlipBaseMinM = 0.04;
const float kFlipBaseMaxM = 0.6;
const float kFlipBaseRagCell = 0.08;
const float kFlipBaseRagLo = 0.3;
const float kFlipBaseRagHi = 1.6;
const float kFlipBaseAirStart = 0.8; // a crown tongue's fade starts up to this share of it above its foot
// The soft fade against the scene's depth spans at least this share of the ribbon's half width.
const float kFlipSoftShare = 0.25;
// The sheet's smoke starts this share of the way up the flame (no smoke at
// the fuel bed under the flame).
const float kFlipSootFrom = 0.25;
// Per style (x front, y tongues, z creep, w bushy): the light's scale.
const vec4 kFlipStyleLight = vec4(1.0, 1.0, 0.9, 1.0); // creep a little dimmer
// Per style: the smoke's albedo's scale. A bushy flame (shrubs, torching
// crowns) makes a dark, sooty smoke; grass a light one.
const vec4 kFlipStyleSmoke = vec4(1.0, 1.0, 1.0, 0.22);
// The luminous zone's soft edge, where its soot is cooling: it still hides
// what is behind it (the soot is there) while it glows less, so the edge
// absorbs as zone^kEdgeOccPow while it emits as the zone: a flame's tips and
// sides go dark orange into smoke instead of letting the sky through its
// orange (salmon and pink over a blue sky). Not much: where tongues overlap a
// stronger law draws a dark rim round each one (paper cut-outs).
const float kEdgeOccPow = 0.75;
// The smoke as a medium: its single-scattering albedo (a grey-tan; soot
// absorbs most of what it meets), the share of the key light (the sun, or the
// moon under the moon key) and of the sky it scatters toward the eye, how
// fast the light inside it dims with its depth, and the flames' own light on
// it (from kFlipSmokeFireK at this share of the flame's emissivity, fading out
// over a flame height above the flame's top).
const vec3 kFlipSmokeAlbedo = vec3(0.3, 0.27, 0.23);
const float kFlipSmokeSun = 0.6;     // as the lit particles take it (LightRig.hpp kParticleKeyShare)
const float kFlipSmokeSky = 1.0;
const float kFlipSmokeSelfShadow = 0.6;
const float kFlipSmokeFireK = 1300.0;
const float kFlipSmokeFire = 0.25;
const float kFlipBrightness = 1.6;   // the materials' brightness the rig folds into every surface (LightRig.cpp kBrightness)
// The luminous zone's emissivity from the temperature, as gen_flame_flipbook.py
// bake() makes it: the simulation's temperature back from the kelvin,
// T = (K - kFlipSimKCold) / kFlipSimKPerT (SIM_K_COLD, SIM_K_PER_T there; keep
// them equal), emissivity = the sheet's (vEmis, from its flame's optical
// path) times smoothstep(kFlipBodyLo, kFlipBodyHi, T).
// The bake writes R = 0 where nothing glows, so cold air never reads as flame.
const float kFlipBodyLo = 0.12;
const float kFlipBodyHi = 0.45;
const float kFlipSimKCold = 1150.0;
const float kFlipSimKPerT = 330.0;
// The bake's fixed layout (gen_flame_flipbook.py K0, K_SPAN, TAU_MAX, the loop, the styles).
const float kFlipK0 = 600.0;
const float kFlipKSpan = 1000.0;
const float kFlipTauMax = 4.0;
const float kFlipFrames = 128.0;
const float kFlipStyles = 4.0; // FireSpreadWorld.hpp kFlameStyles
// Where the flipbook holds nothing (both fields under this), nothing is drawn.
const float kFlipEmpty = 0.002;
const float kFlipEmptyLod = 3.0; // the coarse read's mip (8 texels: wider than the warp's reach)
// (A small or far tongue, a few pixels tall, whose mip would average its core
// with the air round it under the luminous zone's threshold, reads the far
// book instead: fix 08, the vertex shader's kFarFromLod.)
// The fine detail close up: its noise cell is kDetailCell of the sheet's
// height, within kDetailCellMin..Max metres (EngineGL33_FireFlames.cpp's same
// names, which scroll it); the warp reaches kDetailWarp cells across and
// kDetailWarpUp of that up, full from kDetailRampUp of the flame's height
// (the foot stays put); the edge's noise moves the temperature by up to
// kDetailEdge, and the luminous zone's edge narrows to kDetailBand of its
// width. All of it is gone where a noise cell spans more than
// kDetailFarCells pixels' footprint (from kDetailNearCells).
const float kDetailCell = 0.08;
const float kDetailCellMin = 0.03;
const float kDetailCellMax = 0.6;
const float kDetailPeriod = 289.0;
const float kDetailWarp = 0.5;
const float kDetailWarpUp = 0.6;
const float kDetailRampUp = 0.4;
const float kDetailEdge = 0.15;
const float kDetailBand = 0.3;
const float kDetailNearCells = 0.15;
const float kDetailFarCells = 0.45;
// Classic lighting (no photoreal camera): the eye adapts to the scene as the
// camera's meter would: the flames' light is raised by the dark scene's white
// level against the reference's, at most this many times.
const float kFlipClassicAdaptMax = 64.0;

// The linear colour of a black body (CombatLights.cpp BlackBodyRgb: red 1,
// a fit to the sRGB black-body table), at unit luminance times its luminance
// in scene light, at the flames' own exposure (flameExpose.x: more at dusk
// and at night, PhotoCamera.hpp FlameExposureLog2).
vec3 flameBlackBody(float k) {
    float t = clamp(k, 1000.0, 3000.0);
    vec3 c = vec3(1.0, max(0.155 + (t - 1500.0) * 0.000207, 0.05),
                  0.15 * pow(clamp((t - 1900.0) / 1100.0, 0.0, 1.0), 1.5));
    float nits = kFlameCoreNits * exp(kFlameSlopeK * (1.0 / kFlameCoreK - 1.0 / max(k, 1.0)));
    return c * (nits * kFlameNitToScene * exp2(flameExpose.x) / dot(c, kFlameLuma));
}

float detailHash(vec2 i) {
    i = mod(i, kDetailPeriod);
    vec3 p3 = fract(vec3(i.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}
float detailNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = p - i;
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(detailHash(i), detailHash(i + vec2(1.0, 0.0)), u.x),
               mix(detailHash(i + vec2(0.0, 1.0)), detailHash(i + vec2(1.0, 1.0)), u.x), u.y);
}

// The light a soot medium of optical depth tau scatters toward the eye per
// unit of its opacity (scene light): the key light and the sky, dimmer deeper
// in, and the flames' own light from below.
vec3 flameSmokeLight(float tau, float fire, float albedo) {
    float inside = exp(-kFlipSmokeSelfShadow * tau);
    vec3 key = litSun.rgb * (litSunDir.w * kFlipSmokeSun * inside);
    vec3 sky = 0.5 * (litZenith.rgb + litHorizon.rgb) * (kFlipSmokeSky * mix(0.6, 1.0, inside));
    return kFlipSmokeAlbedo * (albedo * kFlipBrightness) * (key + sky) + flameBlackBody(kFlipSmokeFireK) * fire;
}

// ---- Far flames (fix 08; the realism audit's C13) ----
// The far book (EngineGL33_FireFlames.cpp, each texel FlameFarTexel's): per
// frame, at a quarter of the flipbook's size, rgba the luminous zone, the
// zone times its luminance relative to kFarRefCoreK's, the cover it hides
// with (zone^kEdgeOccPow) and the soot outside it, all linear in what they
// make on screen, so their mips are a far tongue's light, kept. Its
// luminance law (kFarRefCoreK, kFarRefSlopeK) is FlameFarTexel's, not the
// flame's own calibration above (which another change may move): from it the
// far book gives back the luminance-equivalent kelvin, and the flame's own
// black body lights it.
uniform sampler2DArray flameFar;
const float kFarRefCoreK = 1650.0;
const float kFarRefSlopeK = 16500.0;
const float kFarEmpty = 1e-4;       // under this zone and soot the far book holds air
const float kSplatMeanLod = 6.0;    // its 1 x 1 level: a frame's mean
const float kSplatBaseKeep = 0.82;  // the share of a frame's light the foot's fade keeps (fix-08 tools\splatconst.py)
const float kSplatSoftShare = 0.02; // a splat is cut where the scene stands this share of its distance in front of it
const float kSplatRadiusPx = 1.0;   // its half size in the flame target's pixels (the vertex shader's)

// What the near, far and splat lights share (main sets them).
float gHalfW, gHeight, gHeat, gSeed, gFrameH, gFlip, gLayer0, gF0, gF1;
int gStyle;

// Classic lighting: the eye's adaptation to the scene's white level (a
// white surface under the key light and the sky, as the rig shows it).
float flameAdapt() {
    if (litCamera.x >= 0.5) return 1.0;
    float white = dot(litSun.rgb * (max(litSunDir.y, 0.0) * litSunDir.w) + litZenith.rgb, kFlameLuma) *
                  kFlipBrightness * litZenith.w;
    return clamp(1.0 / max(white, 1e-4), 1.0, kFlipClassicAdaptMax);
}

// The far book's fields on the sheet's two frames, blended (at the hardware's
// mip, or at `lod` when it is not negative).
vec4 flameFarRead(vec2 uv, float lod) {
    vec3 a = vec3(uv, gLayer0 + gF0), b = vec3(uv, gLayer0 + gF1);
    return lod < 0.0 ? mix(texture(flameFar, a), texture(flameFar, b), fract(vPhase))
                     : mix(textureLod(flameFar, a, lod), textureLod(flameFar, b, lod), fract(vPhase));
}

// The black body's light and the cover of far book fields fb at emissivity
// emis and the foot's fade `fade` (scene light, before the scene's scale):
// the luminance-equivalent kelvin rebuilt from them, offset as the near
// flame's (vTemp.x), lit by the flame's own black body.
void flameFarFields(vec4 fb, float emis, float fade, out vec3 emit, out float occ) {
    emit = vec3(0.0);
    occ = 0.0;
    if (fb.r <= kFarEmpty) return;
    float rel = max(fb.g / fb.r, 1e-6);
    float k = 1.0 / (1.0 / kFarRefCoreK - log(rel) / kFarRefSlopeK);
    emit = flameBlackBody(k + vTemp.x) *
           (fb.r * emis * fade * kFlipGlow * kFlipStyleLight[gStyle] * mix(kFlipDim, 1.0, gHeat));
    occ = emis * fb.b * fade;
}

// A far tongue on its ribbon, from the far book: as the near light, without
// the fine detail and the zone's crisp edge (both under a pixel by then).
// alone: nothing else lights this fragment, so the air is discarded early.
void flameFarLight(bool alone, out vec3 light, out float trans) {
    vec2 uv = vec2(0.5 + 0.5 * gFlip * vSheet.x, vSheet.y / gFrameH);
    vec4 fb = flameFarRead(uv, -1.0);
    if (alone && fb.r <= kFarEmpty && fb.a <= kFarEmpty) discard;
    float soft = flameSoftFadeOver(max(kFlameSoftDepth, kFlipSoftShare * gHalfW));
    float baseH = vBaseFade > 0.0 ? vBaseFade : clamp(kFlipBaseShare * gHeight, kFlipBaseMinM, kFlipBaseMaxM);
    float base0 = vBaseFade > 0.0 ? 0.5 * baseH * kFlipBaseAirStart : 0.0;
    float fade = smoothstep(base0, base0 + baseH, vSheet.y) * soft;
    float emis = 1.0 - pow(1.0 - min(vEmis, 0.999), vGain);
    vec3 emit;
    float occ;
    flameFarFields(fb, emis, fade, emit, occ);
    float yr = vSheet.y / max(gHeight, 0.05);
    float tau = kFlipTauMax * fb.a * mix(kFlipDimSoot, 1.0, gHeat) * fade * vGain * smoothstep(0.0, kFlipSootFrom, yr);
    float sootT = exp(-tau);
    float above = max(vSheet.y - gHeight, 0.0) / max(gHeight, 0.1);
    vec3 smoke = flameSmokeLight(tau, kFlipSmokeFire * 0.6 * gHeat * exp(-above), kFlipStyleSmoke[gStyle]) * (1.0 - sootT);
    light = (smoke + emit * (sqrt(sootT) * flameAdapt())) * flameSceneScale();
    trans = (1.0 - kFlipGasOcclusion * occ) * sootT;
}

// A tongue too small to draw (under kSplatNonePx): a splat (the vertex
// shader's), its light the frame's mean times the ribbon's area in pixels
// (vFar.w) under a tent (vSplat) that sums to 1 over the pixel centres it
// covers wherever it falls: the tongue's whole light, held steady as it
// moves. Seen or hidden as a whole, by the scene at its middle: hidden only
// where the scene stands in front of it (by up to kSplatSoftShare of its
// distance, faded), never because the ground under a low flame lies close
// behind it (seen from above a grass flame's middle is a metre off the ground).
void flameSplatLight(out vec3 light, out float trans) {
    vec4 fb = flameFarRead(vec2(0.5), kSplatMeanLod);
    float emis = 1.0 - pow(1.0 - min(vEmis, 0.999), vGain);
    vec3 emit;
    float occ;
    flameFarFields(fb, emis, kSplatBaseKeep, emit, occ);
    float dist = length(vRel);
    float seen = flameDepthMap.y == 0.0
                     ? 1.0
                     : smoothstep(-max(kFlameSoftDepth, kSplatSoftShare * dist), 0.0,
                                  flameSceneGapAt(gl_FragCoord.xy - vSplat * kSplatRadiusPx));
    float w = (1.0 - abs(vSplat.x)) * (1.0 - abs(vSplat.y)) * vFar.w * seen;
    light = emit * (flameAdapt() * flameSceneScale() * w);
    trans = 1.0 - min(kFlipGasOcclusion * occ * w, 1.0);
}

// ---- Volume tongues (fix 6: big flames drawn as volumes) ----
// A big tongue's body (vsFireFlamesBook flameVolumeQuad): round its straight
// axis, its flipbook frame read across it (vVolLat, square to the axis, the
// way it faces the eye) and along it, its depth (square to both) a soft round
// mask kVolDepthShare of the frame's half width at the foot, narrowing up the
// tongue. Each sight line is marched only through the slab that holds the
// body's depth (not the whole reach round the axis), in up to kVolSteps steps
// (at least kVolStepsMin): one no longer than kVolStepY of the foot's depth
// across the body and kVolStepS of the frame's half width along the axis, so a
// slanting sight line gets more. Each step takes the depth mask's mean over
// its own stretch (flameVolMaskInt: the mask's integral, exact wherever the
// frame is even over the step), not the mask at one point: a thin part of the
// body between two samples is never missed, and the picture does not depend
// on where the samples fall. (The first build marched the whole reach in 3 to
// 8 point samples, each one opaque: the picture was the nearest of two or
// three hard slices of the frame, which swapped as the flipbook played, so
// the body's surface and outline changed in place instead of flowing: the
// morphing the owner saw.) The light is
// composited front to back: the luminous zone glows and hides at the tongue's
// own visible absorption (vVolLat.w, 1/m: a crown 6.5, a shrub 5), its soot
// temperature mottled (kVolMottleK either way, in cells climbing with the gas:
// the turbulent body's brighter and cooler patches, faded out far off); a
// sooty tongue's (vVolDims.z) soft edges hide more than they glow (its
// cooling soot, kVolSootyOccPow: the consult's crown flame, opaque but for a
// ragged outer edge), more so toward its tips (kVolTipOccPow), where its smoke
// thickens and darkens: dark red into black-brown, never the sky's blue through
// a thin orange (salmon). Seen side
// on that is the frame with a round body's depth; end on the sight line runs
// up the whole tongue: a round, opaque, bright mass, never a squashed card.
// Samples behind the scene drop out one by one (a branch inside the flame
// hides only what is behind it), and the depth target takes the depth of
// the samples that show.
const int kVolSteps = 16;
const int kVolStepsMin = 4;
const float kVolStepY = 0.25;     // a step's reach across the body's depth, a share of the foot's half thickness, at most ...
const float kVolStepS = 0.1;      // ... and along the axis, a share of the frame's half width
const float kVolLodStep = 0.5;    // the frame is read no finer than this share of a step's reach along the axis
const float kVolLodBias = 0.5;    // its mip a little coarser than the pixel's (the integration blurs anyway) ...
const float kVolLodMax = 2.5;     // ... and at most this (a coarser one averages the core with the air round it: gone)
const float kVolNearM = 0.5;      // samples nearer the eye than this are skipped
const float kVolTopShare = 1.45;  // the frame is read up to this share of the tongue's length (vsFireFlamesBook's) ...
const float kVolTopFade = 0.15;   // ... fading out over this share of it below that
const float kVolDepthTop = 1.45;  // the depth narrows to nothing kVolDepthTop of the tongue's length up ...
const float kVolDepthMin = 0.15;  // ... no thinner than this share of the foot's ...
const float kVolDepthPow = 0.45;  // ... as (1 - s / (kVolDepthTop L))^kVolDepthPow (the generator's soot envelope)
const float kVolMaskFrom = 0.55;  // the depth's mask fades from this share of its half thickness
// The body is round: its depth there follows the frame's flame across it, thinning toward the luminous zone's
// edge to kVolRoundLo of the full depth (as sqrt of the zone; the smoke, outside the zone, keeps the full depth).
const float kVolRoundLo = 0.3;
const float kVolPathShare = 0.7;  // the mean depth through the mask over its full width: the frame's soot spreads over it
// The luminous zone hides as zone^kVolBodyOccPow in the body (Kirchhoff: as much as it glows; a ribbon's
// kEdgeOccPow, which hides more at the edge, would draw a dark rim round every tongue inside a column), and as
// zone^kVolTipOccPow at a sooty tongue's tip (its cooling soot hides the sky while it glows less).
const float kVolBodyOccPow = 1.0;
const float kVolSootyOccPow = 0.6;
const float kVolTipOccPow = 0.35;
// The soot's temperature mottling (K, either way) and where it fades out (the detail cell over the pixel's footprint).
const float kVolMottleK = 60.0;
const float kVolMottleNear = 0.5, kVolMottleFar = 2.0;
const float kVolTipFrom = 0.5, kVolTipTo = 0.95; // where a tongue's tip begins and where it is all tip (shares of L)
const float kVolSootBoost = 1.5;  // a sooty tongue's smoke over its tips: this much more ...
const float kVolSootDark = 0.75;  // ... and this much darker
const float kVolSoftShare = 0.1;  // a sample fades out over this share of the body's reach in front of the scene
// The puff (vVolDims.w, FireSpreadWorld.hpp VolumePuffCode): the body's length and width hold steady; a bulge
// climbs it with the gas. Where the bulge is the body is wider across and deeper by its share and glows
// kVolBulgeGlow more; half a wave on it necks and dims as much. Nothing within kVolBulgeFoot of the tongue's
// length of its foot: the foot stays as it is.
const float kVolBulgeStep = 0.02; // the bulge's size comes in steps of this (kVolumeBulgeStep)
const float kVolBulgeFoot = 0.25;
const float kVolBulgeGlow = 0.14;

// The depth mask's integral from the body's middle plane out to u (the
// depth over the half thickness there, signed), to the power p: the mask is 1
// within kVolMaskFrom of the half thickness and falls to 0 at it. A step's
// mean mask is the difference of two of these over its reach (p 1: what
// glows; p under 1: a sooty edge, which hides more than it glows).
float flameVolMaskInt(float u, float p) {
    float a = min(abs(u), 1.0);
    float w = 1.0 - kVolMaskFrom;
    float g = a <= kVolMaskFrom ? a : kVolMaskFrom + w / (p + 1.0) * (1.0 - pow((1.0 - a) / w, p + 1.0));
    return u < 0.0 ? -g : g;
}

// The scene's view depth under the pixel (m; 1e6 without the depth copy):
// as flameSoftFadeOver reads it, the farthest of the four texels at half
// resolution.
float flameVolumeSceneZ() {
    if (flameDepthMap.y == 0.0) return 1e6;
    float dz;
    if (flameOut.z > 1.5) {
        ivec2 p = ivec2(gl_FragCoord.xy) * 2;
        ivec2 lim = textureSize(flameDepth, 0) - 1;
        dz = max(max(texelFetch(flameDepth, min(p, lim), 0).r, texelFetch(flameDepth, min(p + ivec2(1, 0), lim), 0).r),
                 max(texelFetch(flameDepth, min(p + ivec2(0, 1), lim), 0).r,
                     texelFetch(flameDepth, min(p + ivec2(1, 1), lim), 0).r));
    } else {
        dz = texelFetch(flameDepth, ivec2(gl_FragCoord.xy), 0).r;
    }
    return flameViewZ(dz);
}

void flameVolume(float footM) {
    float halfW = vFlame.x;
    float heat = vFlame.z;
    float seed = vFlame.w;
    int style = int(clamp(vStyle, 0.0, kFlipStyles - 1.0) + 0.5);
    vec3 foot = vVolFoot.xyz;
    float frameH = vVolFoot.w;
    vec3 ax = vVolAxis.xyz;
    float len = max(vVolAxis.w, 0.05);
    vec3 lat = vVolLat.xyz;
    vec3 dep = cross(ax, lat);
    float kAbs = vVolLat.w;
    float reach = vVolDims.x, depth0 = vVolDims.y, sooty = vVolDims.z;
    // its puff's bulge: its size, the wave's turns a metre along the tongue, its phase at the foot (turns)
    float bulgeAmp = 0.0, bulgePerM = 0.0, bulgeTurns = 0.0;
    if (vVolDims.w > 0.5) {
        float code = floor(vVolDims.w);
        float ampCode = floor(code / 2048.0);
        bulgeTurns = vVolDims.w - code;
        bulgeAmp = kVolBulgeStep * ampCode;
        bulgePerM = 10.0 / max(code - 2048.0 * ampCode, 1.0);
    }
    float depthMax = depth0 * (1.0 + bulgeAmp); // the deepest the body gets, at a bulge
    float dist = max(length(vRel), 1e-3);
    vec3 d = vRel / dist;
    // The sight line's stretch through the body: within its reach of the
    // axis (closest at tc), and along the frame.
    float b = dot(d, ax), fd = dot(d, foot), fa = dot(ax, foot);
    float s2 = 1.0 - b * b;
    float tc = s2 > 1e-4 ? (fd - b * fa) / s2 : dot(d, foot + ax * (0.5 * frameH));
    float sc = b * tc - fa;
    vec3 cl = tc * d - foot - sc * ax;
    float rho2 = dot(cl, cl);
    if (rho2 >= reach * reach) discard;
    float halfT = sqrt(reach * reach - rho2) / max(sqrt(s2), 1e-3);
    float t0 = tc - halfT, t1 = tc + halfT;
    float topS = min(frameH, kVolTopShare * len);
    if (abs(b) > 1e-4) {
        float ta = tc + (-0.02 * frameH - sc) / b, tb = tc + (topS - sc) / b;
        t0 = max(t0, min(ta, tb));
        t1 = min(t1, max(ta, tb));
    }
    // ... and within the slab that holds its depth (the mask is nothing past the foot's half thickness)
    float yc = dot(tc * d - foot, dep), dy = dot(d, dep);
    if (abs(dy) > 1e-4) {
        float ya = tc + (-depthMax - yc) / dy, yb = tc + (depthMax - yc) / dy;
        t0 = max(t0, min(ya, yb));
        t1 = min(t1, max(ya, yb));
    } else if (abs(yc) >= depthMax) {
        discard;
    }
    t0 = max(t0, kVolNearM);
    if (t1 <= t0) discard;
    // its flipbook: the frame pair, the mip by the pixel's footprint (footM, m)
    float layer0 = float(style) * kFlipFrames;
    float f0 = mod(floor(vPhase), kFlipFrames);
    float f1 = mod(f0 + 1.0, kFlipFrames);
    float fr = fract(vPhase);
    float flip = fract(seed * 11.31) < 0.5 ? -1.0 : 1.0;
    float texelM = frameH / float(textureSize(flameBook, 0).y);
    // the steps: short enough across the depth and along the axis
    float stepT = min(kVolStepY * depth0 / max(abs(dy), 1e-3), kVolStepS * halfW / max(abs(b), 1e-3));
    int steps = clamp(int(ceil((t1 - t0) / max(stepT, 1e-3))), kVolStepsMin, kVolSteps);
    float dt = (t1 - t0) / float(steps);
    float hy = 0.5 * dt * abs(dy); // half a step's reach across the depth
    // (the mip: the pixel's footprint or the step's reach along the axis, whichever is coarser)
    float lod = flameOut.y > 0.5 ? 0.0
                                 : clamp(log2(max(max(footM, kVolLodStep * dt * abs(b)) / texelM, 1.0)) + kVolLodBias,
                                         0.0, kVolLodMax);
    // a coarse read over the stretch's whole span (a mip one texel of which covers it): nothing there, nothing drawn
    {
        float tm = 0.5 * (t0 + t1);
        vec3 vm = tm * d - foot;
        float spanT = max(abs(b) * (t1 - t0), abs(dot(d, lat)) * (t1 - t0)) / texelM;
        float lodSkip = clamp(log2(max(spanT, 8.0)) + 1.0, 3.0, 7.0);
        vec2 uvm = vec2(0.5 + 0.5 * flip * dot(vm, lat) / halfW, dot(vm, ax) / frameH);
        vec2 sk = max(textureLod(flameBook, vec3(uvm, layer0 + f0), lodSkip).rg,
                      textureLod(flameBook, vec3(uvm, layer0 + f1), lodSkip).rg);
        if (max(sk.r, sk.g) < 0.5 * kFlipEmpty) discard;
    }
    // the mottling's cells: the detail's (kDetailCell of the tongue, climbing with the gas: vScroll), twice as wide
    float cellD = clamp(kDetailCell * len, kDetailCellMin, kDetailCellMax);
    float mottle = kVolMottleK * (1.0 - smoothstep(kVolMottleNear, kVolMottleFar, footM / cellD));
    float occBody = mix(kVolBodyOccPow, kVolSootyOccPow, sooty);
    // the foot fades in over a ragged height (as the ribbon's), its rag across the body here
    float xc = dot(tc * d - foot, lat);
    vec2 rq = vec2(xc / max(kFlipBaseRagCell * len, 0.01) + floor(seed * 61.0), floor(seed * 17.0));
    float rag = 0.65 * detailNoise(rq) + 0.35 * detailNoise(rq * 2.3 + vec2(13.0, 7.0));
    float baseH = vBaseFade > 0.0 ? vBaseFade : clamp(kFlipBaseShare * len, kFlipBaseMinM, kFlipBaseMaxM);
    float base0 = vBaseFade > 0.0 ? baseH * kFlipBaseAirStart * detailNoise(rq * 1.7 + vec2(31.0, 5.0)) : 0.0;
    float base1 = base0 + baseH * mix(kFlipBaseRagLo, kFlipBaseRagHi, rag);
    // the scene behind: each sample's view depth is t's share of the quad's
    float sceneZ = flameVolumeSceneZ();
    float zPerT = flameDepthMap.y != 0.0 ? flameViewZ(gl_FragCoord.z) / dist : 1.0;
    float softM = max(kFlameSoftDepth, kVolSoftShare * reach);
    float nearFrom = min(kFlameNearFrom, kFlameNearFromFrames * frameH);
    float nearTo = min(kFlameNearTo, kFlameNearToFrames * frameH);
    float glow = kFlipGlow * kFlipStyleLight[style] * mix(kFlipDim, 1.0, heat);
    float sootHeat = mix(kFlipDimSoot, 1.0, heat);
    vec3 emit = vec3(0.0), smoke = vec3(0.0);
    float trans = 1.0, tauSmoke = 0.0, zSum = 0.0, wSum = 0.0;
    for (int i = 0; i < steps; ++i) {
        float t = t0 + (float(i) + 0.5) * dt;
        vec3 v = t * d - foot;
        float s = dot(v, ax);
        float x = dot(v, lat);
        float y = dot(v, dep);
        if (s <= 0.0 || s >= topS) continue;
        // the puff's bulge here (-1 a neck .. 1 the bulge), and the body's width and depth with it
        float bulge = bulgeAmp > 0.0 ? sin(6.2831853 * (bulgeTurns - s * bulgePerM)) *
                                           smoothstep(0.0, kVolBulgeFoot * len, s)
                                     : 0.0;
        float girth = 1.0 + bulgeAmp * bulge;
        if (abs(x) >= halfW * girth) continue;
        float wd = girth * depth0 * pow(max(1.0 - s / (kVolDepthTop * len), kVolDepthMin), kVolDepthPow);
        if (abs(y) - hy >= wd) continue; // the whole step outside the body's depth here
        vec2 uv = vec2(0.5 + 0.5 * flip * x / (halfW * girth), s / frameH);
        vec2 f = mix(textureLod(flameBook, vec3(uv, layer0 + f0), lod).rg,
                     textureLod(flameBook, vec3(uv, layer0 + f1), lod).rg, fr);
        if (max(f.r, f.g) < kFlipEmpty) continue;
        float yr = s / len;
        float k = kFlipK0 + kFlipKSpan * f.r;
        if (mottle > 0.0 && f.r > 0.0) {
            vec2 qa = vec2(x / (2.0 * cellD) + floor(seed * 97.0), s / cellD - vScroll);
            vec2 qb = vec2(y / (2.0 * cellD) + floor(seed * 53.0) + 17.0, s / cellD - vScroll + 11.0);
            k += mottle * (detailNoise(qa) + detailNoise(qb) - 1.0);
        }
        float zone = smoothstep(kFlipBodyLo, kFlipBodyHi, (k - kFlipSimKCold) / kFlipSimKPerT);
        float bodyRound = mix(kVolRoundLo, 1.0, sqrt(clamp(zone + kFlipTauMax * f.g, 0.0, 1.0)));
        // the depth mask's mean over the step's own reach across the depth (its half thickness here: thick)
        float thick = wd * bodyRound;
        float hu = max(hy, 1e-3 * thick) / thick, yu = y / thick;
        float mask = (flameVolMaskInt(yu + hu, 1.0) - flameVolMaskInt(yu - hu, 1.0)) / (2.0 * hu);
        float zv = t * zPerT;
        // its own soft edges (the zone's, the mask, the foot and top fades), then the scene's and the eye's
        float ends = smoothstep(base0, base1, s) * (1.0 - smoothstep(topS - kVolTopFade * len, topS, s));
        float shape = mask * ends;
        float keep = smoothstep(0.0, softM, sceneZ - zv) * smoothstep(nearFrom, nearTo, t);
        if (shape * keep <= 0.0) continue;
        float tip = sooty * smoothstep(kVolTipFrom, kVolTipTo, yr);
        // the luminous zone glows at its own absorption, and hides as (zone x its edges)^p: a sooty tongue's
        // edges more than they glow, its tips more still (the mask's edge to that power too, as its mean
        // over the step)
        float occPow = mix(occBody, kVolTipOccPow, tip);
        float maskHide = (flameVolMaskInt(yu + hu, occPow) - flameVolMaskInt(yu - hu, occPow)) / (2.0 * hu);
        float aEmit = 1.0 - exp(-kAbs * zone * shape * keep * dt);
        float aHide = 1.0 - exp(-kAbs * pow(zone * ends, occPow) * maskHide * keep * dt);
        keep *= shape;
        // its smoke outside the zone: the frame's side-on depth over the body's depth here
        float tauS = kFlipTauMax * f.g * sootHeat * (1.0 - zone) * smoothstep(0.0, kFlipSootFrom, yr) * keep * dt /
                     max(2.0 * kVolPathShare * wd, 0.01) * (1.0 + kVolSootBoost * tip);
        float sootT = exp(-tauS);
        float above = max(s - len, 0.0) / len;
        vec3 sm = flameSmokeLight(tauSmoke + 0.5 * tauS, kFlipSmokeFire * 0.6 * heat * exp(-above),
                                  kFlipStyleSmoke[style] * (1.0 - kVolSootDark * tip));
        vec3 e = flameBlackBody(k + vTemp.x) * (glow * aEmit * (1.0 + kVolBulgeGlow * bulge));
        emit += trans * e;
        smoke += trans * sm * (1.0 - sootT);
        float cover = 1.0 - (1.0 - aHide) * sootT;
        float w = trans * (cover + dot(e, kFlameLuma) / max(flameSceneScale(), 1e-6));
        zSum += w * zv;
        wSum += w;
        trans *= (1.0 - aHide) * sootT;
        tauSmoke += tauS;
        if (trans < 0.004) break;
    }
    if (wSum <= 0.0) discard;
    // Classic lighting: the eye's adaptation (as the ribbon's).
    float adapt = 1.0;
    if (litCamera.x < 0.5) {
        float white = dot(litSun.rgb * (max(litSunDir.y, 0.0) * litSunDir.w) + litZenith.rgb, kFlameLuma) *
                      kFlipBrightness * litZenith.w;
        adapt = clamp(1.0 / max(white, 1e-4), 1.0, kFlipClassicAdaptMax);
    }
    // (its near fade is its samples'; flameWrite's own is off)
    gFlameNearFrom = -2.0;
    gFlameNearTo = -1.0;
    if (flameDepthMap.y != 0.0) gFlameViewZ = zSum / wSum;
    flameWrite((smoke + emit * adapt) * flameSceneScale(), trans);
}

// ---- The burning bed (FireSpreadWorld.hpp FlameStyle::Bed; the realism audit's fix 7) ----
// The continuous zone at the fuel bed under a front and along its edges (the
// wildfire consult, 2026-11-16, second round 4): the hottest flame, 3 to 6
// times as bright as the flame above it, a jagged strip, never a smooth
// ribbon. Drawn procedurally on the bed's ribbon, a soft body: full along its
// middle (out to kBedCoreSide of its half width seen from the side, a solid
// strip, to kBedCore seen from above, a soft streak), its edges ragged by a noise
// along it (kBedFlickCell of its width) that flickers in place at kBedFlickHz
// (time is the noise's second axis: it never slides); fading in over
// kBedFootIn of its length from its foot and tapering out from kBedTipLo of
// its ragged tip, narrowing by kBedTaper toward its tip from a foot rounded
// over kBedRound of its length (a flame lying downwind), its tip rounded too
// and ragged across it (a noise kBedTipRagX cells a half width, follow-up
// 13: a straight cut across its end read as a lit rectangle's edge, worst at
// night where its light is far past the clip);
// broken into patches where the fuel is thin (a noise
// kBedBreakCell of its widths long, flickering in place: under kBedBreak it
// thins to kBedBreakFloorSide from the side, kBedBreakFloor from above); streaked along its axis (cells kBedStreakCell of
// its width long, kBedStreakAcross across, the dark streaks kBedStreakLo of the
// light) by streaks drifting along it at kBedDriftMps (the low flame licking
// downwind). Its light is the black body at kBedK (and the sheet's offset:
// cooler toward a band's back; its streaks kBedKSpread either way) at its
// emissivity times its heat (the share of the bed alight): the sheet's side-on
// emissivity, its optical depth times vGain (from straight above kBedTopGain
// of it: the bed is thin, so from above the ground shows through it, flame
// over black); by Kirchhoff it hides as much of what is behind it. Its soft
// fade against what is behind it spans kBedSoftShare of its half width.
// Near the eye it fades out from kBedNearFromSizes to kBedNearToSizes of its
// size (its length or width, the larger; never past kBedNearFromMax and
// kBedNearToMax): there the tongues standing on it show the fuel bed's flames
// themselves, and the bed, a coarse stand-in for them, magnified to metres
// across, read as a smooth slab of light (follow-up 13, night shot 15).
const float kBedK = 1430.0;
const float kBedKSpread = 60.0;
const float kBedFlickHz = 3.0;
const float kBedFlickCell = 0.7;
const float kBedEdgeLo = 0.5;
const float kBedCore = 0.35;
const float kBedCoreSide = 0.55;
const float kBedFootIn = 0.2;
const float kBedTipLo = 0.6;
const float kBedBreakCell = 0.9;
const float kBedBreak = 0.42;
const float kBedBreakFloor = 0.45;
const float kBedBreakFloorSide = 0.6;
const float kBedStreakCell = 1.2;
const float kBedStreakAcross = 0.25;
const float kBedStreakLo = 0.5;
const float kBedDriftMps = 0.5;
const float kBedSoftShare = 0.35;
const float kBedTaper = 0.6;
const float kBedRound = 0.25;
const float kBedTipRagX = 1.5;
const float kBedNearFromSizes = 3.0;
const float kBedNearToSizes = 7.0;
const float kBedNearFromMax = 10.0;
const float kBedNearToMax = 22.0;
const float kBedTopGain = 0.6; // vsFireFlames' same name: vGain from straight above
// (fix 08) the share of a bed's strip that glows on average (body's mean over
// its ribbon: 0.19 side on, 0.16 from above), its splat's light
const float kBedSplatBody = 0.18;

void flameBedMain() {
    float halfW = vFlame.x;
    float len = vFlame.y;
    float heat = vFlame.z;
    float seed = vFlame.w;
    float w = 2.0 * halfW;
    float time = flameFog.w;
    gFlameNearFrom = min(kBedNearFromSizes * max(len, w), kBedNearFromMax);
    gFlameNearTo = min(kBedNearToSizes * max(len, w), kBedNearToMax);
    float s = vSheet.y;
    float ft = mod(time * kBedFlickHz, kDetailPeriod);
    float ft2 = mod(time * 0.5 * kBedFlickHz, kDetailPeriod);
    vec2 q = vec2(s / (kBedFlickCell * w) + floor(seed * 97.0), ft + floor(seed * 53.0));
    float rag = 0.65 * detailNoise(q) + 0.35 * detailNoise(2.0 * q + vec2(13.0, 7.0));
    float u = s / max(len, 0.05);
    float tip = mix(0.75, 1.0, detailNoise(vec2(41.0 + floor(seed * 89.0) + kBedTipRagX * vSheet.x, ft)));
    float edge = mix(kBedEdgeLo, 1.0, rag);
    // (a flame lying downwind: a rounded foot, narrowing to a rounded tip)
    edge *= (1.0 - kBedTaper * smoothstep(0.25, 1.0, u)) * sqrt(smoothstep(0.0, kBedRound, u)) *
            sqrt(1.0 - smoothstep(kBedTipLo * tip, tip, u));
    // (seen from the side a solid strip, from above a soft streak: by how steeply the eye looks down, vGain's)
    float top = clamp((1.0 - vGain) / (1.0 - kBedTopGain), 0.0, 1.0);
    float across = 1.0 - smoothstep(mix(kBedCoreSide, kBedCore, top) * edge, edge, abs(vSheet.x));
    float along = smoothstep(0.0, kBedFootIn, u) * (1.0 - smoothstep(kBedTipLo * tip, tip, u));
    float gaps = detailNoise(vec2(s / (kBedBreakCell * w) + 0.6 * vSheet.x + floor(seed * 71.0), ft2 + floor(seed * 29.0)));
    float patches = mix(mix(kBedBreakFloorSide, kBedBreakFloor, top), 1.0, smoothstep(kBedBreak - 0.2, kBedBreak + 0.15, gaps));
    float drift = mod((s - time * kBedDriftMps) / (kBedStreakCell * w), kDetailPeriod);
    float st = detailNoise(vec2(vSheet.x / (2.0 * kBedStreakAcross) + floor(seed * 31.0), drift + floor(seed * 17.0)));
    float body = across * along * patches * mix(kBedStreakLo, 1.0, st);
    float splat = vFar.y; // (fix 08: the vertex shader's splat share)
    if (splat <= 0.0 && body * heat < 0.004) discard;
    float soft = flameSoftFadeOver(max(0.05, kBedSoftShare * halfW));
    float emis = 1.0 - pow(1.0 - min(vEmis, 0.999), vGain);
    float eps = emis * body * soft * heat;
    float k = kBedK + vTemp.x + kBedKSpread * (2.0 * st - 1.0);
    float adapt = flameAdapt();
    vec3 light = flameBlackBody(k) * (eps * adapt * flameSceneScale());
    float trans = 1.0 - eps;
    if (splat > 0.0) {
        // (fix 08) Far off, its splat: its mean light (kBedSplatBody of its
        // strip glows on average) times its area as shown, under the tent;
        // hidden only where the scene stands in front of its middle.
        float dist = length(vRel);
        float seen = flameDepthMap.y == 0.0
                         ? 1.0
                         : smoothstep(-max(kFlameSoftDepth, kSplatSoftShare * dist), 0.0,
                                      flameSceneGapAt(gl_FragCoord.xy - vSplat * kSplatRadiusPx));
        float wS = (1.0 - abs(vSplat.x)) * (1.0 - abs(vSplat.y)) * vFar.w * seen;
        float epsS = emis * heat * kBedSplatBody;
        vec3 lightS = flameBlackBody(kBedK + vTemp.x) * (epsS * adapt * flameSceneScale() * wS);
        float transS = 1.0 - min(epsS * wS, 1.0);
        light = mix(light, lightS, splat);
        trans = mix(trans, transS, splat);
    }
    flameWrite(light, trans);
}

// The near book's light (the flipbook's own fields, the fine detail close
// up): what the flame adds at the frame's scale and the share of what is
// behind it that stays. alone: nothing else lights this fragment, so the air
// round the tongue is discarded early (most of the ribbon); otherwise it
// works out to nothing added.
void flameNearLight(bool alone, out vec3 lightOut, out float transOut) {
    float halfW = gHalfW;
    float height = gHeight;
    float heat = gHeat;
    float seed = gSeed;
    int style = gStyle;
    float frameH = gFrameH;
    float flip = gFlip;
    float layer0 = gLayer0;
    float f0 = gF0;
    float f1 = gF1;
    // The fine detail (close up only): on the ribbon in metres, in noise
    // cells climbing with the gas, weighed by the pixel's footprint.
    vec2 m = vec2(vSheet.x * halfW, vSheet.y);
    float cell = clamp(kDetailCell * height, kDetailCellMin, kDetailCellMax);
    float foot = max(fwidth(m.x), fwidth(m.y));
    float closeUp = 1.0 - smoothstep(kDetailNearCells, kDetailFarCells, foot / cell);
    // Most of the ribbon is the air round the tongue: where a coarse read
    // (kFlipEmptyLod mips down, wider than the detail warp reaches) holds
    // nothing, nothing is drawn and nothing more is worked out.
    vec2 uv0 = vec2(0.5 + 0.5 * flip * vSheet.x, vSheet.y / frameH);
    vec2 sc = max(textureLod(flameBook, vec3(uv0, layer0 + f0), kFlipEmptyLod).rg,
                  textureLod(flameBook, vec3(uv0, layer0 + f1), kFlipEmptyLod).rg);
    if (alone && max(sc.r, sc.g) < kFlipEmpty) discard;
    vec2 dWarp = vec2(0.0);
    vec2 q = vec2(0.0);
    float ramp = 0.0;
    if (closeUp > 0.0) {
        q = m / cell + vec2(floor(seed * 97.0) + 0.37, floor(fract(seed * 3.31) * 53.0) - vScroll);
        ramp = smoothstep(0.0, kDetailRampUp * height, vSheet.y) * closeUp;
        vec2 w = vec2(detailNoise(q), detailNoise(q + vec2(17.1, 5.3))) - 0.5 +
                 0.5 * (vec2(detailNoise(q * 2.0 + vec2(31.0, 7.0)), detailNoise(q * 2.0 + vec2(3.0, 59.0))) - 0.5);
        dWarp = kDetailWarp * cell * ramp * vec2(w.x, kDetailWarpUp * w.y);
    }
    vec2 uv = vec2(0.5 + 0.5 * flip * (vSheet.x + dWarp.x / max(halfW, 1e-3)), (vSheet.y + dWarp.y) / frameH);
    // (flameOut.y 1: the debug read at the full resolution only, FLIPDEBUG.
    // A far or small tongue, whose mip would average its core with the air,
    // goes over to the far book: fix 08.)
    vec2 s;
    if (flameOut.y > 0.5)
        s = mix(textureLod(flameBook, vec3(uv, layer0 + f0), 0.0).rg,
                textureLod(flameBook, vec3(uv, layer0 + f1), 0.0).rg, fract(vPhase));
    else
        s = mix(texture(flameBook, vec3(uv, layer0 + f0)).rg, texture(flameBook, vec3(uv, layer0 + f1)).rg,
                fract(vPhase));
    // Most of the ribbon is the air round the tongue: nothing to draw there.
    if (alone && max(s.r, s.g) < kFlipEmpty) discard;
    // (a wide ribbon meets the ground or a trunk over a longer soft fade: its
    // plane cuts them along a line the scene's depth only blurs that far)
    float soft = flameSoftFadeOver(max(kFlameSoftDepth, kFlipSoftShare * halfW));
    // The foot fades in from the ground over a ragged height.
    vec2 rq = vec2(m.x / max(kFlipBaseRagCell * height, 0.01) + floor(seed * 61.0), floor(seed * 17.0));
    float rag = 0.65 * detailNoise(rq) + 0.35 * detailNoise(rq * 2.3 + vec2(13.0, 7.0));
    float baseH = vBaseFade > 0.0 ? vBaseFade : clamp(kFlipBaseShare * height, kFlipBaseMinM, kFlipBaseMaxM);
    // (a tongue standing in a crown, its foot in the air, starts its fade at a
    // ragged height too: no line where it begins)
    float base0 = vBaseFade > 0.0 ? baseH * kFlipBaseAirStart * detailNoise(rq * 1.7 + vec2(31.0, 5.0)) : 0.0;
    float base = smoothstep(base0, base0 + baseH * mix(kFlipBaseRagLo, kFlipBaseRagHi, rag), vSheet.y);
    float fade = base * soft;
    // The fields.
    float k = kFlipK0 + kFlipKSpan * s.r;
    float simT = (k - kFlipSimKCold) / kFlipSimKPerT;
    float yr = vSheet.y / max(height, 0.05);
    // (the field's change over a pixel: the crisp edge is never narrower)
    float simTW = fwidth(simT);
    float bodyLo = kFlipBodyLo, bodyHi = kFlipBodyHi;
    if (ramp > 0.0) {
        // Close up: the edge broken up by finer noise (only where the zone
        // already glows a little, mostly eating into it) and drawn crisper.
        float z0 = clamp((simT - kFlipBodyLo) / (kFlipBodyHi - kFlipBodyLo), 0.0, 1.0);
        simT += kDetailEdge * (detailNoise(q * 3.0 + vec2(71.0, 11.0)) - 0.6) * ramp * step(0.02, z0) * (1.0 - z0);
        float mid = 0.5 * (kFlipBodyLo + kFlipBodyHi);
        float half_ = max(0.5 * (kFlipBodyHi - kFlipBodyLo) * mix(1.0, kDetailBand, ramp), 0.75 * simTW);
        bodyLo = mid - half_;
        bodyHi = mid + half_;
    }
    float zone = smoothstep(bodyLo, bodyHi, simT);
    // The emissivity along this sight line: the side-on optical depth
    // (-ln(1 - vEmis)) times vGain.
    float emis = 1.0 - pow(1.0 - min(vEmis, 0.999), vGain);
    float eps = emis * zone * fade;
    // What it hides: at its soft edge the cooling soot hides more than it
    // glows (kEdgeOccPow); inside, Kirchhoff (as much as it glows).
    float occ = emis * pow(zone, kEdgeOccPow) * fade;
    float tau = kFlipTauMax * s.g * mix(kFlipDimSoot, 1.0, heat) * fade * vGain;
    // The soot glowing in the luminous zone is its emissivity; what the
    // medium adds is the smoke outside it (no grey veil over the flame's body).
    tau *= (1.0 - zone) * smoothstep(0.0, kFlipSootFrom, yr);
    // The luminous zone glows at its emissivity and hides kFlipGasOcclusion
    // of that of what is behind it (Kirchhoff: all of it); about half the
    // smoke round it lies in front of it.
    float kLight = k + vTemp.x;
    vec3 emit = flameBlackBody(kLight) * (eps * kFlipGlow * kFlipStyleLight[style] * mix(kFlipDim, 1.0, heat));
    float sootT = exp(-tau);
    float above = max(vSheet.y - height, 0.0) / max(height, 0.1);
    vec3 smoke = flameSmokeLight(tau, kFlipSmokeFire * 0.6 * heat * exp(-above), kFlipStyleSmoke[style]) * (1.0 - sootT);
    // (classic lighting: the eye's adaptation to the scene's white level)
    lightOut = (smoke + emit * (sqrt(sootT) * flameAdapt())) * flameSceneScale();
    transOut = (1.0 - kFlipGasOcclusion * occ) * sootT;
}

void main() {
    // (fix 6) a big tongue: a volume (its pixel's footprint here, before any branch or discard)
    float volFootM = max(length(dFdx(vRel)), length(dFdy(vRel)));
    if (vVolLat.w > 0.0) {
        flameVolume(volFootM);
        return;
    }
    // (fix 7) the burning bed: its own procedural flame, no flipbook
    if (vStyle > 3.5) {
        flameBedMain();
        return;
    }
    float halfW = vFlame.x;
    float height = vFlame.y;
    float heat = vFlame.z;
    float seed = vFlame.w;
    int style = int(clamp(vStyle, 0.0, kFlipStyles - 1.0) + 0.5);
    // The frame: the tongue's length over its bottom kFlipFlameShare, the
    // ground at its fuel bed; across, the ribbon's width, flipped by the seed.
    float frameH = height / kFlipFlameShare[style];
    gFlameNearFrom = min(kFlameNearFrom, kFlameNearFromFrames * frameH);
    gFlameNearTo = min(kFlameNearTo, kFlameNearToFrames * frameH);
    float flip = fract(seed * 11.31) < 0.5 ? -1.0 : 1.0;
    float layer0 = float(style) * kFlipFrames;
    float f0 = mod(floor(vPhase), kFlipFrames);
    float f1 = mod(f0 + 1.0, kFlipFrames);
    gHalfW = halfW;
    gHeight = height;
    gHeat = heat;
    gSeed = seed;
    gStyle = style;
    gFrameH = frameH;
    gFlip = flip;
    gLayer0 = layer0;
    gF0 = f0;
    gF1 = f1;
    // Near, far, or a splat (fix 08: the vertex shader's vFar, the same over
    // the whole tongue), each alone where it can be, mixed between.
    float splat = vFar.y, farShare = vFar.z;
    vec3 light;
    float trans;
    if (splat <= 0.0 && farShare <= 0.0) {
        flameNearLight(true, light, trans);
    } else if (splat <= 0.0 && farShare >= 1.0) {
        flameFarLight(true, light, trans);
    } else if (splat >= 1.0) {
        flameSplatLight(light, trans);
    } else {
        vec3 lN = vec3(0.0), lF = vec3(0.0), lS = vec3(0.0);
        float tN = 1.0, tF = 1.0, tS = 1.0;
        if (farShare < 1.0) flameNearLight(false, lN, tN);
        if (farShare > 0.0) flameFarLight(false, lF, tF);
        if (splat > 0.0) flameSplatLight(lS, tS);
        light = mix(mix(lN, lF, farShare), lS, splat);
        trans = mix(mix(tN, tF, farShare), tS, splat);
    }
    flameWrite(light, trans);
}
)";

// The merge (EngineGL33_FireFlames.cpp MergeFireFlames): the linear flame
// target (half the frame's size) onto the frame as the post pass holds it,
// decoded (camDecode, or the display's gamma), added, coded again; pixels
// without flames are copied as they are.
//   - The upsample: the four half-resolution texels round the pixel,
//     bilinear, each kept only as far as the full-resolution scene here lies
//     behind that texel's flames (their weighted depth): a trunk or a man in
//     front of a fire cuts it at its own edge, not at the half resolution's.
//   - The fog: with the volumetric fog this frame (flameMerge.z), the flames'
//     light dims by the medium's transmittance up to their depth and what
//     they hide is replaced by the air's light in front of them (the frame
//     already holds the fog up to the scene behind them):
//     C' = C a + L T(zf) + S(zf) (1 - a), with the volume read as psFogApply
//     reads it (fogAt).
//   - The TAA's particle mask (flameMerge.w): the opaque world copy it
//     compares the world with is moved away from the world by the flames'
//     change of the frame, so the resolve treats flames as particles (they
//     never pass through the frame target the mask reads).
//   - Smoke over the flames (fix 02): the smoke drawn after the flames in
//     front of them (frontTex: F, its colour as the frame blended it, coded,
//     and tF, the share of their light it leaves, exp(-optical depth) along
//     the view) dims their light by tF in linear light and stays over them;
//     the flames hide only what lies behind that smoke, P = C - F (decoded,
//     F at most the frame), never the smoke itself:
//     C' = C - P (1 - a) + S tF (1 - a) + L T tF. Without smoke in front
//     (tF 1, F 0) it is the line above. (Reading back what lies behind as
//     (c - F) / tF blew up where tF is small and the frame carries the
//     volumetric fog the smoke's target lacks: pale blobs at night.)
// With flameMerge.y 1 (no post stage to merge before) the upsampled target
// is instead laid over the frame by blending, coded.
const char kFireFlameMergeGLSL[] = R"(
in vec2 vUV;
uniform sampler2D sceneTex;  // unit 0: the frame (coded)
uniform sampler2D flameTex;  // unit 2: the flames (half size): rgb the light they add (the frame's scale), a what stays
uniform sampler2D flameZTex; // unit 3: their depth (half size): r the weighted view depth (km), g the weight
uniform sampler2D depthTex;  // unit 4: the world's depth (full size)
uniform sampler2D worldTex;  // unit 5: the world as resolved, which the TAA's mask reads
uniform sampler3D fogVolume; // unit 6: the volumetric fog's integration (rgb light toward the eye, a transmittance)
uniform vec4 flameMerge;     // x 1 = the photoreal camera's coding, y 1 = blend over the frame, z 1 = fog volume, w 1 = TAA mark
uniform vec4 flameProj;      // x _33, y _43 (view z = y / (ndc z - x)), z w: ndc z = depth z + w
uniform vec4 flameFogDepth;  // x the volume's far end (m), y its slice spacing k, z its slices
uniform vec4 flameRect;      // the 3D viewport in uv
uniform vec4 flameMergeDebug; // x 1 = keep every texel (FLAMEDEBUG 4), y flame texels per frame pixel (0.5), z 1 = show tF (FLAMEDEBUG 16), w 1 = tiles (FLAMEDEBUG 32)
uniform sampler2D frontTex;  // the smoke in front of the flames (half size, fix 02): rgb its colour (coded, laid over), a tF
// The camera's clip on the flames' light (PhotoCamera.hpp FlameFilm): x 1 on
// (the photoreal camera), y the exposure the composite applies (the frame's
// light times it is scene light after exposure), z the knee (camFlameKnee), w
// the clipped channels' spill (kFlameSpill).
uniform vec4 flameFilmK;
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragMark; // into the TAA's opaque copy (blended by its alpha), with flameMerge.w
)"
                                   R"(
// ---- The flames' clip: a camera's channels on the flames' light ----
// A camera's channels clip one by one (PhotoCamera.hpp FlameFilm,
// FlameKneeCurve): each channel rolls off toward its clip on its own from
// kFlameKneeSoft of the knee, so a flame's red is held while its green climbs
// (yellow), and the clipped channels' excess spills into every channel (the
// sensor's crosstalk: white, about four stops past the knee). The peak
// channel's light is kept, so the composite's hue-keeping curve shows the
// ratios this makes. Nothing changes under half the knee.
const float kFlameKneeSoft = 0.5;
float flameKneeCurve(float u) {
    if (u <= kFlameKneeSoft) return u;
    return 1.0 - (1.0 - kFlameKneeSoft) * (1.0 - kFlameKneeSoft) / (u - 2.0 * kFlameKneeSoft + 1.0);
}
vec3 flameFilm(vec3 light) {
    light = max(light, vec3(0.0));
    float peak = max(light.r, max(light.g, light.b));
    if (flameFilmK.x < 0.5 || peak * flameFilmK.y <= kFlameKneeSoft * flameFilmK.z) return light;
    vec3 u = light * (flameFilmK.y / flameFilmK.z);
    float spill = flameFilmK.w * dot(max(u - 1.0, vec3(0.0)), vec3(1.0));
    vec3 f = vec3(flameKneeCurve(u.r + spill), flameKneeCurve(u.g + spill), flameKneeCurve(u.b + spill));
    return f * (peak / max(max(f.r, max(f.g, f.b)), 1e-6));
}
)"
                                   R"(
// A texel's flames are kept in full while the scene lies behind their depth,
// faded out over this much in front of it (m, or this share of the depth).
const float kMergeKeepNear = 1.0;
const float kMergeKeepShare = 0.02;
const float kMergeMarkMin = 2.0 / 255.0; // the TAA mask's low threshold (TemporalAA.hpp kTaaReactiveLow)
// Smoke over the flames (fix 02): under this share of the flames' light left
// there is smoke in front of them.
const float kFrontNone = 0.9995;

float mergeViewZ(float d) {
    return d >= 0.999999 ? 1e6 : flameProj.y / ((d * flameProj.z + flameProj.w) - flameProj.x);
}

void main() {
    ivec2 p = ivec2(gl_FragCoord.xy);
    float sceneZ = mergeViewZ(texelFetch(depthTex, p, 0).r);
    ivec2 lim = textureSize(flameTex, 0) - 1;
    vec2 hp = (vec2(p) + 0.5) * flameMergeDebug.y - 0.5;
    ivec2 b = ivec2(floor(hp));
    vec2 fr = hp - vec2(b);
    vec3 light = vec3(0.0);
    float trans = 0.0;
    vec2 zw = vec2(0.0);
    vec3 front = vec3(0.0); // smoke over the flames (fix 02): its colour, and tF
    float tF = 0.0;
    vec3 lightOver = vec3(0.0); // the flames' light without the smoke in front (FLAMEDEBUG 32 only)
    for (int k = 0; k < 4; ++k) {
        ivec2 o = ivec2(k & 1, k >> 1);
        ivec2 q = clamp(b + o, ivec2(0), lim);
        float bw = (o.x == 1 ? fr.x : 1.0 - fr.x) * (o.y == 1 ? fr.y : 1.0 - fr.y);
        vec4 v = texelFetch(flameTex, q, 0);
        vec2 z = texelFetch(flameZTex, q, 0).rg;
        vec4 sv = texelFetch(frontTex, q, 0);
        float keep = 1.0;
        if (z.y > 1e-5) {
            float zf = 1000.0 * z.x / z.y;
            float m = max(kMergeKeepNear, kMergeKeepShare * zf);
            keep = flameMergeDebug.x > 0.5 ? 1.0 : clamp((sceneZ - zf + m) / m, 0.0, 1.0);
            zw += (bw * keep) * z;
        }
        light += (bw * keep * sv.a) * v.rgb;
        lightOver += (bw * keep) * v.rgb;
        trans += bw * (keep * v.a + (1.0 - keep));
        front += (bw * keep) * sv.rgb;
        tF += bw * (keep * sv.a + (1.0 - keep));
    }
    bool camera = flameMerge.x > 0.5;
    if (flameMerge.y > 0.5) {
        vec3 code = camera ? camFold(flameFilm(light)) : pow(flameTonemap(light), vec3(1.0 / 2.2));
        fragColor = vec4(code, 1.0 - tF * (1.0 - trans));
        fragMark = vec4(0.0);
        return;
    }
    vec4 c = texelFetch(sceneTex, p, 0);
    if (trans >= 1.0 && max(light.r, max(light.g, light.b)) <= 0.0) {
        fragColor = c;
        fragMark = vec4(0.0);
        return;
    }
    vec3 S = vec3(0.0);
    float T = 1.0;
    if (flameMerge.z > 0.5 && zw.y > 1e-5) {
        float zf = 1000.0 * zw.x / zw.y;
        vec2 cuv = (vUV - flameRect.xy) / flameRect.zw;
        float t = flameFogDepth.y < 1e-3 ? zf / flameFogDepth.x
                                         : log(1.0 + zf * (exp(flameFogDepth.y) - 1.0) / flameFogDepth.x) / flameFogDepth.y;
        float f = clamp(t, 0.0, 1.0) * flameFogDepth.z;
        vec4 v = textureLod(fogVolume, vec3(cuv, (f - 0.5) / flameFogDepth.z), 0.0);
        float k0 = clamp(f, 0.0, 1.0);
        S = max(v.rgb, vec3(0.0)) * k0;
        T = mix(1.0, clamp(v.a, 0.0, 1.0), k0);
    }
    // The flames' light through the fog, as the camera's clip shows it.
    vec3 flame = camera ? flameFilm(light * T) : light * T;
    vec3 lin = (camera ? camDecode(c.rgb) : flameToLinear(c.rgb)) * trans + flame + S * (1.0 - trans);
    // ---- Smoke over the flames (fix 02) ----
    // light is already dimmed by the smoke in front (tF), so the camera's
    // clip above (fix 01) sees the flames as the smoke leaves them.
    if (tF < kFrontNone) {
        vec3 cl = camera ? camDecode(c.rgb) : flameToLinear(c.rgb);
        vec3 fl = camera ? camDecode(min(front, c.rgb)) : flameToLinear(min(front, c.rgb));
        vec3 behind = max(cl - fl, vec3(0.0));
        lin = cl - behind * (1.0 - trans) + S * (tF * (1.0 - trans)) + flame;
    }
    if (flameMergeDebug.z > 0.5)
        lin = camera ? camDecode(vec3(tF)) : flameToLinear(vec3(tF));
    // FLAMEDEBUG 32: 8 px tiles in turn show this merge, the flames laid
    // over the smoke as before (the old line, their light undimmed), and tF,
    // so one frame measures all three side by side.
    if (flameMergeDebug.w > 0.5) {
        int tile = (int(gl_FragCoord.x) / 8 + int(gl_FragCoord.y) / 8) % 3;
        if (tile == 1)
            lin = (camera ? camDecode(c.rgb) : flameToLinear(c.rgb)) * trans +
                  (camera ? flameFilm(lightOver * T) : lightOver * T) + S * (1.0 - trans);
        else if (tile == 2)
            lin = camera ? camDecode(vec3(tF)) : flameToLinear(vec3(tF));
    }
    // ---- end smoke over the flames ----
    vec3 code = camera ? camFold(lin) : pow(flameTonemap(lin), vec3(1.0 / 2.2));
    fragColor = vec4(code, c.a);
    fragMark = vec4(0.0);
    if (flameMerge.w > 0.5) {
        vec3 dc = abs(code - c.rgb);
        float d = min(max(dc.r, max(dc.g, dc.b)), 0.5);
        if (d > kMergeMarkMin) {
            vec3 w = texelFetch(worldTex, p, 0).rgb;
            fragMark = vec4(clamp(w + mix(vec3(d), vec3(-d), step(vec3(0.5), w)), 0.0, 1.0), 1.0);
        }
    }
}
)";

// The flames' depth for the smoke in front of them (EngineGL33_FireFlames.cpp
// FillFlameCoverDepth): half resolution, the depth buffer the smoke drawn
// after the flames is tested against, at their weighted depth where they
// are, at the near plane (nothing passes) where there are none. That depth is
// dithered (interleaved gradient noise, a new phase each frame, the TAA
// averaging it) by up to flameCoverDither.x of itself, at least .y metres,
// either way: a flat smoke puff crossing the flames' depth would otherwise
// dim them up to a straight line.
const char kFireFlameCoverDepthGLSL[] = R"(
uniform sampler2D flameZTex; // r the weighted view depth (km), g the weight
uniform vec4 flameProj;      // x _33, y _43 (view z = y / (ndc z - x)), z w: ndc z = depth z + w
uniform vec4 flameCoverDither; // x the dither's reach (a share of the depth), y at least (m), z the frame's phase (0..1)
void main() {
    vec2 z = texelFetch(flameZTex, ivec2(gl_FragCoord.xy), 0).rg;
    if (z.y <= 1e-5) {
        gl_FragDepth = 0.0;
        return;
    }
    float zf = 1000.0 * z.x / z.y;
    vec2 px = gl_FragCoord.xy + 5.588238 * floor(flameCoverDither.z * 64.0);
    float ign = fract(52.9829189 * fract(dot(px, vec2(0.06711056, 0.00583715))));
    zf += (2.0 * ign - 1.0) * max(flameCoverDither.y, flameCoverDither.x * zf);
    float ndc = flameProj.x + flameProj.y / max(zf, 0.01);
    gl_FragDepth = clamp((ndc - flameProj.w) / flameProj.z, 0.0, 1.0);
}
)";

} // namespace

void FlameFarTexel(unsigned char r, unsigned char g, float out[4])
{
    // psFireFlamesBook's kelvin, zone and cover (kFlipK0, kFlipKSpan,
    // kFlipSimKCold, kFlipSimKPerT, kFlipBodyLo, kFlipBodyHi, kEdgeOccPow) and
    // the far book's luminance law (kFarRefCoreK, kFarRefSlopeK): keep equal.
    constexpr float kK0 = 600.0f, kKSpan = 1000.0f, kSimKCold = 1150.0f, kSimKPerT = 330.0f;
    constexpr float kBodyLo = 0.12f, kBodyHi = 0.45f, kEdgeOccPow = 0.75f;
    constexpr float kRefCoreK = 1650.0f, kRefSlopeK = 16500.0f;
    const float k = kK0 + kKSpan * (static_cast<float>(r) / 255.0f);
    const float t = std::clamp(((k - kSimKCold) / kSimKPerT - kBodyLo) / (kBodyHi - kBodyLo), 0.0f, 1.0f);
    const float zone = t * t * (3.0f - 2.0f * t);
    out[0] = zone;
    out[1] = zone * std::exp(kRefSlopeK * (1.0f / kRefCoreK - 1.0f / k));
    out[2] = zone > 0.0f ? std::pow(zone, kEdgeOccPow) : 0.0f;
    out[3] = (static_cast<float>(g) / 255.0f) * (1.0f - zone);
}

const char* FireFlameVertexSource(bool flipbook)
{
    static const std::string procedural = std::string("#version 330 core\n") + kFireFlameVertexGLSL;
    static const std::string book = std::string("#version 330 core\n#define FLAME_FLIPBOOK 1\n") + kFireFlameVertexGLSL;
    return flipbook ? book.c_str() : procedural.c_str();
}

const char* FireFlameFragmentSource(bool flipbook)
{
    static const std::string head = std::string("#version 330 core\n") + LitConstantsBlock() + kCameraCodecGLSL +
                                    kFireFlameTonemapGLSL + kFireFlameCommonGLSL;
    static const std::string procedural = head + kFireFlameFragmentGLSL;
    static const std::string book = head + kFireFlameFlipbookGLSL;
    return flipbook ? book.c_str() : procedural.c_str();
}

const char* FireFlameMergeSource()
{
    static const std::string s =
        std::string("#version 330 core\n") + kCameraCodecGLSL + kFireFlameTonemapGLSL + kFireFlameMergeGLSL;
    return s.c_str();
}

const char* FireFlameCoverDepthSource()
{
    static const std::string s = std::string("#version 330 core\n") + kFireFlameCoverDepthGLSL;
    return s.c_str();
}

} // namespace Poseidon::gl33lighting
