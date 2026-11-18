// The volumetric clouds' shaders (the experiment POSEIDON_XP_CLOUDS;
// EngineGL33_XpClouds.cpp runs them and says what they do). No GL code, so
// the unit tests compile the sources the engine builds.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <string>

namespace Poseidon::gl33lighting
{

namespace
{

// ---- The march (half resolution) ---------------------------------------------
const char kXpCloudsMarchHead[] = R"(#version 330 core
in vec2 vUV;
uniform sampler3D xpNoise;    // unit 0: r billows (Perlin-Worley), g detail (Worley), tileable
uniform sampler2D skyLut;     // unit 8: the sky table (the world's binding)
uniform sampler2D skyDensity; // unit 9: the weather sky's cloud density (the world's binding)
uniform vec4 xpRight;         // xyz camera right * tan(fov x / 2), w the frame's jitter seed
uniform vec4 xpUp;            // xyz camera up * tan(fov y / 2), w the billows' boil (m)
uniform vec4 xpFwd;           // xyz camera forward, w primary steps
uniform vec4 xpLayer;         // x cloud base (m ASL), y slab thickness (m), z extinction (1/m), w deck weight
uniform vec4 xpShape;         // x cover from, y cover full, z erosion, w detail erosion
uniform vec4 xpLight;         // x sun gain, y multiple-scattering share, z sky gain, w haze distance (m)
uniform vec4 xpShadow;        // the shadow pass only: x the map's size (m, it wraps), y its steps, z its texel (m)
out vec4 fragColor;           // rgb the clouds' light toward the eye (linear), a the sky's transmittance
)";

const char kXpCloudsHelpers[] = R"(
const float kXpDeck = 1800.0;         // SkyDensity.hpp kSkyShadowDeckAltitude
const float kXpAnchorRadius = 1500.0; // kSkyShadowAnchorRadius
const float kXpMaxDist = 60000.0;
const float kXpPi = 3.14159265;
const float kXpBillow = 1800.0;       // the billow noise's tile (m)
const float kXpDetail = 420.0;        // the detail noise's tile (m)
const float kXpPixel = 0.0016;        // a march pixel's angle (rad, about 70 degrees over 960)

// SkyDomeUv (the dome's own mapping, turned with the sky: litSky.z).
vec2 xpDomeUv(vec3 w) {
    float az = atan(w.x, w.z) * 57.29578;
    float e = asin(clamp(w.y * inversesqrt(max(dot(w, w), 1e-12)), -1.0, 1.0)) * 57.29578;
    float v;
    if (e < 0.87) v = mix(1.0, 0.9869, clamp((e + 15.0) / 15.87, 0.0, 1.0));
    else if (e < 29.5) v = mix(0.9869, 0.5434, (e - 0.87) / 28.63);
    else if (e < 59.5) v = mix(0.5434, 0.1736, (e - 29.5) / 30.0);
    else v = mix(0.1736, 0.036, clamp((e - 59.5) / 29.5, 0.0, 1.0));
    return vec2(az / 180.0 + litSky.z, v);
}

// The column over the camera-relative ground point xz: x its cover (0..1,
// where the painted broken cloud is thick enough; the dome keeps the thinner
// painted cloud, and an overcast deck stays the painted deck), y the top's
// height fraction (thick and dark cloud tall), z 0, w its density. Looked up
// as domeCloudCover looks the shadows up: the deck point seen from the anchor
// (and the previous anchor while the shadows cross-fade).
vec4 xpColumn(vec2 xz) {
    float hy = kXpDeck - litCamPos.y;
    vec4 m = textureLod(skyDensity, xpDomeUv(vec3(xz.x - litSkyShadow.x, hy, xz.y - litSkyShadow.y)), 0.0);
    if (litSkyShadow.w > 0.0) {
        vec2 prev = litSkyShadow.xy + kXpAnchorRadius * vec2(cos(litSkyShadow.z), sin(litSkyShadow.z));
        vec4 mp = textureLod(skyDensity, xpDomeUv(vec3(xz.x - prev.x, hy, xz.y - prev.y)), 0.0);
        m = mix(m, mp, litSkyShadow.w);
    }
    float cb = m.r * (0.75 + 0.25 * m.b) * (1.0 - m.g * xpLayer.w);
    float cover = smoothstep(xpShape.x, xpShape.y, cb);
    float top = mix(0.35, 1.0, smoothstep(0.4, 0.85, cb)) * mix(0.75, 1.0, m.b);
    float dmax = mix(0.5, 1.0, smoothstep(0.45, 0.85, cb));
    return vec4(cover, top, 0.0, dmax);
}

// The noise's frame turns with the field (the anchor at its centre), so the
// billows ride on the clouds; xpUp.w boils them slowly.
vec3 xpNoisePos(vec3 p) {
    vec2 r = p.xz - litSkyShadow.xy;
    float a = litSky.z * kXpPi; // the field's turn (u wraps every 180 degrees)
    float ca = cos(a), sa = sin(a);
    vec2 q = vec2(r.x * ca + r.y * sa, -r.x * sa + r.y * ca);
    return vec3(q.x, p.y + litCamPos.y + xpUp.w, q.y);
}

float xpRemap(float x, float lo, float hi) {
    return clamp((x - lo) / max(hi - lo, 1e-4), 0.0, 1.0);
}

// Density (0..1) at the camera-relative point p of height fraction hf in the
// slab, the noise read at `fp` metres of footprint; `detail` the fine noise.
// The shape is the billow noise under the height profile, cut by the column's
// cover (Schneider 2015: where the cover is thin only the noise's peaks stay,
// small puffs; where it is whole the cloud is whole, its base and top still
// rolling with the noise), then eroded by the detail noise at its edge.
float xpDensity(vec3 p, float hf, vec4 col, float fp, bool detail) {
    if (col.x <= 0.0 || hf <= 0.0 || hf >= col.y) return 0.0;
    vec3 np = xpNoisePos(p);
    float n = textureLod(xpNoise, np / kXpBillow, log2(max(fp / (kXpBillow / 64.0), 1.0))).r;
    float prof = smoothstep(0.0, 0.07, hf) * (1.0 - smoothstep(col.y * 0.4, col.y, hf));
    float shape = xpRemap(n * prof, 1.0 - col.x, 1.0) * col.x;
    if (shape <= 0.0) return 0.0;
    if (detail && shape < 0.6) {
        float m = textureLod(xpNoise, np / kXpDetail, log2(max(fp / (kXpDetail / 64.0), 1.0))).g;
        shape = xpRemap(shape, (1.0 - m) * xpShape.w * (1.0 - shape), 1.0);
    }
    return shape * col.w;
}

// Henyey-Greenstein times 4 pi (its mean over the sphere is 1).
float xpHG(float c, float g) {
    float g2 = g * g;
    return (1.0 - g2) / pow(max(1.0 + g2 - 2.0 * g * c, 1e-4), 1.5);
}

// Interleaved gradient noise (Jimenez 2014), the frame's seed added.
float xpIgn(vec2 p) {
    p += xpRight.w * vec2(47.0, 17.0);
    return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

)";

const char kXpCloudsMarchMain[] = R"(
void main() {
    vec2 ndc = vUV * 2.0 - 1.0;
    vec3 d = normalize(xpFwd.xyz + xpRight.xyz * ndc.x + xpUp.xyz * ndc.y);
    float camY = litCamPos.y;
    float hb = xpLayer.x;
    float th = xpLayer.y;
    vec3 sunDir = litSkySun.xyz;
    vec3 sunL = litAtmoCloudSun.rgb * xpLight.x;
    vec3 above = textureLod(skyLut, vec2(2.5 / 3.0, 0.5 + 0.5 * sqrt(0.61 / (0.5 * kXpPi))), 0.0).rgb;
    // The sky's light inside a cloud is half diffused already: half its hue.
    vec3 tint = mix(above / max(dot(above, vec3(0.2126, 0.7152, 0.0722)), 1e-8), vec3(1.0), 0.5);
    vec3 skyL = litAtmoCloudSun.w * xpLight.z * tint;
    float cosT = dot(d, sunDir);
    float phase = mix(xpHG(cosT, 0.6), xpHG(cosT, -0.2), 0.3);
    float sunUp = clamp(sunDir.y * 6.0 + 0.6, 0.0, 1.0);
    vec3 light = vec3(0.0);
    float trans = 1.0;
    float tSum = 0.0;
    float wSum = 0.0;

    if (d.y > 0.004 && camY < hb - 50.0) {
        float t0 = (hb - camY) / d.y;
        float t1 = min((hb + th - camY) / d.y, kXpMaxDist);
        if (t0 < kXpMaxDist) {
            int n = int(xpFwd.w);
            float ds = (t1 - t0) / float(n);
            float t = t0 + ds * xpIgn(gl_FragCoord.xy);
            for (int i = 0; i < n; ++i, t += ds) {
                vec3 p = d * t;
                float hf = (camY + p.y - hb) / th;
                vec4 col = xpColumn(p.xz);
                if (col.x <= 0.0) continue;
                float fp = t * kXpPixel;
                float dens = xpDensity(p, hf, col, fp, true);
                if (dens > 0.002) {
                    float sigma = dens * xpLayer.z;
                    // Toward the sun: 5 steps of 40 m doubling (to 1.2 km).
                    float tau = 0.0;
                    float ls = 40.0;
                    vec3 lp = p;
                    for (int j = 0; j < 5; ++j) {
                        lp += sunDir * ls;
                        float lhf = (camY + lp.y - hb) / th;
                        if (lhf >= 1.0) break;
                        tau += xpDensity(lp, lhf, xpColumn(lp.xz), max(fp, ls), false) * xpLayer.z * ls;
                        ls *= 2.0;
                    }
                    // Single scattering, plus the light diffused through the
                    // cloud above (a slab's diffuse transmission, 1 / (1 +
                    // 0.11 tau) for g 0.85), so thick cloud goes grey, not black.
                    vec3 sun = sunL * (phase * exp(-tau) + xpLight.y / (1.0 + 0.11 * tau)) * sunUp;
                    // The sky above through the cloud above (its diffuse
                    // transmission), less toward the base; a little ground
                    // light from below.
                    float up = clamp(hf / max(col.y, 1e-3), 0.0, 1.0);
                    float tauUp = col.x * col.w * xpLayer.z * max(col.y - hf, 0.0) * th;
                    vec3 amb = skyL * mix(0.5, 1.0, up) / (1.0 + 0.11 * tauUp) +
                               sunL * (0.05 * max(sunDir.y, 0.0) * (1.0 - up));
                    float st = exp(-sigma * ds);
                    float w = trans * (1.0 - st);
                    light += w * (sun + amb);
                    tSum += w * t;
                    wSum += w;
                    trans *= st;
                    if (trans < 0.01) break;
                }
            }
        }
    }
    // Aerial perspective: far clouds fade into the clear sky behind them.
    float tc = wSum > 1e-5 ? tSum / wSum : 0.0;
    float f = exp(-tc / xpLight.w);
    fragColor = vec4(light * f, 1.0 - (1.0 - trans) * f);
}
)";

// ---- The cloud shadows (a world-space map around the camera) -----------------
// Each texel is a point on the cloud base (xpLayer.x), in world xz: the map
// wraps every xpShadow.x metres and a texel is the copy of its point nearest
// the camera's own base point along the sun, so the grid is fixed to the world
// and does not swim as the camera moves. r: the share of the sun that comes
// through the march's own clouds along the sun from that point (the density
// the march draws, the detail erosion included) and through the overcast deck,
// which the dome still paints and so keeps today's shadow; g: today's shadow
// cover there (domeCloudCover, from a point at the camera's height), for the
// shadow dump.
const char kXpCloudsShadowMain[] = R"(
// x today's cover, y its overcast deck's part alone (SkyCloudDensity with the
// broken cloud left out).
vec2 xpOldCover(vec3 b, vec3 l, float up) {
    vec3 q = b + l * ((kXpDeck - xpLayer.x) / up);
    vec2 a = litSkyShadow.xy;
    vec4 m = textureLod(skyDensity, xpDomeUv(vec3(q.x - a.x, q.y, q.z - a.y)), 0.0);
    if (litSkyShadow.w > 0.0) {
        vec2 prev = a + kXpAnchorRadius * vec2(cos(litSkyShadow.z), sin(litSkyShadow.z));
        vec4 mp = textureLod(skyDensity, xpDomeUv(vec3(q.x - prev.x, q.y, q.z - prev.y)), 0.0);
        m = mix(m, mp, litSkyShadow.w);
    }
    float deck = m.g * (1.0 - smoothstep(0.35, 0.75, litSky.w));
    float k = mix(0.75 + 0.25 * m.b, 0.5 + 0.5 * m.b, deck);
    return vec2(smoothstep(0.43, 0.67, max(m.r, deck) * k), smoothstep(0.43, 0.67, deck * k));
}

void main() {
    vec3 l = normalize(litSunDir.xyz);
    float up = max(l.y, 0.06);
    float hb = xpLayer.x;
    float th = xpLayer.y;
    float camY = litCamPos.y;
    float size = xpShadow.x;
    vec2 centre = litCamPos.xz + l.xz * (max(hb - camY, 0.0) / up);
    vec2 at = size * (floor(centre / size - vUV + 0.5) + vUV);
    vec3 b = vec3(at.x - litCamPos.x, hb - camY, at.y - litCamPos.z);
    int n = int(xpShadow.y);
    float ds = min(th / up, 8000.0) / float(n);
    float fp = xpShadow.z;
    float tau = 0.0;
    for (int i = 0; i < n; ++i) {
        vec3 p = b + l * ((float(i) + 0.5) * ds);
        float hf = (camY + p.y - hb) / th;
        if (hf >= 1.0) break;
        tau += xpDensity(p, hf, xpColumn(p.xz), fp, true) * xpLayer.z * ds;
    }
    vec2 today = xpOldCover(b, l, up);
    fragColor = vec4(exp(-tau) * (1.0 - today.y), today.x, 0.0, 1.0);
}
)";

// ---- The resolve (half resolution) -------------------------------------------
const char kXpCloudsResolveGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D xpCur;  // unit 0: this frame's march
uniform sampler2D xpHist; // unit 11: the last frame's resolve
uniform vec4 xpRight;     // the camera basis, as the march's
uniform vec4 xpUp;
uniform vec4 xpFwd;
uniform vec4 xpPrevRight; // the last frame's basis; xpPrevFwd.w 1 = a history
uniform vec4 xpPrevUp;
uniform vec4 xpPrevFwd;
uniform vec4 xpResolve;   // x the history's share
out vec4 fragColor;
void main() {
    ivec2 ip = ivec2(gl_FragCoord.xy);
    ivec2 lim = textureSize(xpCur, 0) - 1;
    // The march's 3x3 neighbourhood: its bounds clamp the history, and its
    // tent-filtered value (1 2 1) is this frame's sample, so a pixel with no
    // history (the edge a fast pan brings in) shows no stripes of the march's
    // per-pixel jitter.
    vec4 c = vec4(0.0);
    vec4 mn = vec4(1e9);
    vec4 mx = vec4(-1e9);
    for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
            vec4 s = texelFetch(xpCur, clamp(ip + ivec2(i, j), ivec2(0), lim), 0);
            c += s * float((2 - abs(i)) * (2 - abs(j)));
            mn = min(mn, s);
            mx = max(mx, s);
        }
    }
    c /= 16.0;
    vec4 res = c;
    if (xpPrevFwd.w > 0.5) {
        vec2 ndc = vUV * 2.0 - 1.0;
        vec3 d = normalize(xpFwd.xyz + xpRight.xyz * ndc.x + xpUp.xyz * ndc.y);
        float fz = dot(d, xpPrevFwd.xyz);
        if (fz > 1e-3) {
            vec3 R = xpPrevRight.xyz;
            vec3 U = xpPrevUp.xyz;
            vec2 n = vec2(dot(d, R) / dot(R, R), dot(d, U) / dot(U, U)) / fz;
            vec2 uv = n * 0.5 + 0.5;
            if (all(greaterThanEqual(uv, vec2(0.0))) && all(lessThanEqual(uv, vec2(1.0)))) {
                vec4 h = clamp(texture(xpHist, uv), mn, mx);
                res = mix(c, h, xpResolve.x);
            }
        }
    }
    fragColor = res;
}
)";

} // namespace

// The two programs' fragment sources as the engine assembles them (the unit
// tests compile them): 0 the march, 1 the resolve, 2 the cloud shadows; null
// past the end.
const char* XpCloudsShaderSource(int which)
{
    static const std::string march =
        std::string(kXpCloudsMarchHead) + kLitConstantsBlockGLSL + kXpCloudsHelpers + kXpCloudsMarchMain;
    static const std::string resolve = kXpCloudsResolveGLSL;
    static const std::string shadow =
        std::string(kXpCloudsMarchHead) + kLitConstantsBlockGLSL + kXpCloudsHelpers + kXpCloudsShadowMain;
    switch (which)
    {
        case 0:
            return march.c_str();
        case 1:
            return resolve.c_str();
        case 2:
            return shadow.c_str();
        default:
            return nullptr;
    }
}

} // namespace Poseidon::gl33lighting
