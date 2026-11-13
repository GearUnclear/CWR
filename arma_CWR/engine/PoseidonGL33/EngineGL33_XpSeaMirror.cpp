// XP sea mirror (perf squad lane EX5-SEA, plan item R-2; the switch and what
// it does: Graphics/Remaster/SeaMirror.hpp). One wave field for the sea's
// shading and its mirror:
//   - the water program writes the slope that built its wave normal into a
//     sea-only target: a multisampled RG16F texture on attachment 2 of the
//     frame target, in the draw buffers only between BeginSea and EndSea, so
//     no other draw writes it and it needs no clear (the trace reads it only
//     where this frame's tags say the sea's water program drew last);
//   - the reflection trace reads it over each half-resolution texel in place
//     of its own ripples and swaps the sea's own sky weight on that normal.
// Both shaders are built from their stock text with the additions spliced in
// at fixed anchors, so with the switch off the stock sources are used
// unchanged.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/SeaFlow.hpp>
#include <Poseidon/Graphics/Remaster/SeaMirror.hpp>

#include <cstdio>
#include <string>

namespace rm = Poseidon::remaster;

namespace
{

// Inserts `text` after (or before) the first `anchor` in `s`; false when the
// anchor is missing.
bool SpliceAfter(std::string& s, const char* anchor, const char* text)
{
    const size_t at = s.find(anchor);
    if (at == std::string::npos)
        return false;
    s.insert(at + std::char_traits<char>::length(anchor), text);
    return true;
}

bool SpliceBefore(std::string& s, const char* anchor, const char* text)
{
    const size_t at = s.find(anchor);
    if (at == std::string::npos)
        return false;
    s.insert(at, text);
    return true;
}

bool Replace(std::string& s, const char* anchor, const char* text)
{
    const size_t at = s.find(anchor);
    if (at == std::string::npos)
        return false;
    s.replace(at, std::char_traits<char>::length(anchor), text);
    return true;
}

// XP sea flow (Graphics/Remaster/SeaFlow.hpp): the travelling layers. The
// rates are the process's (the switch's speed scale), written into the text.
std::string Num(float v)
{
    char buf[48];
    std::snprintf(buf, sizeof(buf), "%.9g", static_cast<double>(v));
    std::string t = buf;
    if (t.find_first_of(".e") == std::string::npos)
        t += ".0";
    return t;
}

std::string Vec2(const float v[2])
{
    return "vec2(" + Num(v[0]) + ", " + Num(v[1]) + ")";
}

std::string Mat2(const float m[4])
{
    return "mat2(" + Num(m[0]) + ", " + Num(m[1]) + ", " + Num(m[2]) + ", " + Num(m[3]) + ")";
}

bool SpliceFlow(std::string& s, float speed, float rho, bool morph, rm::SeaFlowField field)
{
    const rm::SeaFlowRates r = rm::ComputeSeaFlowRates(speed, field);
    const rm::SeaFlowShape& shape = rm::SeaFlowFieldShape(field);
    float chopMap[4];
    rm::SeaFlowChopMap(shape, chopMap);
    float swell2Map[4];
    rm::SeaFlowSwell2Map(shape, swell2Map);
    // The swell's partner (the sea field): the swell's own size, travelling
    // against it, so the big waves stand as the fine ones do.
    const bool swell2 = shape.swell2K > 0.0f;
    const std::string swell2Decls =
        swell2 ? "const vec2 kFlowRateS2 = " + Vec2(r.swell2) + ";\nconst mat2 kFlowSwell2Map = " + Mat2(swell2Map) +
                     ";\nconst float kFlowSwell2Freq = " + Num(rm::kSeaFlowSwellFreq * shape.swell2Freq) +
                     ";\nconst float kFlowSwell2K = " + Num(shape.swell2K) + "; // the swell's partner\n"
               : std::string();
    const std::string swell2Code =
        swell2 ? "        // XP sea flow: the swell's partner, travelling against it.\n"
                 "        vec2 uvS2 = kFlowSwell2Map * vUV0 + vec2(0.53, 0.11) + fract(kFlowRateS2 * flowT);\n"
                 "        vec2 gUs2 = kFlowSwell2Map[0][0] * gU + kFlowSwell2Map[1][0] * gV;\n"
                 "        vec2 gVs2 = kFlowSwell2Map[0][1] * gU + kFlowSwell2Map[1][1] * gV;\n"
                 "        float eS2 = max(1.0 / 512.0, e0 * kFlowSwell2Freq);\n"
                 "        grad += kFlowSwell2K * seaGradS(uvS2, seaLumS(uvS2), gUs2, gVs2, eS2);\n"
               : "";
    // The swell's frame: without the dissolve, seaNext alone (a pattern of its
    // own; one fetch, as litWater0.y is 0 then: the round 3 first cut fetched
    // tex0 too and mixed it in at weight 0, which cost as much as the dissolve);
    // with it, the same one-way mix as the other layers (a mix the other way
    // round would jump at each step, where the frames change places).
    const char* swellLum =
        morph ? "    return dot(mix(texture(tex0, uv), texture(seaNext, uv), litWater0.y).rgb, kLuma);\n"
              : "    return dot(texture(seaNext, uv).rgb, kLuma);\n";
    const std::string decls = "// XP sea flow (Graphics/Remaster/SeaFlow.hpp): the sea's animation does not play\n"
                              "// back and forth. Every wave layer travels over the world at its own speed and\n"
                              "// heading (offsets per second of the clock litWater2.y, whole numbers of 1 / 4096 so\n"
                              "// the clock's wrap does not show). litWater0.y is the one-way dissolve from tex0 to\n"
                              "// seaNext (0 without it, and then the swell takes seaNext alone).\n"
                              "const vec2 kFlowRateB = " +
                              Vec2(r.base) +
                              ";\n"
                              "const vec2 kFlowRateS = " +
                              Vec2(r.swell) +
                              ";\n"
                              "const vec2 kFlowRateC = " +
                              Vec2(r.chop) +
                              ";\n"
                              "const vec2 kFlowRateR = " +
                              Vec2(r.ripple) +
                              ";\n"
                              "const mat2 kFlowChopMap = " +
                              Mat2(chopMap) +
                              ";\n"
                              "const float kFlowChopFreq = " +
                              Num(shape.chopFreq) +
                              ";\n"
                              "const float kFlowBaseK = " +
                              Num(shape.baseK) +
                              "; // the texture's layer (stock 0.8)\n"
                              "const float kFlowChopK = " +
                              Num(shape.chopK) +
                              "; // the third layer\n"
                              "const float kFlowSwellK = " +
                              Num(shape.swellK) + "; // the swell (stock 1.2)\n" + swell2Decls +
                              "const float kFlowRho = " + Num(rho) +
                              "; // gradient correlation of the two frames\n"
                              "float seaLumS(vec2 uv) {\n" +
                              std::string(swellLum) +
                              "}\n"
                              "vec2 seaGradS(vec2 uv, float h0, vec2 gU, vec2 gV, float e) {\n"
                              "    float hu = seaLumS(uv + vec2(e, 0.0));\n"
                              "    float hv = seaLumS(uv + vec2(0.0, e));\n"
                              "    return ((hu - h0) * gU + (hv - h0) * gV) / e;\n"
                              "}\n";
    bool ok = true;
    ok = ok && SpliceAfter(s, "uniform sampler2D seaNext; // unit 11: the animation frame tex0 cross-fades into\n",
                           decls.c_str());
    ok = ok && Replace(s, "void main() {\n    vec4 t1 = texture(tex1, vUV1);\n",
                       "void main() {\n"
                       "    // XP sea flow: the layers' coordinates at this instant.\n"
                       "    float flowT = litWater2.y;\n"
                       "    vec2 uvB = vUV0 + fract(kFlowRateB * flowT);\n"
                       "    vec2 uvR = vUV1 + fract(kFlowRateR * flowT);\n"
                       "    vec4 t1 = texture(tex1, uvR);\n");
    ok = ok && Replace(s, "        vec4 t0 = seaTex(vUV0);\n", "        vec4 t0 = seaTex(uvB);\n");
    ok = ok && Replace(s, "        vec2 grad = kSeaWaveK * seaGrad(vUV0, dot(t0.rgb, kLuma), gU, gV, e0);\n",
                       "        vec2 grad = kFlowBaseK * seaGrad(uvB, dot(t0.rgb, kLuma), gU, gV, e0);\n");
    ok = ok && Replace(s, "        vec2 uvS = R * vUV0 + vec2(0.31, 0.57);\n",
                       "        vec2 uvS = R * vUV0 + vec2(0.31, 0.57) + fract(kFlowRateS * flowT);\n");
    const std::string swellAndChop =
        "        grad += kFlowSwellK * seaGradS(uvS, seaLumS(uvS), gUs, gVs, eS);\n" + swell2Code +
        "        // XP sea flow: the third layer (the sea field: the texture's twin,\n"
        "        // travelling against it).\n"
        "        vec2 uvC = kFlowChopMap * vUV0 + vec2(0.73, 0.19) + fract(kFlowRateC * flowT);\n"
        "        vec2 gUc = kFlowChopMap[0][0] * gU + kFlowChopMap[1][0] * gV;\n"
        "        vec2 gVc = kFlowChopMap[0][1] * gU + kFlowChopMap[1][1] * gV;\n"
        "        float eC = max(1.0 / 512.0, e0 * kFlowChopFreq);\n"
        "        grad += kFlowChopK * seaGrad(uvC, seaLum(uvC), gUc, gVc, eC);\n";
    ok = ok &&
         Replace(s, "        grad += kSeaSwellK * seaGrad(uvS, seaLum(uvS), gUs, gVs, eS);\n", swellAndChop.c_str());
    ok = ok && Replace(s, "        grad += kSeaRippleK * seaRippleGrad(vUV1, t1.a, gU * 32.0, gV * 32.0, e1);\n",
                       "        grad += kSeaRippleK * seaRippleGrad(uvR, t1.a, gU * 32.0, gV * 32.0, e1);\n");
    ok = ok && Replace(s, "2.0 * w * (1.0 - w) * kSeaFrameRho;\n", "2.0 * w * (1.0 - w) * kFlowRho;\n");
    return ok;
}

std::string BuildWaterMain(bool mirror, bool flow, float flowSpeed, float flowRho, bool flowMorph,
                           rm::SeaFlowField flowField)
{
    std::string s = Poseidon::gl33lighting::RemasterWaterMainStock();
    bool ok = true;
    if (flow)
        ok = ok && SpliceFlow(s, flowSpeed, flowRho, flowMorph, flowField);
    if (!mirror)
        return ok ? s : std::string();
    ok = ok && SpliceAfter(s, "uniform sampler2D seaNext; // unit 11: the animation frame tex0 cross-fades into\n",
                           "// XP sea mirror: the wave slope (the gradient that built the wave normal) for the\n"
                           "// reflections' trace; a sea-only target, in the draw buffers only while the sea draws.\n"
                           "layout(location = 2) out vec4 fragWave;\n");
    ok = ok && SpliceAfter(s, "void main() {\n", "    fragWave = vec4(0.0, 0.0, 0.0, 1.0);\n");
    ok = ok && SpliceBefore(s, "        // The surface target: the body's sky light, as the stock share has it.\n",
                            "        // XP sea mirror: the slope the trace moves the mirror image by.\n"
                            "        fragWave = vec4(grad, 0.0, 1.0);\n");
    return ok ? s : std::string();
}

const char kTraceUniforms[] = R"(// XP sea mirror (EngineGL33_XpSeaMirror.cpp): the sea program's wave slope per sample.
uniform sampler2DMS seaWave; // unit 6
uniform vec4 ssrSea;         // x 1 = the sea wrote its slope this frame, y the mirror's slope scale, z the sea's own sky weight's share
)";

const char kTraceHelpers[] = R"(// XP sea mirror: the sea program's own waves over the water pixels of this
// half-resolution texel. Each pixel's sky weight is the sea program's (wSky:
// the mean share plus the waves' Fresnel part at full weight, F0 0.02, and the
// wave masking when its part runs); meanW is their mean. The mirror image
// moves by their slopes' mean weighted by that sky weight (the facets that
// show the sky are the ones whose sky the reflection replaces), scaled by
// ssrSea.y; the weighted spread of the slopes, which a half-resolution trace
// cannot follow, blurs it instead (lostVar, per axis).
vec3 seaWaveNormal(ivec2 ij, ivec2 lim, vec3 vW, float dist, out float lostVar, out float meanW) {
    float Ff = 0.02 + 0.98 * pow(1.0 - clamp(vW.y, 0.0, 1.0), 5.0);
    float skyMean = litWater0.z * mix(kSeaFarSkyFloor, 1.0, 1.0 - smoothstep(kSeaFarSkyStart, kSeaFarSkyEnd, dist));
    bool masking = (int(litWater2.x + 0.5) & 8) != 0;
    vec2 sum = vec2(0.0);
    float sq = 0.0;
    float wsum = 0.0;
    float wtot = 0.0;
    float n = 0.0;
    for (int k = 0; k < 4; ++k) {
        ivec2 q = min(ij * 2 + ivec2(k & 1, k >> 1), lim);
        int tg = int(texelFetch(tagTex, q, 0).r * 255.0 + 0.5);
        if ((tg & 0x20) != 0 || ((tg >> 6) & 3) != 2) continue;
        vec2 g = clamp(texelFetch(seaWave, q, 0).rg, vec2(-4.0), vec2(4.0));
        vec3 nk = normalize(vec3(-g.x, 1.0, -g.y));
        float F = 0.02 + 0.98 * pow(clamp(1.0 - max(dot(nk, vW), 0.0), 0.0, 1.0), 5.0);
        float w = clamp(skyMean * Ff + (F - Ff), 0.0, 0.95);
        if (masking) w *= mix(0.6, 1.0, smoothstep(-0.08, 0.0, reflect(-vW, nk).y));
        wtot += w;
        n += 1.0;
        float a = w + 0.02;
        sum += a * g;
        sq += a * dot(g, g);
        wsum += a;
    }
    meanW = n > 0.0 ? wtot / n : 0.0;
    vec2 mg = wsum > 0.0 ? sum / wsum : vec2(0.0);
    float var = wsum > 0.0 ? max(sq / wsum - dot(mg, mg), 0.0) : 0.0;
    lostVar = 0.5 * var * ssrSea.y * ssrSea.y;
    vec2 gm = mg * ssrSea.y;
    return normalize(vec3(-gm.x, 1.0, -gm.y));
}

)";

std::string BuildTraceSource()
{
    std::string s = Poseidon::gl33lighting::PostShaderSource(Poseidon::gl33lighting::PostShader::SsrTrace);
    bool ok = true;
    ok = ok && SpliceAfter(s,
                           "uniform vec4 ssrWetMode; // x 1 = the wet weather shades the wet ground (no darkening "
                           "here; its puddles), y its puddle level\n",
                           kTraceUniforms);
    ok = ok && SpliceBefore(s, "void main() {\n    ivec2 ij = ivec2(gl_FragCoord.xy);\n    fragSpread = 0.0;\n",
                            kTraceHelpers);
    ok = ok && Replace(s, "        vec3 nWave = rippleNormal(posW.xz, ssrParams.z, foot, lostVar);\n",
                       "        // XP sea mirror: the sea program's own waves move the mirror image\n"
                       "        // (its ripples only when the sea wrote no slope this frame).\n"
                       "        float seaW = 0.0;\n"
                       "        vec3 nWave = ssrSea.x > 0.5 ? seaWaveNormal(ij, lim, vW, dist, lostVar, seaW)\n"
                       "                                    : rippleNormal(posW.xz, ssrParams.z, foot, lostVar);\n");
    ok = ok &&
         Replace(s,
                 "        }\n"
                 "        // A covered panel the ray met first (a boat's canvas or glass) lies\n",
                 "        } else if (ssrSea.x > 0.5) {\n"
                 "            // XP sea mirror: the flat mirror ray met nothing on screen, but a\n"
                 "            // wave facet tilted away from the viewer reflects lower: what stands\n"
                 "            // on screen in its direction (land, a hull; never the sky or the sea)\n"
                 "            // takes the sky's place, as for a hit, so the edge of a mirrored hill\n"
                 "            // breaks up with the waves instead of ending in a line.\n"
                 "            vec3 dV = w2v * rP;\n"
                 "            vec2 uvD = dV.z > 0.02 ? uvOfView(dV) : vec2(-1.0);\n"
                 "            float zD = inFrame(uvD) ? sceneZ(halfOfUv(uvD)) : kSsrSkyDepth;\n"
                 "            vec3 hv = w2v * (relW + rP * min(zD, 1.0e5));\n"
                 "            if (zD < kSsrSkyDepth && hv.z > 0.1) {\n"
                 "                vec2 uvP = uvOfView(hv);\n"
                 "                int tp = tagAt(uvP);\n"
                 "                float zP = sceneZ(halfOfUv(uvP));\n"
                 "                if (inFrame(uvP) && (tp & 0xC0) != 0x80 && zP < kSsrSkyDepth && zP > z * 0.98) {\n"
                 "                    float px = sigma * zD / (max(zP, 0.1) * pixAngle);\n"
                 "                    pxH = px * sqrt(max(vW.y, 0.02)) * 2.0;\n"
                 "                    conf = edgeFade(uvP);\n"
                 "                    R = streakSample(uvP, px, pxH);\n"
                 "                }\n"
                 "            }\n"
                 "        }\n"
                 "        // A covered panel the ray met first (a boat's canvas or glass) lies\n");
    ok = ok && Replace(s,
                       "        float seaLum = dot(frameLin(textureLod(mipTex, uvC, kSsrSeaBlurLod).rgb), kLuma);\n"
                       "        float share = clamp(skyMean * Ff * dot(S, kLuma) / max(seaLum, 1e-4), 0.0, 1.0);\n",
                       "        float seaLum = dot(frameLin(textureLod(mipTex, uvC, kSsrSeaBlurLod).rgb), kLuma);\n"
                       "        // XP sea mirror: ssrSea.z of the sea program's own sky weight over its\n"
                       "        // waves (seaW) over the light of the 2 x 2 pixels this texel covers, in\n"
                       "        // place of the mean share over the sea's colour around the pixel.\n"
                       "        float wS = skyMean * Ff;\n"
                       "        if (ssrSea.x > 0.5 && ssrSea.z > 0.0) {\n"
                       "            vec2 uvT = (vec2(ij) * 2.0 + 1.0) / screen.xy;\n"
                       "            float lumT = dot(frameLin(textureLod(mipTex, uvT, 1.0).rgb), kLuma);\n"
                       "            wS = mix(wS, seaW, ssrSea.z);\n"
                       "            seaLum = mix(seaLum, lumT, ssrSea.z);\n"
                       "        }\n"
                       "        float share = clamp(wS * dot(S, kLuma) / max(seaLum, 1e-4), 0.0, 1.0);\n");
    return ok ? s : std::string();
}

} // namespace

namespace Poseidon::gl33lighting
{

const char* SeaMirrorWaterMainXp()
{
    static const std::string s = BuildWaterMain(true, false, 1.0f, 0.3f, false, rm::SeaFlowField::Sea);
    return s.empty() ? nullptr : s.c_str();
}

std::string SeaFlowWaterMainXp(bool mirror, float speed, float rho, bool morph, rm::SeaFlowField field)
{
    return BuildWaterMain(mirror, true, speed, rho, morph, field);
}

const char* SeaWaterMainXp()
{
    // The process's switches: the mirror, the flow, both or neither (null).
    static const std::string s = []
    {
        const bool mirror = rm::SeaMirrorXpOn();
        const bool flow = rm::SeaFlowXpOn();
        if (!mirror && !flow)
            return std::string();
        std::string both = BuildWaterMain(mirror, flow, rm::SeaFlowXpSpeed(), rm::SeaFlowXpRho(),
                                          rm::SeaFlowXpMorph() > 0.0f, rm::SeaFlowXpField());
        // A missing anchor for the flow leaves the mirror alone, if it runs.
        if (both.empty() && mirror && flow)
            both = BuildWaterMain(true, false, 1.0f, 0.3f, false, rm::SeaFlowField::Sea);
        return both;
    }();
    return s.empty() ? nullptr : s.c_str();
}

const char* SeaMirrorTraceSourceXp()
{
    static const std::string s = BuildTraceSource();
    return s.empty() ? nullptr : s.c_str();
}

} // namespace Poseidon::gl33lighting

// ---- The sea's slope target ------------------------------------------------

void EngineGL33::DestroySeaWaveTarget()
{
    if (_seaWaveTex)
    {
        GL33Bind::OnTexDeleted(_seaWaveTex);
        glDeleteTextures(1, &_seaWaveTex);
    }
    _seaWaveTex = 0;
    _seaWaveW = _seaWaveH = _seaWaveSamples = 0;
    _seaWaveOpen = false;
}

void EngineGL33::SeaWaveBegin()
{
    if (_seaWaveOpen || _seaWaveFailed || !rm::SeaMirrorXpOn() || !Poseidon::gl33lighting::SeaMirrorWaterMainXp() ||
        !Poseidon::gl33lighting::SeaMirrorTraceSourceXp() || _msaaSamples < 2 || !_ssaaFbo ||
        !rm::FeatureOn(_lighting.features, rm::Feature::Ssr))
        return;
    GLint drawFbo = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
    if (static_cast<unsigned int>(drawFbo) != _ssaaFbo)
        return;
    if (_seaWaveTex && (_seaWaveW != _ssaaW || _seaWaveH != _ssaaH || _seaWaveSamples != _msaaSamples))
    {
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
        DestroySeaWaveTarget();
    }
    bool created = false;
    if (!_seaWaveTex)
    {
        created = true;
        // Made on first use and attached for the frame target's life
        // (DestroySSAATarget drops it with the frame target).
        glGenTextures(1, &_seaWaveTex);
        glActiveTexture(GL_TEXTURE0 + Poseidon::gl33lighting::kSeaMirrorWaveUnit);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _seaWaveTex);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _msaaSamples, GL_RG16F, _ssaaW, _ssaaH, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        GL33Bind::Invalidate();
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, _seaWaveTex, 0);
        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_WARN(Graphics, "GL33: XP sea mirror target {}x{} ({} samples) incomplete; the mirror keeps its ripples",
                     _ssaaW, _ssaaH, _msaaSamples);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
            DestroySeaWaveTarget();
            _seaWaveFailed = true;
            return;
        }
        _seaWaveW = _ssaaW;
        _seaWaveH = _ssaaH;
        _seaWaveSamples = _msaaSamples;
        LOG_INFO(Graphics, "GL33: XP sea mirror target {}x{} ({} samples), slope scale {}", _ssaaW, _ssaaH,
                 _msaaSamples, rm::SeaMirrorXpScale());
    }
    const GLenum surfBuf = _surfTargetOn ? static_cast<GLenum>(GL_COLOR_ATTACHMENT1) : static_cast<GLenum>(GL_NONE);
    const GLenum bufs[3] = {GL_COLOR_ATTACHMENT0, surfBuf, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, bufs);
    glColorMaski(2, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    if (created)
    {
        // A new target starts as flat water everywhere (the trace reads it
        // only under this frame's sea tags, but nothing should ever read
        // what the driver left in it).
        const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
        if (scissor)
            glDisable(GL_SCISSOR_TEST);
        const GLfloat flat[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glClearBufferfv(GL_COLOR, 2, flat);
        if (scissor)
            glEnable(GL_SCISSOR_TEST);
    }
    _seaWaveOpen = true;
    _seaWaveFrame = GetFrameCounter();
    _seaWaveFrameValid = true;
}

void EngineGL33::SeaWaveEnd()
{
    if (!_seaWaveOpen)
        return;
    _seaWaveOpen = false;
    GLint drawFbo = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _ssaaFbo);
    const GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(_surfTargetOn ? 2 : 1, bufs);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(drawFbo));
}

bool EngineGL33::SeaWaveThisFrame() const
{
    return _seaWaveTex && _seaWaveFrameValid && _seaWaveFrame == GetFrameCounter();
}
