#!/usr/bin/env python3
"""Exercise the production GLSL in an EGL context, without game assets.

Requires moderngl. Run from any directory:
    python test_sunlight_gl33.py
Optional --baseline points at the original EngineGL33_Shaders.cpp for A/B tests.
These are synthetic shader tests, not in-game or hardware-performance acceptance.
"""
import argparse
import json
from pathlib import Path
import re
import struct

import moderngl

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "engine/PoseidonGL33/EngineGL33_Shaders.cpp"
IDENTITY = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]
CURVED = [(1, 0, 0), (-1, 0, 0), (0, 0, 1)]
PLANAR = [(0, 0, 1)] * 3


def shaders(path):
    return dict(re.findall(
        r'static const char (s_\w+GLSL)\[\] = R"\((.*?)\)";',
        path.read_text(), re.S))


def floats(values):
    return struct.pack(f"<{len(values)}f", *values)


def pixel(ctx, source, normals, *, sun=1, diffuse=.6, ambient=.1,
          screen=False, fragment="s_psNormalGLSL"):
    resources = []
    def keep(resource):
        resources.append(resource)
        return resource
    try:
        program = keep(ctx.program(
            vertex_shader=source["s_vsScreenGLSL" if screen else "s_vsTransformGLSL"],
            fragment_shader=source[fragment]))
        vs = [0.] * 280
        def slot(index, values):
            vs[index * 4:index * 4 + len(values)] = values
        for index in (0, 4, 8, 24, 28):
            slot(index, IDENTITY)
        slot(12, [0, 0, -1, 0])
        slot(13, [ambient] * 3 + [0])
        slot(14, [diffuse] * 3 + [0])
        slot(15, [0, 0, 0, 1])
        slot(20, [sun, 0, 0, 0])
        slot(21, [2 / 65, 2 / 65, 0, 0])
        ps = [0.] * 108
        ps[12:16] = [1] * 4  # object tint
        ps[20:24] = [0, 0, 0, .75]  # grass alpha: .5 + texture alpha .5 = 1
        ps[24:28] = [0, 0, 0, .5]  # grass alpha multiplier
        ps[28:32] = [.3, .6, .1, 1]  # preserve full colour (normal/detail)
        for name, binding, data in (
            ("VSConstants", 0, vs), ("PSConstants", 1, ps),
            ("WorldInstances", 2, IDENTITY + [0.] * (4096 - 16)),
        ):
            if name in program:
                program[name].binding = binding
                keep(ctx.buffer(floats(data))).bind_to_uniform_block(binding)
        positions = [(-.8, -.8, 0), (.8, -.8, 0), (0, .8, 0)]
        vertices = []
        if screen:
            for x, y, z in positions:
                vertices += [(x + 1) * 32.5, (1 - y) * 32.5, z, 1,
                             .2, .4, .6, .8, .01, .02, .03, 1, .5, .5, .5, .5]
            layout = ('3f 1f 4f 4f 2f 2f', 'aPos', 'aRhw', 'aColor',
                      'aSpecular', 'aUV0', 'aUV1')
        else:
            for position, normal in zip(positions, normals):
                vertices += list(position) + list(normal) + [.5, .5]
            layout = ('3f 3f 2f', 'pos', 'normal', 'uv')
        buffer = keep(ctx.buffer(floats(vertices)))
        vao = keep(ctx.vertex_array(program, [(buffer, *layout)], skip_errors=True))
        keep(ctx.texture((1, 1), 4, floats([1, 1, 1, 1]), dtype='f4')).use(0)
        program['tex0'].value = 0
        if 'tex1' in program:
            keep(ctx.texture((1, 1), 4, floats([.5] * 4), dtype='f4')).use(1)
            program['tex1'].value = 1
        keep(ctx.texture_array((1, 1, 1), 1, floats([1]), dtype='f4')).use(2)
        program['shadowMap'].value = 2
        output = keep(ctx.texture((65, 65), 4, dtype='f4'))
        framebuffer = keep(ctx.framebuffer([output]))
        framebuffer.use()
        framebuffer.clear()
        vao.render()
        data = framebuffer.read(components=4, dtype='f4')
        return struct.unpack_from('<4f', data, (32 * 65 + 32) * 16)
    finally:
        for resource in reversed(resources):
            resource.release()


def close(actual, expected):
    if max(abs(a - b) for a, b in zip(actual, expected)) >= 1e-5:
        raise AssertionError(f'{actual} != {expected}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    ctx = moderngl.create_standalone_context(backend='egl', require=330)
    current = shaders(SOURCE)
    baseline = shaders(args.baseline) if args.baseline else None
    results = []
    for vs in ('s_vsScreenGLSL', 's_vsTransformGLSL'):
        for fs in ('s_psNormalGLSL', 's_psDetailGLSL', 's_psGrassGLSL',
                   's_psWaterGLSL', 's_psFlatGLSL'):
            ctx.program(vertex_shader=current[vs], fragment_shader=current[fs]).release()
            results.append({'case': f'compile:{vs}/{fs}', 'result': 'PASS'})
    ctx.program(vertex_shader=current['s_vsShadowGLSL'],
                fragment_shader=current['s_psShadowGLSL']).release()
    results.append({'case': 'compile:shadow', 'result': 'PASS'})
    cases = (
        ('curved', CURVED, {}, (.7, .7, .7, 1)),
        ('planar', PLANAR, {}, (.7, .7, .7, 1)),
        ('ambient_only', CURVED, {'diffuse': 0}, (.1, .1, .1, 1)),
        ('sun_off', CURVED, {'sun': 0}, (0, 0, 0, 1)),
        ('prelit_screen', PLANAR, {'screen': True}, (.21, .42, .63, .8)),
    )
    for fs in ('s_psNormalGLSL', 's_psDetailGLSL', 's_psGrassGLSL'):
        for name, normals, options, expected in cases:
            actual = pixel(ctx, current, normals, fragment=fs, **options)
            if fs == 's_psDetailGLSL' and name == 'prelit_screen':
                expected = (.21, .42, .63, 1.8)  # existing detail adds vSpecColor.a
            if fs == 's_psGrassGLSL' and name == 'prelit_screen':
                expected = (.2, .4, .6, 1)  # grass has no specular term, own alpha
            close(actual, expected)
            row = {'case': f'{fs}:{name}', 'pixel': actual, 'result': 'PASS'}
            if baseline:
                before = pixel(ctx, baseline, normals, fragment=fs, **options)
                close(before, (.4, .4, .4, 1) if name == 'curved' else expected)
                row['baseline'] = before
            results.append(row)
    report = {'renderer': ctx.info['GL_RENDERER'], 'version': ctx.info['GL_VERSION'],
              'scope': 'Synthetic GLSL checks; not demo or full Remaster acceptance',
              'results': results}
    print(json.dumps(report, indent=2))
    if args.report:
        args.report.write_text(json.dumps(report, indent=2) + '\n')
    ctx.release()


if __name__ == '__main__':
    main()
