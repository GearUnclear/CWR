#!/usr/bin/env python3
"""Actual-GLSL regression guard for the H2/H3 hemisphere lighting model.

Select a frozen C++ snapshot with --source. Production is the default, but later
experiments may intentionally fail this contract. --palette h3 is the default;
use --palette h2 for the preserved H2 coefficients. Selection is explicit, never
inferred from source. E tests remain separate and unchanged. Requires moderngl
and EGL/OpenGL 3.3; no game assets are used.
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
LUMA = (.2126, .7152, .0722)
PALETTES = {'h2': (.70, .65, .52), 'h3': (.60, .66, .63)}
SKY, GROUND = (.55, .60, .70), (.28, .23, .16)


def extract(path):
    return extract_text(path.read_text())


def extract_text(text):
    return dict(re.findall(r'static const char (s_\w+GLSL)\[\] = R"\((.*?)\)";', text, re.S))


def packed(values):
    return struct.pack(f'<{len(values)}f', *values)


def close(actual, expected):
    if len(actual) != len(expected) or any(not math.isfinite(a) or abs(a-b) > 1e-5 for a, b in zip(actual, expected)):
        raise AssertionError(f'{actual} != {expected}')


@dataclass(frozen=True)
class Scene:
    fragment: str = 'normal'
    normal: tuple = (0, 0, 1)
    daylight: float = 1
    sun: float = 1
    ambient: tuple = (.4, .4, .4)  # true ambient, also supplied separately at slot22
    forced: float = 0            # combined slot13 includes this; separate slot23
    diffuse: float = 0
    emissive: float = 0
    local: float = 0             # one local ambient light, inside startAtten
    visibility: float = .25
    shadow: bool = False
    depth: float = .1
    albedo: tuple = (.5, .5, .5)
    material_alpha: float = 1
    texture_alpha: float = 1
    tint: tuple = (1, 1, 1, 1)
    detail: tuple = (.5, .5, .5, .5)
    specular: float = 0
    fog: tuple = (0, 0, 0, 0)  # start, inverse range, enabled, I height-fog opt-in
    fog_color: tuple = (.1, .2, .3, 1)
    screen: bool = False


class Fixture:
    """Unmodified extracted shader pair, UBO inputs, constant map and float target."""
    def __init__(self, ctx, shaders):
        self.ctx, self.shaders, self.programs = ctx, shaders, {}

    def close(self):
        for program in self.programs.values():
            program.release()

    def pixel(self, s):
        key = (s.fragment, s.screen)
        ctx = self.ctx
        if key not in self.programs:
            self.programs[key] = ctx.program(
                vertex_shader=self.shaders['s_vsScreenGLSL' if s.screen else 's_vsTransformGLSL'],
                fragment_shader=self.shaders[FRAGMENTS[s.fragment]])
        program = self.programs[key]
        resources = []

        def keep(resource):
            resources.append(resource)
            return resource

        def slot(buffer, index, values):
            buffer[4*index:4*index+len(values)] = values

        try:
            vs, ps = [0.] * 280, [0.] * 108
            for index in (0, 4, 8, 24, 28):
                slot(vs, index, IDENTITY)
            slot(vs, 12, (0, 0, -1, 0))
            slot(vs, 13, tuple(a+s.forced for a in s.ambient)+(0,))
            slot(vs, 14, (s.diffuse,)*3+(0,))
            slot(vs, 15, (s.emissive,)*3+(s.material_alpha,))
            slot(vs, 16, s.fog)
            slot(vs, 17, (0, 0, 10, 0))
            slot(vs, 18, (s.specular,)*3+(1,))
            slot(vs, 19, (int(s.specular != 0), 0, 0, 0))
            slot(vs, 20, (s.sun, 0, 0, 0))
            slot(vs, 21, (2/65, 2/65, 0, 0))
            slot(vs, 22, s.ambient+(s.daylight,))
            slot(vs, 23, (s.forced,)*3+(0,))
            slot(vs, 33, (int(s.local != 0), 0, 0, 0))
            slot(vs, 34, (0, 0, 10, 100))
            slot(vs, 50, (s.local,)*3+(0,))
            slot(ps, 0, s.fog_color)
            slot(ps, 2, (int(s.shadow), 0, s.visibility, 0))
            slot(ps, 3, s.tint)
            slot(ps, 5, (0, 0, 0, .75))
            slot(ps, 6, (0, 0, 0, .5))
            slot(ps, 7, (.3, .6, .1, 1))  # preserve colour, no night postprocess
            light_vp = list(IDENTITY)
            light_vp[14] = .5
            slot(ps, 8, light_vp)
            slot(ps, 24, (100,)*4)
            slot(ps, 25, (1, 1, 0, 1))
            slot(ps, 26, (0, 0, 1, 0))
            for name, binding, data in (
                ('VSConstants', 0, vs), ('PSConstants', 1, ps),
                ('WorldInstances', 2, list(IDENTITY)+[0.]*(4096-16)),
            ):
                if name in program:
                    program[name].binding = binding
                    keep(ctx.buffer(packed(data))).bind_to_uniform_block(binding)
            vertices = []
            for position in POSITIONS:
                if s.screen:
                    x, y, z = position
                    vertices.extend(((x+1)*32.5, (1-y)*32.5, z, 1,
                                     .2, .4, .6, .8, .01, .02, .03, 1, .5, .5, .5, .5))
                else:
                    vertices.extend((*position, *s.normal, .5, .5))
            layout = (('3f 1f 4f 4f 2f 2f', 'aPos', 'aRhw', 'aColor', 'aSpecular', 'aUV0', 'aUV1')
                      if s.screen else ('3f 3f 2f', 'pos', 'normal', 'uv'))
            buffer = keep(ctx.buffer(packed(vertices)))
            vao = keep(ctx.vertex_array(program, [(buffer, *layout)], skip_errors=True))
            for unit, name, value in ((0, 'tex0', s.albedo+(s.texture_alpha,)), (1, 'tex1', s.detail)):
                if name in program:
                    keep(ctx.texture((1, 1), 4, packed(value), dtype='f4')).use(unit)
                    program[name].value = unit
            keep(ctx.texture_array((1, 1, 1), 1, packed([s.depth]), dtype='f4')).use(2)
            program['shadowMap'].value = 2
            output = keep(ctx.texture((65, 65), 4, dtype='f4'))
            fbo = keep(ctx.framebuffer([output]))
            fbo.use()
            ctx.viewport = (0, 0, 65, 65)
            fbo.clear()
            vao.render(mode=moderngl.TRIANGLES)
            return struct.unpack('<4f', fbo.read(viewport=(32, 32, 1, 1), components=4, dtype='f4'))
        finally:
            for resource in reversed(resources):
                resource.release()


def hemisphere(normal, horizon):
    up = normal[1] / math.sqrt(sum(x*x for x in normal))
    endpoint = SKY if up >= 0 else GROUND
    return tuple(h+(end-h)*abs(up) for h, end in zip(horizon, endpoint))


def shoulder(rgb):
    """Independent scalar luminance contract; preserves environmental RGB ratios."""
    luminance = sum(a*b for a, b in zip(rgb, LUMA))
    if luminance <= .55:
        return rgb
    excess = luminance - .55
    mapped = .55 + excess / (1+excess/.45)
    return tuple(x*mapped/luminance for x in rgb)


def expected(s, horizon):
    """Independent hemisphere oracle, with legacy fog and full-colour postprocess.

    I's opt-in height fog is deliberately disabled by fog.w=0, camera altitude=0.
    Pixel centre has barycentric weights (1/4, 1/4, 1/2).
    """
    day = s.daylight * s.sun
    weights = (.25, .25, .5)
    distances = [math.sqrt(x*x+y*y+100) for x, y, _ in POSITIONS]
    fog = sum(w*min(1, max(0, 1-(r-s.fog[0])*s.fog[1])) for w, r in zip(weights, distances)) if s.fog[2] > .5 else 1
    visibility = s.visibility if s.shadow and s.depth < .5 else 1
    visibility = 1-(1-visibility)*fog
    direct_vis = visibility*(1-day)+visibility**2*day
    forced_vis = 1-day+direct_vis*day
    ndotl = max(0, s.normal[2]/math.sqrt(sum(x*x for x in s.normal)))
    environment = tuple(s.sun*(a*(1-day+hemi*day)+s.forced*forced_vis+s.diffuse*ndotl*direct_vis)*albedo
                        for a, hemi, albedo in zip(s.ambient, hemisphere(s.normal, horizon), s.albedo))
    protected = tuple((s.local+s.emissive)*a for a in s.albedo)
    raw = tuple(e+p for e, p in zip(environment, protected))
    mapped = tuple(e+p for e, p in zip(shoulder(environment), protected))
    if s.fragment == 'grass':
        rgb = tuple(min(1, max(0, a*detail*2))*(1-day)+b*detail*2*day
                    for a, b, detail in zip(raw, mapped, s.detail))
        alpha = min(1, max(0, .5+s.detail[3]))
    else:
        rgb = tuple((min(1, max(0, a))*(1-day)+b*day)*tint for a, b, tint in zip(raw, mapped, s.tint))
        if s.fragment == 'detail':
            rgb = tuple(x*s.detail[3]*2 for x in rgb)
        normal_length = math.sqrt(sum(x*x for x in s.normal))
        spec = 0
        for weight, (x, y, _), distance in zip(weights, POSITIONS, distances):
            half = (-x/distance, -y/distance, 1+10/distance)
            half_length = math.sqrt(sum(v*v for v in half))
            ndoth = max(0, sum(a*b for a, b in zip(s.normal, half))/(normal_length*half_length))
            spec += weight*min(1, s.specular*s.sun*ndoth)
        rgb = tuple(x+spec*visibility for x in rgb)
        alpha = min(1, max(0, s.material_alpha))*s.texture_alpha*s.tint[3]
    rgb = tuple(f*(1-fog)+x*fog for x, f in zip(rgb, s.fog_color))
    return rgb+(alpha,)


def check_wiring(source, shaders):
    """Shared H2/H3 ABI guard; this does not execute C++ material uploads."""
    for name in ('s_vsTransformGLSL', 's_vsShadowGLSL'):
        clean = re.sub(r'//[^\n]*', '', shaders[name])
        block = re.search(r'uniform VSConstants\s*\{(.*?)\}', clean, re.S).group(1)
        offset, offsets = 0, {}
        for kind, field, count in re.findall(r'(mat4|vec4)\s+(\w+)(?:\[(\d+)\])?\s*;', block):
            offsets[field] = offset
            offset += (4 if kind == 'mat4' else 1)*int(count or 1)
        for field, wanted in {'vpScale':21, 'skyAmbient':22, 'forcedSun':23, 'texMat0':24, 'texMat1':28}.items():
            if offsets.get(field) != wanted:
                raise AssertionError(f'H2/H3 ABI: {name}.{field} must occupy slot {wanted}')
    patterns = (
        r'Color\s+trueAmbient\s*=\s*sun->Ambient\(\)\s*\*\s*mat\.ambient\s*;',
        r'Color\s+forced\s*=\s*sun->Diffuse\(\)\s*\*\s*mat\.forcedDiffuse\s*;',
        r'float\s+sky\[4\]\s*=\s*\{trueAmbient\.R\(\),\s*trueAmbient\.G\(\),\s*trueAmbient\.B\(\),\s*1\.0f\s*-\s*sun->NightEffect\(\)\}',
        r'float\s+forcedRGB\[4\]\s*=\s*\{forced\.R\(\),\s*forced\.G\(\),\s*forced\.B\(\),\s*0\.0f\}',
        r'memcpy\(\s*s_vsShadow\s*\+\s*22\s*\*\s*4\s*,\s*sky\s*,\s*16\s*\)',
        r'memcpy\(\s*s_vsShadow\s*\+\s*23\s*\*\s*4\s*,\s*forcedRGB\s*,\s*16\s*\)',
        r'vSkyAmbient\s*=\s*vec4\(skyAmbient\.rgb\s*\*\s*sunEn\.x,\s*skyAmbient\.w\s*\*\s*sunEn\.x\)',
        r'vForcedSun\s*=\s*forcedSun\.rgb\s*\*\s*sunEn\.x\s*;',
    )
    for pattern in patterns:
        if not re.search(pattern, source):
            raise AssertionError(f'H2/H3 upload/varying wiring missing: {pattern}')


def run(fixture, baseline=None, *, palette='h3'):
    horizon = PALETTES[palette]
    rows = []

    def check(label, scene, target=None):
        actual = fixture.pixel(scene)
        try:
            close(actual, expected(scene, horizon) if target is None else target)
        except AssertionError as error:
            raise AssertionError(f'{scene.fragment}/{label}: {error}') from error
        rows.append({'case': f'{scene.fragment}/{label}', 'pixel': actual, 'result': 'PASS'})
        return actual

    for fragment in FRAGMENTS:
        base = Scene(fragment=fragment)
        for y in (-1, -.5, -1e-4, 0, 1e-4, .5, 1):
            n = (math.sqrt(max(0, 1-y*y)), y, 0)
            for day in (0, .5, 1):
                check(f'hemisphere/y{y}/day{day}', replace(base, normal=n, daylight=day))
        # Lateral cool balance is input-dependent, not intrinsically neutral.
        lateral = check('blue_sky_neutral_cool_lateral', replace(base, ambient=(.3, .35, .45), albedo=(1,)*3))
        lateral_targets = {'h2': (.21, .2275, .234, 1), 'h3': (.18, .231, .2835, 1)}
        close(lateral, lateral_targets[palette])
        for channel in ('diffuse', 'forced'):
            for day in (0, .25, .5, 1):
                for visibility in (0, .25, .5, 1):
                    scene = replace(base, ambient=(0,)*3, daylight=day, shadow=True,
                                    visibility=visibility, **{channel: .4})
                    check(f'{channel}/day{day}/visibility{visibility}', scene)
                    check(f'{channel}/day{day}/visibility{visibility}/unoccluded', replace(scene, depth=.9))
        # Independently check monotonic luminance and chromaticity across the knee.
        previous_luminance = -1
        for strength in (0, .1, .5499, .55, .5501, .8, 1, 2, 4, 16):
            # Compensate hemisphere to supply known post-hemisphere colour ratios.
            color = (.8, 1, 1.2)
            rgb = tuple(strength*c/h for c, h in zip(color, horizon))
            pixel = check(f'shoulder/strength{strength}', replace(base, ambient=rgb, albedo=(1,)*3))
            lum = sum(a*b for a, b in zip(pixel, LUMA))
            if lum < previous_luminance-1e-6 or lum >= 1+1e-6:
                raise AssertionError('Environmental shoulder is not monotonic/bounded in luminance')
            previous_luminance = lum
            if strength:
                close((pixel[0]/pixel[1], pixel[2]/pixel[1]), (.8, 1.2))
        for field in ('emissive', 'local'):
            for day in (0, .25, .5, .75, 1):
                for power in (.12, 2):
                    scene = replace(base, ambient=(2,)*3, albedo=(.8,)*3, daylight=day, **{field: power})
                    check(f'protected/{field}/day{day}/power{power}', scene)
                    if day == 1:
                        without = fixture.pixel(replace(scene, **{field: 0}))
                        actual = fixture.pixel(scene)
                        close(tuple(a-b for a, b in zip(actual[:3], without[:3])), (power*.8,)*3)
        for day in (0, .5, 1):
            check(f'sun_off/day{day}', replace(base, daylight=day, sun=0, emissive=.2, local=.3))
            for shadow in (False, True):
                for fog_label, fog in (('none', (0, 0, 0, 0)), ('partial', (0, .05, 1, 0)),
                                       ('complete', (-100, 1, 1, 0))):
                    # HDR tint and detail must remain after environmental shoulder;
                    # specular uses original visibility, not the daylight-square policy.
                    scene = replace(base, daylight=day, shadow=shadow, ambient=(2, 1.7, 1.4),
                                    forced=.2, diffuse=.4, emissive=.3, local=.2, specular=.8,
                                    tint=(1.8, .7, 1.2, .65), detail=(.8, .6, .7, .75),
                                    fog=fog, material_alpha=.73, texture_alpha=.61)
                    check(f'material_order/day{day}/shadow{shadow}/fog{fog_label}', scene)
                    if baseline and day == 0:
                        close(fixture.pixel(scene), baseline.pixel(scene))
            check(f'specular_linear_visibility/day{day}', replace(base, daylight=day, ambient=(0,)*3,
                                                                specular=.8, shadow=True))
            for shadow in (False, True):
                scene = replace(base, daylight=day, shadow=shadow, diffuse=.4, forced=.2,
                                material_alpha=.73, texture_alpha=.61)
                check(f'alpha/day{day}/shadow{shadow}', scene)
                prelit = replace(scene, screen=True, albedo=(1,)*3, texture_alpha=1)
                rgb = (.2, .4, .6) if fragment == 'grass' else (.21, .42, .63)
                alpha = 1 if fragment == 'grass' else (1.8 if fragment == 'detail' else .8)
                vis = .25 if shadow else 1
                check(f'TL/day{day}/shadow{shadow}', prelit, tuple(x*vis for x in rgb)+(alpha,))
                if baseline:
                    close(fixture.pixel(prelit), baseline.pixel(prelit))
        if baseline:
            for normal in ((0, 1, 0), (0, 0, 1), (0, -1, 0)):
                for shadow in (False, True):
                    scene = replace(base, normal=normal, daylight=0, shadow=shadow,
                                    forced=.1, diffuse=.4, emissive=2, local=2, albedo=(.7, .8, .9))
                    close(fixture.pixel(scene), baseline.pixel(scene))
                    rows.append({'case':f'{fragment}/night_baseline/{normal}/{shadow}', 'result':'PASS'})
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--palette', choices=tuple(PALETTES), default='h3',
                        help='Expected horizon coefficients, never auto-detected (default: h3)')
    parser.add_argument('--baseline', type=Path, help='Optional G source for night and TL identity comparisons')
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    source_bytes = args.source.read_bytes()
    source_text = source_bytes.decode('utf-8')
    shaders = extract_text(source_text)
    check_wiring(source_text, shaders)
    ctx = moderngl.create_standalone_context(backend='egl', require=330)
    fixture = Fixture(ctx, shaders)
    baseline = Fixture(ctx, extract(args.baseline)) if args.baseline else None
    try:
        rows = run(fixture, baseline, palette=args.palette)
        rows.append({'case':'H2_H3_ABI_and_upload/static_guard', 'result':'PASS'})
        report = {'source':str(args.source.resolve()), 'source_sha256':hashlib.sha256(source_bytes).hexdigest(),
                  'baseline':str(args.baseline.resolve()) if args.baseline else None,
                  'palette':args.palette, 'horizon_coefficients':PALETTES[args.palette],
                  'renderer':ctx.info['GL_RENDERER'], 'version':ctx.info['GL_VERSION'],
                  'scope':'H2/H3 actual GLSL with explicit expected palette, synthetic triangle; not C++ upload execution, visual acceptance, or hardware performance.',
                  'optional_effects':'I height-fog disabled: VS fogParam.w=0; camPos.w=0. Separate J fullscreen contact pass is not executed.',
                  'tests':len(rows), 'results':rows}
        text = json.dumps(report, indent=2)
        print(text)
        if args.report:
            args.report.write_text(text+'\n')
    finally:
        fixture.close()
        if baseline:
            baseline.close()
        ctx.release()


if __name__ == '__main__':
    main()
