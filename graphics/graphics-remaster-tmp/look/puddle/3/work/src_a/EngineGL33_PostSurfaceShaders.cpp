// Remastered lighting, the look lane "surface" (Graphics/Remaster/
// SurfaceLight.hpp): the post pass's horizon-based ambient occlusion. GLSL
// only, no GL code (the unit tests compile it through PostShaderSource); the
// pass runs in EngineGL33_PostFX.cpp (PostAo).

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

// Ground-truth-based ambient occlusion (Jimenez et al. 2016, as XeGTAO lays it
// out) at half resolution. Per pixel kGtaoSlices slices through the view
// vector, turned per pixel (interleaved gradient noise) and per TAA frame
// (gtaoParams.w); in each slice kGtaoSteps samples per side, quadratically
// spaced out to the world radius (capped on screen), find the highest horizon,
// and the slice's visibility is the cosine-weighted integral of the open arc
// around the projected normal. A sample counts as an occluder by three
// weights: distance (it fades out over the last 60% of the radius, so far
// geometry does not occlude), thickness (one in front of the receiver by more
// than a thin object's depth, a crown or trunk before a wall or the grass
// behind it, is a separate object the sky reaches behind, and fades out), and
// height (one below the receiver is the ground: it bounces the light it
// hides, so it counts at upView.w). Samples are clamped to the 3D viewport:
// past its edge the edge's own depth stands in (counting them as open left a
// lighter rim along the frame's border, as far depth did a brighter one).
// Under TAA every pixel takes gtaoSlices.x slices (two; the accumulation
// over frames, psPostGtaoAccum, covers the rest); without it kSlices, near
// pixels whose search spans more than gtaoParams.z half-resolution pixels
// one fewer (their samples are far apart and cost the most). Output r =
// visibility (1 = open), g = view depth in metres (the bilateral blur and the
// upsample read it). Inputs, then the shared depth reconstruction
// (s_glslPostCommon), then this.
const char kPostGtaoInputsGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D depthTex;
uniform vec4 projInfo;   // x = _33, y = _43 (viewZ = y / (ndcZ - x)), z = 1/_11, w = 1/_22
uniform vec4 depthMap;   // ndcZ = depth * x + y
uniform vec4 aoParams;   // x radius (m), y its multiple on static architecture, z fade start (m), w fade end (m)
uniform sampler2D surfTex; // the resolved surface target: a 1 = static architecture (SurfaceLight.hpp)
uniform vec4 screen;     // xy full-res size, zw 1/size
uniform vec4 worldRect;  // xy origin, zw size of the 3D viewport in uv
uniform vec4 gtaoParams; // x max screen radius (share of the height), y min radius (pixels), z one-slice radius (half-res px), w frame turn
uniform vec4 upView;     // xyz world up in view space, w the weight of an occluder below the receiver
uniform vec4 gtaoSlices; // x slices a pixel this frame (kGtaoTemporalSlices under TAA, else kGtaoSlices)
out vec4 fragColor;
)";

const char kPostGtaoMainGLSL[] = R"(
const float kPi = 3.14159265;
const int kSlices = 3;
const int kSteps = 8;

float gtaoNoise(vec2 p) {
    return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

// Every depth read is snapped to a full-resolution texel centre and its
// position rebuilt there: a position at the exact uv with the depth of the
// nearest texel sits off the surface on a grazing plane (the ground ahead), so
// open flat ground read as about 8% occluded.
vec2 snapUV(vec2 uv) {
    return (floor(uv * screen.xy) + 0.5) * screen.zw;
}

vec3 sampleView(vec2 uv) {
    vec2 s = snapUV(uv);
    return viewPos(s, viewZ(s));
}

void main() {
    vec2 uvC = snapUV(vUV);
    float d0 = texture(depthTex, uvC).r;
    if (d0 >= 0.999999) { fragColor = vec4(1.0, 1e6, 0.0, 1.0); return; } // sky
    float z = viewZ(uvC);
    if (z > aoParams.w) { fragColor = vec4(1.0, z, 0.0, 1.0); return; }
    vec3 p = viewPos(uvC, z);

    // Normal from depth: on each axis the neighbour with the smaller step, so
    // silhouettes do not bend it.
    vec2 tx = vec2(screen.z, 0.0);
    vec2 ty = vec2(0.0, screen.w);
    vec3 pr = sampleView(uvC + tx);
    vec3 pl = sampleView(uvC - tx);
    vec3 pu = sampleView(uvC + ty);
    vec3 pd = sampleView(uvC - ty);
    vec3 dx = abs(pr.z - p.z) < abs(p.z - pl.z) ? pr - p : p - pl;
    vec3 dy = abs(pu.z - p.z) < abs(p.z - pd.z) ? pu - p : p - pd;
    vec3 n = normalize(cross(dx, dy));
    vec3 v = normalize(-p);
    if (dot(n, v) < 0.0) n = -n; // face the camera

    // The search radius in uv: the world radius at this depth, capped on
    // screen (near pixels would otherwise sample the whole frame), and at least
    // a few pixels (far pixels keep a contact term).
    // Static architecture (the surface target's alpha code 1) searches
    // farther: eaves, door openings and the wall-ground seam are metres across.
    float radius = aoParams.x * mix(1.0, aoParams.y, smoothstep(0.5, 1.0, texture(surfTex, uvC).a));
    vec2 uvPerM = vec2(0.5 / projInfo.z, 0.5 / projInfo.w) * worldRect.zw / z;
    vec2 rUV = radius * uvPerM;
    float cap = gtaoParams.x * worldRect.w;
    float fit = min(1.0, cap / max(rUV.y, 1e-6));
    rUV *= fit;
    float worldR = radius * fit; // the radius the capped search covers
    float minPx = gtaoParams.y * screen.w;
    if (rUV.y < minPx) { rUV *= minPx / max(rUV.y, 1e-9); }
    float falloffRange = 0.6 * worldR;
    float falloffMul = -1.0 / max(falloffRange, 1e-4);
    float falloffAdd = (worldR - falloffRange) / max(falloffRange, 1e-4) + 1.0;
    // Thickness: an occluder this far in front of the receiver (view depth)
    // starts to fade, and is gone at the second.
    float thick0 = 0.2 * worldR;
    float thick1 = 0.7 * worldR;
    int base = clamp(int(gtaoSlices.x + 0.5), 1, kSlices);
    int slices = (base == kSlices && rUV.y * screen.y * 0.5 > gtaoParams.z) ? kSlices - 1 : base;

    vec2 uvLo = worldRect.xy + screen.zw * 0.5;
    vec2 uvHi = worldRect.xy + worldRect.zw - screen.zw * 0.5;
    float noiseA = fract(gtaoNoise(gl_FragCoord.xy) + gtaoParams.w);
    float noiseB = fract(gtaoNoise(gl_FragCoord.yx + vec2(17.0, 5.0)) + gtaoParams.w * 1.618);
    float visibility = 0.0;
    float openSum = 0.0;
    for (int s = 0; s < kSlices; ++s) {
        if (s >= slices) break;
        float phi = (float(s) + noiseA) * (kPi / float(slices));
        vec2 dir = vec2(cos(phi), sin(phi));
        vec3 dir3 = vec3(dir, 0.0);
        vec3 ortho = dir3 - dot(dir3, v) * v;
        vec3 axis = normalize(cross(ortho, v));
        vec3 projN = n - axis * dot(n, axis);
        float projNLen = length(projN);
        float sgnN = sign(dot(ortho, projN));
        float cosN = clamp(dot(projN, v) / max(projNLen, 1e-5), 0.0, 1.0);
        float nAng = sgnN * acos(cosN);
        float low0 = cos(nAng + 0.5 * kPi);
        float low1 = cos(nAng - 0.5 * kPi);
        float h0c = low0;
        float h1c = low1;
        for (int k = 0; k < kSteps; ++k) {
            float t = (float(k) + noiseB) / float(kSteps);
            t = t * t;
            vec2 o = dir * rUV * t + dir * screen.zw * 1.5; // skip the pixel itself
            vec2 uv0 = clamp(uvC + o, uvLo, uvHi);
            vec2 uv1 = clamp(uvC - o, uvLo, uvHi);
            vec3 s0 = sampleView(uv0) - p;
            vec3 s1 = sampleView(uv1) - p;
            float l0 = max(length(s0), 1e-5);
            float l1 = max(length(s1), 1e-5);
            float c0 = dot(s0, v) / l0;
            float c1 = dot(s1, v) / l1;
            float w0 = clamp(l0 * falloffMul + falloffAdd, 0.0, 1.0) * (1.0 - smoothstep(thick0, thick1, -s0.z)) *
                       mix(upView.w, 1.0, smoothstep(-0.25, 0.05, dot(s0, upView.xyz) / l0));
            float w1 = clamp(l1 * falloffMul + falloffAdd, 0.0, 1.0) * (1.0 - smoothstep(thick0, thick1, -s1.z)) *
                       mix(upView.w, 1.0, smoothstep(-0.25, 0.05, dot(s1, upView.xyz) / l1));
            c0 = mix(low0, c0, w0);
            c1 = mix(low1, c1, w1);
            h0c = max(h0c, c0);
            h1c = max(h1c, c1);
        }
        projNLen = mix(projNLen, 1.0, 0.05);
        float h0 = -acos(clamp(h1c, -1.0, 1.0));
        float h1 = acos(clamp(h0c, -1.0, 1.0));
        h0 = nAng + clamp(h0 - nAng, -0.5 * kPi, 0.5 * kPi);
        h1 = nAng + clamp(h1 - nAng, -0.5 * kPi, 0.5 * kPi);
        float sinN = sin(nAng);
        float arc0 = (cosN + 2.0 * h0 * sinN - cos(2.0 * h0 - nAng)) * 0.25;
        float arc1 = (cosN + 2.0 * h1 * sinN - cos(2.0 * h1 - nAng)) * 0.25;
        visibility += projNLen * (arc0 + arc1);
        // The same slice with nothing in it: the cosine-weighted integral is
        // 1 only on average over every slice direction, and one or two slices
        // clamped to 1 read open ground seen at a grazing angle as about 10%
        // occluded. Their ratio is 1 wherever nothing occludes.
        openSum += projNLen * (cosN + nAng * sinN);
    }
    visibility = clamp(visibility / max(openSum, 1e-4), 0.0, 1.0);
    visibility = mix(visibility, 1.0, smoothstep(aoParams.z, aoParams.w, z));
    fragColor = vec4(visibility, z, 0.0, 1.0);
}
)";

// The occlusion accumulated over TAA frames: each half-resolution texel's
// position (its view depth, g) taken into the last frame by the TAA resolve's
// camera mapping, the history read there kept at accum.x where it held the
// same surface (its depth within 3% plus 10 cm), this frame's blurred
// occlusion mixed in at the rest. A static camera's turning slice pattern
// then settles instead of moving frame to frame. Output as the input: r the
// occlusion, g the view depth.
const char kPostGtaoAccumGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D curTex;  // this frame's blurred occlusion (r) and view depth (g)
uniform sampler2D histTex; // the last frames' (r) and the depth it was found at (g)
uniform vec4 projInfo;     // x = _33, y = _43, z = 1/_11, w = 1/_22
uniform vec4 worldRect;    // xy origin, zw size of the 3D viewport in uv
uniform vec4 reproj0;      // previous view position = (dot(row.xyz, p) + row.w) per row
uniform vec4 reproj1;
uniform vec4 reproj2;
uniform vec4 prevProj;     // xy _11 and _22 of the previous frame
uniform vec4 accum;        // x the history's weight (0 = none: this frame alone)
out vec4 fragColor;
void main() {
    vec2 c = texture(curTex, vUV).rg;
    fragColor = vec4(c, 0.0, 1.0);
    if (accum.x <= 0.0 || c.g > 1e5) return;
    vec2 ndc = (vUV - worldRect.xy) / worldRect.zw * 2.0 - 1.0;
    vec3 p = vec3(ndc.x * c.g * projInfo.z, ndc.y * c.g * projInfo.w, c.g);
    vec3 pp = vec3(dot(reproj0.xyz, p) + reproj0.w, dot(reproj1.xyz, p) + reproj1.w, dot(reproj2.xyz, p) + reproj2.w);
    if (pp.z <= 0.05) return;
    vec2 prevNdc = pp.xy * prevProj.xy / pp.z;
    vec2 huv = worldRect.xy + (prevNdc * 0.5 + 0.5) * worldRect.zw;
    if (any(lessThan(huv, worldRect.xy)) || any(greaterThan(huv, worldRect.xy + worldRect.zw))) return;
    vec2 h = texture(histTex, huv).rg;
    if (abs(h.g - pp.z) > 0.03 * pp.z + 0.1) return;
    fragColor = vec4(mix(c.r, h.r, accum.x), c.g, 0.0, 1.0);
}
)";

} // namespace Poseidon::gl33lighting
