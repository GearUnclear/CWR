"""Build the haze shader override dir from the dumped engine GLSL + src/haze_block.glsl.

Variant 0 (saturation knob at its 1.12 default) keeps the current Remastered
fog and shading bit for bit; see haze_block.glsl for the switch.

usage: python build_override.py <dump_dir> <out_dir>
"""
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
dump = sys.argv[1]
out = sys.argv[2]
os.makedirs(out, exist_ok=True)
block = open(os.path.join(here, "..", "src", "haze_block.glsl"), encoding="utf-8").read()


def rd(name):
    return open(os.path.join(dump, name + ".glsl"), encoding="utf-8").read()


def wr(name, text):
    with open(os.path.join(out, name + ".glsl"), "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


def sub(text, old, new, count=1):
    n = text.count(old)
    if n != count:
        raise SystemExit(f"expected {count} of {old!r}, found {n}")
    return text.replace(old, new)


# ---- fragment common
c = rd("psRemasterCommon")
c = sub(c, "// Point lights and spotlights per fragment", block + "\n// Point lights and spotlights per fragment")
c = sub(c, "return max(mix(vec3(grey), display, litFlags.w), vec3(0.0));",
        "return max(mix(vec3(grey), display, hzSaturation()), vec3(0.0));")
old_tail = """        if (litFlags.x > 0.5) r0.a = alphaRef.y > 0.5 ? 1.0 : 1.0 - ambientShare * clamp(vFogTC, 0.0, 1.0);
    }
    if (nightEye) {
        float luminance = clamp(dot(r0.rgb, rgbEyeCoef.rgb), 0.0, 1.0);
        float nightBlend = clamp(luminance + rgbEyeCoef.a, 0.0, 1.0);
        r0.rgb = mix(vec3(luminance), r0.rgb, nightBlend);
    }
    r0.rgb = mix(vLitMode > 0.5 ? fogColorToward(vWorldRel) : fogColor.rgb, r0.rgb, vFogTC);"""
new_tail = """        if (litFlags.x > 0.5) r0.a = alphaRef.y > 0.5 ? 1.0 : 1.0 - ambientShare * clamp(vFogTC, 0.0, 1.0);
    }
    if (nightEye) {
        float luminance = clamp(dot(r0.rgb, rgbEyeCoef.rgb), 0.0, 1.0);
        float nightBlend = clamp(luminance + rgbEyeCoef.a, 0.0, 1.0);
        r0.rgb = mix(vec3(luminance), r0.rgb, nightBlend);
    }
    if (vLitMode > 0.5 && hzOn()) {
        float hzT;
        r0.rgb = hzApply(r0.rgb, vWorldRel, hzT);
        if (alphaRef.z <= 0.5 && litFlags.x > 0.5 && alphaRef.y <= 0.5) r0.a = 1.0 - ambientShare * clamp(hzT, 0.0, 1.0);
    } else
    r0.rgb = mix(vLitMode > 0.5 ? fogColorToward(vWorldRel) : fogColor.rgb, r0.rgb, vFogTC);"""
c = sub(c, old_tail, new_tail)
wr("psRemasterCommon", c)

# ---- water main
w = rd("psWaterRemasterMain")
w = sub(w, "    r0.rgb = mix(vLitMode > 0.5 ? fogColorToward(vWorldRel) : fogColor.rgb, r0.rgb, vFogTC);",
        "    if (vLitMode > 0.5 && hzOn()) { float hzT; r0.rgb = hzApply(r0.rgb, vWorldRel, hzT); }\n"
        "    else r0.rgb = mix(vLitMode > 0.5 ? fogColorToward(vWorldRel) : fogColor.rgb, r0.rgb, vFogTC);")
wr("psWaterRemasterMain", w)

# ---- vertex shaders: two more outputs
for name in ("vsScreenRemaster", "vsShadowRemaster", "vsTransformRemaster"):
    v = rd(name)
    v = sub(v, "flat out float vLitMode;", "flat out float vLitMode;\nflat out vec2 vFogRange;\nout vec3 vSkyClip;")
    if name == "vsTransformRemaster":
        v = sub(v, "    vFogTC = (fogParam.z > 0.5) ? fogFactor : 1.0;",
                "    vFogTC = (fogParam.z > 0.5) ? fogFactor : 1.0;\n"
                "    vFogRange = vec2(fogParam.x, fogParam.y > 0.0 ? fogParam.x + 1.0 / fogParam.y : 1e9);\n"
                "    {\n"
                "        // The sky behind this vertex: its direction, lifted to just above the\n"
                "        // horizon when it points down, projected to the screen.\n"
                "        vec3 dr = worldPos.xyz;\n"
                "        float dl = max(length(dr), 1e-3);\n"
                "        vec3 dh = normalize(vec3(dr.x, max(dr.y, 0.03 * dl), dr.z));\n"
                "        vec4 sc = proj * vec4(mat3(view) * dh * 1000.0, 1.0);\n"
                "        vSkyClip = sc.xyw;\n"
                "    }")
    else:
        v = sub(v, "    vLitMode", "    vFogRange = vec2(0.0, 1e9);\n    vSkyClip = vec3(0.0, 0.0, 1.0);\n    vLitMode")
    wr(name, v)
print("wrote", out)
