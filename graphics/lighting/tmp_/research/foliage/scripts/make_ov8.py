"""ov8 = ov7 with a gentler, gated leaf tint: the chroma boost is mixed back
toward white, and the leaf-green lean applies only to saturated textures whose
green is above their blue (leaves), not to white flowers, blue-grey foliage or
bark."""
from pathlib import Path

P = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\foliage\ov8\psRemasterCommon.glsl")
s = P.read_text(encoding="utf-8")
crlf = "\r\n" in s


def R(a, b):
    global s
    if crlf:
        a = a.replace("\n", "\r\n")
        b = b.replace("\n", "\r\n")
    assert s.count(a) == 1, (a[:70], s.count(a))
    s = s.replace(a, b)


R("""    vec3 tint = pow(clamp(a / la, 0.0, 4.0), vec3(0.8));
    tint /= max(dot(tint, kLuma), 1e-5);""",
  """    vec3 chroma = clamp(a / la, 0.0, 4.0);
    // How leaf-like the texel is: saturated, and green above blue. White
    // flowers, blue-grey foliage and bark keep their own colour.
    float amax = max(a.r, max(a.g, a.b));
    float asat = (amax - min(a.r, min(a.g, a.b))) / max(amax, 1e-5);
    float leafy = smoothstep(0.15, 0.45, asat) * smoothstep(-0.02, 0.06, (a.g - a.b) / la);
    vec3 tint = mix(vec3(1.0), chroma, 0.7);
    tint /= max(dot(tint, kLuma), 1e-5);""")
R("""    vec3 tintT = mix(tint, kLeafTrans, tintGreen * tintGreenGate);
    tint = mix(vec3(1.0), mix(tint, kLeafTrans, 0.3 * tintGreen), tintGreenGate);""",
  """    vec3 tintT = mix(tint, kLeafTrans, tintGreen * tintGreenGate * leafy);
    tint = mix(vec3(1.0), mix(tint, kLeafTrans, 0.3 * tintGreen * leafy), tintGreenGate);""")
P.write_bytes(s.encode("utf-8"))
print("ov8", P.stat().st_size)
