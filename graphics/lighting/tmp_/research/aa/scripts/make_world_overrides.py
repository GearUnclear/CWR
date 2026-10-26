"""Build the aa world-shader overrides from the dumped engine GLSL.

psRemasterCommon.glsl: decodes the research selector j from the saturation
knob (triSetLighting saturation=1.12+0.001*j), restores the exact saturation,
and lets finishFragment blend alpha-to-coverage toward the raw (mip-averaged)
alpha at distance (j = 2, 4).
psNormalRemasterMain.glsl: computes the minification level of tex0 and, for
alpha-tested draws, scales alpha up per mip level (j = 1, 3: Golus-style
coverage preservation) before the alpha test / coverage sharpening.
"""
import sys
from pathlib import Path

dump = Path(sys.argv[1])
out = Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)
common = (dump / "psRemasterCommon.glsl").read_text(encoding="utf-8")

def sub(text, old, new, count=1):
    n = text.count(old)
    if n != count:
        raise SystemExit(f"expected {count} of {old!r}, found {n}")
    return text.replace(old, new)

# 1. selector + saturation decode, globals for the coverage logic
common = sub(common, "const vec3 kLuma = vec3(0.2126, 0.7152, 0.0722);",
"""const vec3 kLuma = vec3(0.2126, 0.7152, 0.0722);

// ---- research track aa: selector j rides on the saturation knob
int aaSel() { return litFlags.w > 1.1195 ? int(floor((litFlags.w - 1.12) * 1000.0 + 0.5)) : 0; }
float aaSaturation() { return litFlags.w - 0.001 * float(aaSel()); }
float g_alphaLod = 0.0;   // minification level of the cutout texture (0 = 1:1 or magnified)
float g_rawAlpha = -1.0;  // alpha before any mip scaling (-1 = not a cutout main)
""")
common = sub(common, "return max(mix(vec3(grey), display, litFlags.w), vec3(0.0));",
                     "return max(mix(vec3(grey), display, aaSaturation()), vec3(0.0));")
# 2. coverage toward the mip-averaged alpha at distance
common = sub(common, """        float cov = clamp((r0.a - alphaRef.x) / max(fwidth(r0.a), 1e-4) + 0.5, 0.0, 1.0);
        if (cov <= 0.0) discard;""",
"""        float cov = clamp((r0.a - alphaRef.x) / max(fwidth(r0.a), 1e-4) + 0.5, 0.0, 1.0);
        int aaJ = aaSel();
        if ((aaJ == 2 || aaJ == 4) && g_rawAlpha >= 0.0) {
            // A minified cutout's mip alpha is the leaf fraction of the pixel's
            // footprint: use it as the coverage instead of thresholding it at
            // 0.75, which thins distant crowns.
            float far = smoothstep(0.5, 2.0, g_alphaLod);
            cov = mix(cov, clamp(g_rawAlpha * (aaJ == 4 ? 1.15 : 1.0), 0.0, 1.0), far);
        }
        if (cov <= 0.0) discard;""")
(out / "psRemasterCommon.glsl").write_text(common, encoding="utf-8", newline="\n")

main = """void main() {
    vec4 t0 = texture(tex0, vUV0);
    // ---- research track aa: alpha coverage preservation for cutouts
    int aaJ = aaSel();
    // Opaque cutouts only (alpha ref 0xc0). Blended draws (camouflage nets, glass,
    // flares) also run the alpha test, with ref 1/255: leave them alone.
    if (aaJ > 0 && alphaRef.y > 0.5 && alphaRef.x > 0.5) {
        vec2 ts = vec2(textureSize(tex0, 0));
        vec2 dx = dFdx(vUV0 * ts);
        vec2 dy = dFdy(vUV0 * ts);
        // minor axis: with anisotropic filtering the hardware samples about this level
        g_alphaLod = max(0.5 * log2(max(min(dot(dx, dx), dot(dy, dy)), 1e-8)), 0.0);
        g_rawAlpha = vColor.a * t0.a * constColor.a;
        bool a2c = alphaRef.z > 0.5;
        if (aaJ == 1) {
            // Golus-style mip scale (dilates: the effective threshold keeps falling)
            t0.a = min(t0.a * (1.0 + 0.5 * g_alphaLod), 1.0);
        } else if (aaJ == 3 || (aaJ == 2 && !a2c) || (aaJ == 4 && !a2c)) {
            // The authored mips are 1-bit with preserved coverage; what thins a
            // minified crown is the 0.753 threshold on bilinear/trilinear/aniso
            // filtered alpha. Move the effective threshold to the unbiased 0.5
            // once the texture is minified.
            float s = mix(1.0, alphaRef.x / 0.5, smoothstep(0.0, 1.0, g_alphaLod));
            t0.a = min(t0.a * s, 1.0);
        }
    }
    vec4 r0;
    float ambientShare = 0.0;
    if (vLitMode > 0.5) {
        r0.rgb = shadeRemastered(t0.rgb * constColor.rgb, false, ambientShare);
        r0.a = vColor.a * t0.a * constColor.a;
    } else {
        r0 = vColor * t0;
        r0 *= constColor;
        r0.rgb += vSpecColor.rgb;
    }
    finishFragment(r0, ambientShare, true);
}
"""
orig_main = (dump / "psNormalRemasterMain.glsl").read_text(encoding="utf-8")
assert "finishFragment(r0, ambientShare, true);" in orig_main
(out / "psNormalRemasterMain.glsl").write_text(main, encoding="utf-8", newline="\n")
print("wrote", out)
