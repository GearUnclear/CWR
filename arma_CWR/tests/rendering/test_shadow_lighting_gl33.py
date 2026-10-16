#!/usr/bin/env python3
"""Historical semantic-E regressions using unmodified GLSL and EGL.

Requires Python 3 and moderngl plus an EGL/OpenGL 3.3 implementation. No game
assets are required. --source is required and must select a preserved E C++ source snapshot;
--baseline optionally compares disabled/unoccluded output to an older source.
This is not the current H2 material suite. E and H2 have different UBO layouts
and lighting contracts. Assertions are intentionally preserved, not relaxed.
This does not execute C++ material upload or establish visual/performance quality.
"""
import argparse
from dataclasses import dataclass, replace
import hashlib
import json
import math
from pathlib import Path
import re
import struct

import moderngl

SOURCE = Path(__file__).resolve().parents[2] / 'engine/PoseidonGL33/EngineGL33_Shaders.cpp'
IDENTITY = (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)
POSITIONS = ((-.8, -.8, 0), (.8, -.8, 0), (0, .8, 0))
FRAGMENTS = {'normal': 's_psNormalGLSL', 'detail': 's_psDetailGLSL', 'grass': 's_psGrassGLSL'}


def load_shaders(path):
    """Extract raw literals only: never rewrite shader or test source."""
    return dict(re.findall(r'static const char (s_\w+GLSL)\[\] = R"\((.*?)\)";',
                           path.read_text(), re.S))


def packed(values):
    return struct.pack(f'<{len(values)}f', *values)


@dataclass(frozen=True)
class Scene:
    fragment: str = 'normal'
    normal: tuple = (0, 0, 1)
    screen: bool = False
    shadow: bool = True
    depth: float = .1  # receiver is at light-space z=.5; .9 is unoccluded
    visibility: float = .25
    sun: float = 1
    diffuse: float = .6
    ambient: float = 0  # TRUE ambient; combined ambient UBO also contains forced
    forced: float = 0
    emissive: float = 0
    local_ambient: float = 0
    local_diffuse: float = 0
    specular: float = 0
    material_alpha: float = 1
    texture: tuple = (1, 1, 1, 1)
    detail: tuple = (.5, .5, .5, .5)
    tint: tuple = (1, 1, 1, 1)
    fog: tuple = (0, 0, 0, 0)  # start, inverse range, enabled, unused
    fog_color: tuple = (.1, .2, .3, 1)
    night: tuple = (.3, .6, .1, 1)
    grass_alpha: tuple = (.75, .5)


class Fixture:
    """One triangle, float framebuffer, controlled single-layer depth texture."""
    def __init__(self, ctx, shaders):
        self.ctx, self.shaders = ctx, shaders
        self.programs = {}

    def close(self):
        for program in self.programs.values():
            program.release()

    def pixel(self, scene):
        ctx = self.ctx
        key = (scene.screen, scene.fragment)
        if key not in self.programs:
            self.programs[key] = ctx.program(
                vertex_shader=self.shaders['s_vsScreenGLSL' if scene.screen else 's_vsTransformGLSL'],
                fragment_shader=self.shaders[FRAGMENTS[scene.fragment]])
        program = self.programs[key]
        resources = []

        def keep(resource):
            resources.append(resource)
            return resource

        def slot(data, index, values):
            data[index * 4:index * 4 + len(values)] = values

        try:
            vs = [0.] * 280
            for index in (0, 4, 8, 24, 28):
                slot(vs, index, IDENTITY)
            slot(vs, 12, (0, 0, -1, 0))
            slot(vs, 13, (scene.ambient + scene.forced,) * 3 + (0,))
            slot(vs, 14, (scene.diffuse,) * 3 + (0,))
            slot(vs, 15, (scene.emissive,) * 3 + (scene.material_alpha,))
            slot(vs, 16, scene.fog)
            slot(vs, 17, (0, 0, 10, 0))
            slot(vs, 18, (scene.specular,) * 3 + (1,))
            slot(vs, 19, (int(scene.specular != 0), 0, 0, 0))
            slot(vs, 20, (scene.sun, 0, 0, 0))
            slot(vs, 21, (2 / 65, 2 / 65, 0, 0))
            slot(vs, 22, (scene.forced,) * 3 + (0,))
            slot(vs, 33, (int(scene.local_ambient != 0 or scene.local_diffuse != 0), 0, 0, 0))
            slot(vs, 34, (0, 0, 10, 100))  # no distance attenuation at triangle
            slot(vs, 42, (scene.local_diffuse,) * 3 + (0,))
            slot(vs, 50, (scene.local_ambient,) * 3 + (0,))
            ps = [0.] * 108
            slot(ps, 0, scene.fog_color)
            slot(ps, 2, (int(scene.shadow), 0, scene.visibility, 0))
            slot(ps, 3, scene.tint)
            slot(ps, 5, (0, 0, 0, scene.grass_alpha[0]))
            slot(ps, 6, (0, 0, 0, scene.grass_alpha[1]))
            slot(ps, 7, scene.night)
            light_vp = list(IDENTITY)
            light_vp[14] = .5
            slot(ps, 8, light_vp)
            slot(ps, 24, (100,) * 4)
            slot(ps, 25, (1, 1, 0, 1))
            slot(ps, 26, (0, 0, 1, 0))
            for name, binding, values in (
                ('VSConstants', 0, vs), ('PSConstants', 1, ps),
                ('WorldInstances', 2, list(IDENTITY) + [0.] * (4096 - 16)),
            ):
                if name in program:
                    program[name].binding = binding
                    keep(ctx.buffer(packed(values))).bind_to_uniform_block(binding)
            vertices = []
            for position in POSITIONS:
                if scene.screen:
                    x, y, z = position
                    vertices.extend(((x + 1) * 32.5, (1 - y) * 32.5, z, 1,
                                     .2, .4, .6, .8, .01, .02, .03, 1, .5, .5, .5, .5))
                else:
                    vertices.extend((*position, *scene.normal, .5, .5))
            layout = (('3f 1f 4f 4f 2f 2f', 'aPos', 'aRhw', 'aColor', 'aSpecular', 'aUV0', 'aUV1')
                      if scene.screen else ('3f 3f 2f', 'pos', 'normal', 'uv'))
            buffer = keep(ctx.buffer(packed(vertices)))
            vao = keep(ctx.vertex_array(program, [(buffer, *layout)], skip_errors=True))
            for unit, name, values in ((0, 'tex0', scene.texture), (1, 'tex1', scene.detail)):
                if name in program:
                    keep(ctx.texture((1, 1), 4, packed(values), dtype='f4')).use(unit)
                    program[name].value = unit
            keep(ctx.texture_array((1, 1, 1), 1, packed([scene.depth]), dtype='f4')).use(2)
            program['shadowMap'].value = 2
            target = keep(ctx.texture((65, 65), 4, dtype='f4'))
            framebuffer = keep(ctx.framebuffer([target]))
            framebuffer.use()
            ctx.viewport = (0, 0, 65, 65)
            framebuffer.clear()
            vao.render(mode=moderngl.TRIANGLES)
            return struct.unpack('<4f', framebuffer.read(viewport=(32, 32, 1, 1), components=4, dtype='f4'))
        finally:
            for resource in reversed(resources):
                resource.release()


def close(actual, expected):
    if len(actual) != len(expected) or any(not math.isfinite(a) or abs(a - b) > 1e-5
                                         for a, b in zip(actual, expected)):
        raise AssertionError(f'{actual} != {expected}')


def check_wiring(source, shaders):
    """Static ABI guard, not a claim that the C++ upload function was executed."""
    for name in ('s_vsTransformGLSL', 's_vsShadowGLSL'):
        clean_shader = re.sub(r'//[^\n]*', '', shaders[name])
        block = re.search(r'uniform VSConstants\s*\{(.*?)\}', clean_shader, re.S).group(1)
        offset, offsets = 0, {}
        for kind, field, count in re.findall(r'(mat4|vec4)\s+(\w+)(?:\[(\d+)\])?\s*;', block):
            offsets[field] = offset
            offset += (4 if kind == 'mat4' else 1) * int(count or 1)
        for field, expected in {'vpScale': 21, 'forcedSun': 22, '_pad23': 23,
                                'texMat0': 24, 'texMat1': 28, 'texCtrl': 32}.items():
            if offsets.get(field) != expected:
                raise AssertionError(f'{name}: {field} is not at slot {expected}')
    patterns = (
        r'const\s+Color\s+forced\s*=\s*sun->Diffuse\(\)\s*\*\s*mat\.forcedDiffuse\s*;',
        r'memcpy\(\s*s_vsShadow\s*\+\s*22\s*\*\s*4\s*,\s*forcedSun\s*,\s*16\s*\)',
        r'vForcedSun\s*=\s*forcedSun\.rgb\s*\*\s*sunEn\.x\s*;',
    )
    for pattern in patterns:
        if not re.search(pattern, source):
            raise AssertionError(f'Missing forced-sun wiring: {pattern}')


def run(fixture, baseline=None):
    rows = []

    def check(label, scene, expected):
        actual = fixture.pixel(scene)
        try:
            close(actual, expected)
        except AssertionError as error:
            raise AssertionError(f'{scene.fragment}/{label}: {error}') from error
        rows.append({'case': f'{scene.fragment}/{label}', 'pixel': actual, 'result': 'PASS'})

    for fragment in FRAGMENTS:
        base = Scene(fragment=fragment)
        for normal in ((1, 0, 0), (0, 0, 1)):
            for visibility in (0, .25, 1):
                for sun in (0, 1):
                    for mixed in (False, True):
                        scene = replace(base, normal=normal, visibility=visibility, sun=sun,
                                        diffuse=.4, forced=.2, ambient=.12 if mixed else 0,
                                        emissive=.07 if mixed else 0, local_ambient=.09 if mixed else 0)
                        expected = scene.emissive + scene.local_ambient + sun * (
                            scene.ambient + visibility * (.2 + .4 * normal[2]))
                        label = f'forced/n{normal[2]}/v{visibility}/sun{sun}/mixed{mixed}'
                        check(label, scene, (min(1, expected),) * 3 + (1,))
                        for mode, changes in (('disabled', {'shadow': False}), ('unoccluded', {'depth': .9})):
                            unshadowed = replace(scene, **changes)
                            value = scene.emissive + scene.local_ambient + sun * (scene.ambient + .2 + .4 * normal[2])
                            check(label + '/' + mode, unshadowed, (min(1, value),) * 3 + (1,))
                            if baseline:
                                close(fixture.pixel(unshadowed), baseline.pixel(unshadowed))
        for label, options, expected in (
            ('true_ambient', {'diffuse': 0, 'ambient': .3}, .3),
            ('emissive', {'diffuse': 0, 'emissive': .2}, .2),
            ('local_ambient', {'diffuse': 0, 'local_ambient': .2}, .2),
            ('direct', {'diffuse': .6}, .15),
            ('saturated_input', {'diffuse': 1.2, 'ambient': .2}, .5),
            ('material_products', {'ambient': .5 * 1.6, 'diffuse': .6 * .4, 'forced': .6 * .4}, .92),
        ):
            check(label, replace(base, **options), (expected,) * 3 + (1,))
        # Interpolated vertex-local Lambert contribution: exact centre weights 1/4,1/4,1/2.
        local = .2 * (.5 * 10 / math.sqrt(101.28) + .5 * 10 / math.sqrt(100.64))
        for sun in (0, 1):
            check(f'local_diffuse/sun{sun}', replace(base, sun=sun, diffuse=0, local_diffuse=.2),
                  (local,) * 3 + (1,))
        if fragment != 'grass':
            scene = replace(base, diffuse=0, specular=.2)
            lit = fixture.pixel(replace(scene, shadow=False))
            check('sun_specular', scene, tuple(x * .25 for x in lit[:3]) + (1,))
            check('sun_specular_off', replace(scene, sun=0), (0, 0, 0, 1))
            scene = replace(base, material_alpha=.8, texture=(1, 1, 1, .6), tint=(.5, .6, .7, .5))
            check('tint_and_alpha', scene, (.075, .09, .105, .24))
            scene = replace(base, diffuse=0, emissive=.4, texture=(1, .5, .25, 1), night=(.3, .6, .1, 0))
            luminance = .25
            check('night_coefficients', scene, tuple(luminance * .75 + x * .25 for x in (.4, .2, .1)) + (1,))
        else:
            check('grass_alpha', replace(base, grass_alpha=(.6, .4), detail=(.5, .5, .5, .3)),
                  (.15, .15, .15, .4))
            check('grass_detail_after_saturation', replace(base, diffuse=1.2, ambient=.2, detail=(.8,) * 4),
                  (.8, .8, .8, 1))
        if fragment == 'detail':
            check('detail_alpha_modulates_rgb', replace(base, detail=(.8, .7, .6, .3)), (.09, .09, .09, 1))
        # Complete fog masks lighting. Partial fog also weakens the shadow first.
        check('full_fog', replace(base, fog=(-100, 1, 1, 0)), (.1, .2, .3, 1))
        fog_factor = 1 - .05 * (.5 * math.sqrt(101.28) + .5 * math.sqrt(100.64))
        light = .6 * (1 - .75 * fog_factor)
        expected = tuple(c * (1 - fog_factor) + light * fog_factor for c in (.1, .2, .3)) + (1,)
        check('partial_fog_shadow_fade', replace(base, fog=(0, .05, 1, 0)), expected)
        prelit = (.2, .4, .6, 1) if fragment == 'grass' else (.21, .42, .63, 1.8 if fragment == 'detail' else .8)
        check('prelit_disabled', replace(base, screen=True, forced=.2, shadow=False), prelit)
        check('prelit_fallback', replace(base, screen=True, forced=.2),
              tuple(x * .25 for x in prelit[:3]) + prelit[3:])
        # Shadowing never changes alpha, including sun-disabled meshes.
        for sun in (0, 1):
            scene = replace(base, sun=sun, material_alpha=.73, texture=(1, 1, 1, .61))
            close(fixture.pixel(scene)[3:], fixture.pixel(replace(scene, shadow=False))[3:])
            rows.append({'case': f'{fragment}/alpha_shadow_invariance/sun{sun}', 'result': 'PASS'})
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True, help='Preserved E source; do not use H2/current production')
    parser.add_argument('--baseline', type=Path)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    source = args.source.read_text()
    shaders = load_shaders(args.source)
    check_wiring(source, shaders)
    ctx = moderngl.create_standalone_context(backend='egl', require=330)
    fixture = Fixture(ctx, shaders)
    baseline = Fixture(ctx, load_shaders(args.baseline)) if args.baseline else None
    try:
        rows = run(fixture, baseline)
        rows.append({'case': 'reserved_ubo_slot22/static_wiring', 'result': 'PASS'})
        report = {'source': str(args.source.resolve()),
                  'source_sha256': hashlib.sha256(args.source.read_bytes()).hexdigest(),
                  'baseline': str(args.baseline.resolve()) if args.baseline else None,
                  'renderer': ctx.info['GL_RENDERER'], 'version': ctx.info['GL_VERSION'],
                  'scope': 'Semantic E, synthetic actual GLSL; C++ upload static only; no asset or hardware acceptance',
                  'tests': len(rows), 'results': rows}
        encoded = json.dumps(report, indent=2)
        print(encoded)
        if args.report:
            args.report.write_text(encoded + '\n')
    finally:
        fixture.close()
        if baseline:
            baseline.close()
        ctx.release()


if __name__ == '__main__':
    main()
