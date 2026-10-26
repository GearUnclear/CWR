"""Assemble a shader override dir: original remastered common + research slots
+ the materials GLSL, and main bodies that switch on litResearch.x."""
import sys, os
here = os.path.dirname(os.path.abspath(__file__))
root = os.path.dirname(here)
out = sys.argv[1]
extra = sys.argv[2] if len(sys.argv) > 2 else os.path.join(here, 'materials_glsl.txt')
os.makedirs(out, exist_ok=True)
common = open(os.path.join(root, 'orig', 'psRemasterCommon.glsl')).read()
anchor = '    vec4 litLightDir[8];  // 50-57: xyz beam direction\n'
assert anchor in common
common = common.replace(anchor, anchor +
    '    vec4 litMatClass;     // 58: x surface class, y object kind, z blend texture, w texture hash\n'
    '    vec4 litWeather;      // 59: x wetness, y rain density, z puddles, w time (s)\n'
    '    vec4 litResearch;     // 60: x materials on, y albedo bump, z r1, w r2\n')
common += open(extra).read()
open(os.path.join(out, 'psRemasterCommon.glsl'), 'w', newline='\n').write(common)

normal = r'''
void main() {
    vec4 t0 = texture(tex0, vUV0);
    vec4 r0;
    float ambientShare = 0.0;
    if (vLitMode > 0.5) {
        if (litResearch.x > 0.5) {
            g_surface = 0;
            r0.rgb = shadeMaterials(t0.rgb * constColor.rgb, false, ambientShare);
            r0.a = vColor.a * t0.a * constColor.a;
            // Blended glass: the reflected part is not see-through.
            if (litFlags.x < 0.5 && alphaRef.z < 0.5) r0.a = mix(r0.a, 1.0, clamp(g_envF, 0.0, 1.0));
        } else {
            r0.rgb = shadeRemastered(t0.rgb * constColor.rgb, false, ambientShare);
            r0.a = vColor.a * t0.a * constColor.a;
        }
    } else {
        r0 = vColor * t0;
        r0 *= constColor;
        r0.rgb += vSpecColor.rgb;
    }
    finishFragment(r0, ambientShare, true);
}
'''
detail = r'''
void main() {
    vec4 t0 = texture(tex0, vUV0);
    vec4 t1 = texture(tex1, vUV1);
    vec4 r0;
    float ambientShare = 0.0;
    if (vLitMode > 0.5) {
        // Detail modulation stays in display space, as in the classic shader,
        // so the terrain keeps its authored micro-contrast.
        vec3 albedo = t0.rgb * constColor.rgb * (t1.a * 2.0);
        if (litResearch.x > 0.5) {
            g_surface = 1;
            g_detailTex = t1;
            r0.rgb = shadeMaterials(albedo, true, ambientShare);
        } else {
            r0.rgb = shadeRemastered(albedo, true, ambientShare);
        }
        r0.a = vColor.a * t0.a * constColor.a;
    } else {
        r0 = vColor * t0;
        r0 *= constColor;
        r0.rgb *= t1.a * 2.0;
        r0 += vSpecColor;
    }
    finishFragment(r0, ambientShare, true);
}
'''
grass = r'''
void main() {
    vec4 t0 = texture(tex0, vUV0);
    vec4 t1 = texture(tex1, vUV1);
    if (vFogTC < 0.0) discard;
    vec4 r0;
    float ambientShare = 0.0;
    r0.a = clamp((psC5.a * 2.0 - 1.0) + t1.a, 0.0, 1.0);
    if (vLitMode > 0.5) {
        vec3 albedo = clamp(t0.rgb * t1.rgb * 2.0, 0.0, 1.0);
        if (litResearch.x > 0.5) {
            g_surface = 2;
            g_detailTex = t1;
            r0.rgb = shadeMaterials(albedo, true, ambientShare);
        } else {
            r0.rgb = shadeRemastered(albedo, true, ambientShare);
        }
    } else {
        r0.rgb = clamp(vColor.rgb * t0.rgb * t1.rgb * 2.0, 0.0, 1.0);
    }
    r0.a = clamp(psC6.a * r0.a * 2.0, 0.0, 1.0);
    finishFragment(r0, ambientShare, false);
}
'''
for name, body in (('psNormalRemasterMain', normal), ('psDetailRemasterMain', detail), ('psGrassRemasterMain', grass)):
    open(os.path.join(out, name + '.glsl'), 'w', newline='\n').write(body)
ssr = sys.argv[3] if len(sys.argv) > 3 else os.path.join(root, 'orig', 'psPostSsr.glsl')
if os.path.exists(ssr):
    open(os.path.join(out, 'psPostSsr.glsl'), 'w', newline='\n').write(open(ssr).read())
print('wrote', out)
