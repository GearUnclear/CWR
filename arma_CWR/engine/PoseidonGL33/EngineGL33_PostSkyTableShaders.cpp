// Remastered sky: the GLSL of the sky table's two fullscreen passes
// (EngineGL33_SkyTable.cpp runs them before the world, when the sun has moved;
// they are not post stages). No GL code, so the unit tests compile these very
// sources; kPostShaders in EngineGL33_PostShaders.cpp assembles them with the
// fullscreen vertex shader the post passes share.
//
// They are the GPU side of Graphics/Remaster/Atmosphere.cpp: SkyViewTexel and
// SkyViewRowMean compute the same texels on the CPU, step for step (the
// numbers come in as uniforms from Atmosphere.hpp, the transmittance and
// multiple-scattering tables as float textures read with texelFetch and
// interpolated by hand on their grid points), so triSkyTableCheck can compare
// the two.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

// Inputs of both passes.
const char kPostSkyTableHeaderGLSL[] = R"(#version 330 core
in vec2 vUV;
out vec4 fragColor;
uniform sampler2D transTex; // transmittance table (texelFetch)
uniform sampler2D msTex;    // multiple-scattering table (texelFetch)
uniform sampler2D viewTex;  // the mean pass: regions 0 and 1 of the sky-view table (texelFetch)
uniform vec4 atmoRadii;     // x ground radius, y top radius, z Rayleigh scale height, w Mie scale height (m)
uniform vec4 atmoRayleigh;  // rgb Rayleigh scattering (1/m), w Mie asymmetry g
uniform vec4 atmoMie;       // x Mie scattering, y Mie extinction (1/m, haze applied), z ozone centre, w ozone half width (m)
uniform vec4 atmoOzone;     // rgb ozone absorption (1/m)
uniform vec4 atmoView;      // x sun elevation (rad), y eye radius (m), z march steps, w azimuths per row mean
uniform ivec4 atmoSizes;    // xy transmittance size, z multiple-scattering size, w sky-view size per region

const float kPi = 3.14159265358979323846;

// The sky-view parametrisation at texel centres (SkyViewRowElevation,
// SkyViewColumnAzimuth).
float rowElevation(int row) {
    float v = (float(row) + 0.5) / float(atmoSizes.w);
    float x = 2.0 * v - 1.0;
    return (x < 0.0 ? -1.0 : 1.0) * x * x * (kPi * 0.5);
}

vec3 skyDirection(float l, float phi) {
    return vec3(cos(l) * cos(phi), sin(l), cos(l) * sin(phi));
}
)";

// The model: ray-sphere distances, the medium and the two table lookups.
const char kPostSkyTableModelGLSL[] = R"(
// (r mu)^2 plus a difference of squares of nearby radii: precise in float near
// the ground, as Atmosphere.cpp writes them.
float distanceToTop(float r, float mu) {
    float rmu = r * mu;
    float disc = rmu * rmu + (atmoRadii.y - r) * (atmoRadii.y + r);
    return max(0.0, -rmu + sqrt(max(disc, 0.0)));
}

float distanceToBottom(float r, float mu) {
    float rmu = r * mu;
    float disc = rmu * rmu - (r - atmoRadii.x) * (r + atmoRadii.x);
    return max(0.0, -rmu - sqrt(max(disc, 0.0)));
}

bool hitsGround(float r, float mu) {
    float rmu = r * mu;
    return mu < 0.0 && rmu * rmu - (r - atmoRadii.x) * (r + atmoRadii.x) >= 0.0;
}

void medium(float h, out vec3 scatR, out float scatM, out vec3 ext) {
    float hh = max(h, 0.0);
    float rr = exp(-hh / atmoRadii.z);
    float rm = exp(-hh / atmoRadii.w);
    float ro = max(0.0, 1.0 - abs(h - atmoMie.z) / atmoMie.w);
    scatR = atmoRayleigh.rgb * rr;
    scatM = atmoMie.x * rm;
    ext = scatR + atmoMie.y * rm + atmoOzone.rgb * ro;
}

// Bilinear on the grid points 0..n-1 of a table.
vec3 gridLookup(sampler2D tex, int w, int h, float fx, float fy) {
    int x0 = min(int(fx), w - 2);
    int y0 = min(int(fy), h - 2);
    float tx = fx - float(x0);
    float ty = fy - float(y0);
    vec3 a = texelFetch(tex, ivec2(x0, y0), 0).rgb;
    vec3 b = texelFetch(tex, ivec2(x0 + 1, y0), 0).rgb;
    vec3 e = texelFetch(tex, ivec2(x0, y0 + 1), 0).rgb;
    vec3 f = texelFetch(tex, ivec2(x0 + 1, y0 + 1), 0).rgb;
    return (a + (b - a) * tx) * (1.0 - ty) + (e + (f - e) * tx) * ty;
}

vec3 transmittance(float r, float mu) {
    if (hitsGround(r, mu)) return vec3(0.0);
    float rg = atmoRadii.x;
    float rt = atmoRadii.y;
    float hTop = sqrt(rt * rt - rg * rg);
    float rho = sqrt(max((r - rg) * (r + rg), 0.0));
    float d = distanceToTop(r, mu);
    float dMin = rt - r;
    float dMax = rho + hTop;
    float xMu = dMax > dMin ? (d - dMin) / (dMax - dMin) : 0.0;
    float xR = rho / hTop;
    return gridLookup(transTex, atmoSizes.x, atmoSizes.y, clamp(xMu, 0.0, 1.0) * float(atmoSizes.x - 1),
                      clamp(xR, 0.0, 1.0) * float(atmoSizes.y - 1));
}

vec3 multiScatter(float height, float muS) {
    int n = atmoSizes.z;
    return gridLookup(msTex, n, n, clamp(muS * 0.5 + 0.5, 0.0, 1.0) * float(n - 1),
                      clamp(height / (atmoRadii.y - atmoRadii.x), 0.0, 1.0) * float(n - 1));
}
)";

// Pass 1, into a (2 x size) x size target: region 0 (Rayleigh with its phase
// plus multiple scattering) on the left, region 1 (Mie without its phase) on
// the right. The march of SkyViewTexel: steps that grow quadratically with
// distance, the source sampled 0.3 into each step.
const char kPostSkyViewMainGLSL[] = R"(
void main() {
    ivec2 ij = ivec2(gl_FragCoord.xy);
    int n = atmoSizes.w;
    int region = ij.x / n;
    int col = ij.x - region * n;
    float u = (float(col) + 0.5) / float(n);
    vec3 dir = skyDirection(rowElevation(ij.y), kPi * u * u);
    float se = atmoView.x;
    vec3 sun = vec3(cos(se), sin(se), 0.0);
    float r0 = atmoView.y;
    vec3 origin = vec3(0.0, r0, 0.0);
    float mu = dir.y;
    float tMax = hitsGround(r0, mu) ? distanceToBottom(r0, mu) : distanceToTop(r0, mu);
    float cosT = dot(dir, sun);
    float phaseR = 3.0 / (16.0 * kPi) * (1.0 + cosT * cosT);
    vec3 tr = vec3(1.0);
    vec3 lr = vec3(0.0);
    vec3 lm = vec3(0.0);
    float tPrev = 0.0;
    int steps = int(atmoView.z);
    for (int k = 0; k < steps; ++k) {
        float f1 = float(k + 1) / float(steps);
        float tNext = tMax * f1 * f1;
        float dt = tNext - tPrev;
        float t = tPrev + 0.3 * dt;
        tPrev = tNext;
        vec3 p = origin + dir * t;
        float rp = sqrt(dot(p, p));
        vec3 scatR;
        float scatM;
        vec3 ext;
        medium(rp - atmoRadii.x, scatR, scatM, ext);
        float muSp = dot(p * (1.0 / rp), sun);
        vec3 ts = transmittance(rp, muSp);
        vec3 ms = multiScatter(rp - atmoRadii.x, muSp);
        ext = max(ext, vec3(1e-12));
        vec3 stepT = exp(-ext * dt);
        vec3 srcR = scatR * phaseR * ts + (scatR + scatM) * ms;
        vec3 srcM = scatM * ts;
        lr += tr * (srcR - srcR * stepT) / ext;
        lm += tr * (srcM - srcM * stepT) / ext;
        tr *= stepT;
    }
    fragColor = vec4(region == 0 ? lr : lm, 1.0);
}
)";

// Pass 2, into the (3 x size) x size table the shaders read: regions 0 and 1
// copied from pass 1, region 2 the mean of each row over its azimuths
// (SkyViewRowMean), with the lookup the shaders make (texel centres, the Mie
// phase applied per direction), and in alpha the row's mean of the original
// dome's sun-glow weight max(cos, 0)^6 (SkyViewRowGlow).
const char kPostSkyViewMeanMainGLSL[] = R"(
vec3 viewAt(int x, int y) {
    return texelFetch(viewTex, ivec2(x, y), 0).rgb;
}

// AtmosphereMiePhase: the broad lobe and the aureole's peak.
const float kAureoleG = 0.97;    // atmo::kMieAureoleG
const float kAureoleShare = 0.3; // atmo::kMieAureoleShare
float miePhase(float c) {
    float g = atmoRayleigh.w;
    float k = 3.0 / (8.0 * kPi) * (1.0 - g * g) / (2.0 + g * g);
    float broad = k * (1.0 + c * c) / pow(max(1.0 + g * g - 2.0 * g * c, 1e-6), 1.5);
    float peak = (1.0 - kAureoleG * kAureoleG) /
                 (4.0 * kPi * pow(max(1.0 + kAureoleG * kAureoleG - 2.0 * kAureoleG * c, 1e-7), 1.5));
    return mix(broad, peak, kAureoleShare);
}

vec3 sampleView(float l, float phi, float cosToSun) {
    int n = atmoSizes.w;
    float size = float(n);
    float u = sqrt(clamp(phi, 0.0, kPi) / kPi);
    float lc = clamp(l, -kPi * 0.5, kPi * 0.5);
    float v = 0.5 + 0.5 * (lc < 0.0 ? -1.0 : 1.0) * sqrt(abs(lc) / (kPi * 0.5));
    float fx = clamp(u * size - 0.5, 0.0, size - 1.0);
    float fy = clamp(v * size - 0.5, 0.0, size - 1.0);
    int x0 = min(int(fx), n - 2);
    int y0 = min(int(fy), n - 2);
    float tx = fx - float(x0);
    float ty = fy - float(y0);
    vec3 r = (viewAt(x0, y0) + (viewAt(x0 + 1, y0) - viewAt(x0, y0)) * tx) * (1.0 - ty) +
             (viewAt(x0, y0 + 1) + (viewAt(x0 + 1, y0 + 1) - viewAt(x0, y0 + 1)) * tx) * ty;
    vec3 m = (viewAt(n + x0, y0) + (viewAt(n + x0 + 1, y0) - viewAt(n + x0, y0)) * tx) * (1.0 - ty) +
             (viewAt(n + x0, y0 + 1) + (viewAt(n + x0 + 1, y0 + 1) - viewAt(n + x0, y0 + 1)) * tx) * ty;
    return r + m * miePhase(cosToSun);
}

void main() {
    ivec2 ij = ivec2(gl_FragCoord.xy);
    int n = atmoSizes.w;
    if (ij.x < 2 * n) {
        fragColor = vec4(viewAt(ij.x, ij.y), 1.0);
        return;
    }
    float l = rowElevation(ij.y);
    float se = atmoView.x;
    vec3 sun = vec3(cos(se), sin(se), 0.0);
    int taps = int(atmoView.w);
    vec3 sum = vec3(0.0);
    float glow = 0.0;
    for (int a = 0; a < taps; ++a) {
        float phi = (float(a) + 0.5) / float(taps) * kPi;
        float c = dot(skyDirection(l, phi), sun);
        sum += sampleView(l, phi, c);
        // The original dome's sun-glow weight (SkyViewRowGlow).
        float cp = max(c, 0.0);
        float c2 = cp * cp;
        glow += c2 * c2 * c2;
    }
    fragColor = vec4(sum * (1.0 / float(taps)), glow * (1.0 / float(taps)));
}
)";

} // namespace Poseidon::gl33lighting
