#!/usr/bin/env python3
"""Synthetic EGL/OpenGL UV sampling regression and driver diagnostic.

Nondegenerate triangle UVs must sample finite opaque white. Constant UV results
are observations, not portable failure expectations: one llvmpipe configuration
returned NaN with constant UV and 16x anisotropy. No engine/game assets required.
Requires moderngl and an EGL/OpenGL 3.3 implementation. Exit 1 means a regression
assertion failed; context/dependency failures are errors, not passing skips.
"""
import argparse
import datetime
import json
import math
from pathlib import Path
import struct

import moderngl

VERTEX = """#version 330 core
in vec2 position;
in vec2 coordinate;
out vec2 uv;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    uv = coordinate;
}
"""
FRAGMENT = """#version 330 core
uniform sampler2D tex0;
in vec2 uv;
out vec4 color;
void main() { color = texture(tex0, uv); }
"""
POSITIONS = ((-1., -1.), (3., -1.), (-1., 3.))
UV_CASES = {
    'triangle': ((0., 0.), (1., 0.), (.5, 1.)),
    'constant_zero': ((0., 0.),) * 3,
    'constant_half': ((.5, .5),) * 3,
}


def json_float(value):
    """Keep reports strict JSON even when a driver produces nonfinite values."""
    if math.isnan(value):
        return 'NaN'
    if math.isinf(value):
        return '+Infinity' if value > 0 else '-Infinity'
    return value


def run(repeats):
    ctx = moderngl.create_standalone_context(backend='egl', require=330)
    try:
        supported = bool({'GL_EXT_texture_filter_anisotropic',
                          'GL_ARB_texture_filter_anisotropic'} & ctx.extensions)
        maximum = float(ctx.max_anisotropy) if supported else 1.
        levels = [1.]
        if supported and maximum > 1.:
            levels.append(min(16., maximum))
        skipped = [] if len(levels) == 2 else [
            'Anisotropic (>1) cases skipped: extension unavailable or maximum <=1.']
        program = ctx.program(vertex_shader=VERTEX, fragment_shader=FRAGMENT)
        texture = ctx.texture((1, 1), 4, bytes([255] * 4), dtype='f1')
        # Complete mip chain for a 1x1 texture consists of level zero only.
        texture.use(0)
        program['tex0'].value = 0
        output = ctx.texture((16, 16), 4, dtype='f4')
        framebuffer = ctx.framebuffer([output])
        framebuffer.use()
        ctx.viewport = (0, 0, 16, 16)
        rows, failures = [], []
        checks = 0
        for level in levels:
            for name, coordinates in UV_CASES.items():
                vertices = [v for p, uv in zip(POSITIONS, coordinates) for v in (*p, *uv)]
                buffer = ctx.buffer(struct.pack('<12f', *vertices))
                vao = ctx.vertex_array(program, [(buffer, '2f 2f', 'position', 'coordinate')])
                # Omit unsupported anisotropy property rather than pretending to set it.
                settings = dict(filter=(moderngl.LINEAR_MIPMAP_LINEAR, moderngl.LINEAR),
                                repeat_x=True, repeat_y=True)
                if supported:
                    settings['anisotropy'] = level
                sampler = ctx.sampler(**settings)
                try:
                    sampler.use(0)
                    applied = float(sampler.anisotropy) if supported else None
                    if supported:
                        checks += 1
                        if not math.isfinite(applied) or abs(applied - level) > 1e-5:
                            failures.append(f'{name}: requested anisotropy {level}, got {applied}')
                    samples, whites = [], []
                    for repeat in range(repeats):
                        framebuffer.clear()
                        vao.render(mode=moderngl.TRIANGLES)
                        values = struct.unpack('<4f', framebuffer.read(
                            viewport=(8, 8, 1, 1), components=4, dtype='f4'))
                        white = all(math.isfinite(v) and abs(v - 1.) <= 1e-6 for v in values)
                        samples.append([json_float(v) for v in values])
                        whites.append(white)
                        if name == 'triangle':
                            checks += 1
                            if not white:
                                failures.append(f'triangle anisotropy={level}, repeat={repeat}: {samples[-1]}')
                    rows.append(dict(case=name,uv=coordinates,anisotropy_requested=level,
                                     anisotropy_reported=applied,rgba=samples,
                                     finite_white_every_repeat=all(whites),
                                     assertion_required=name == 'triangle'))
                finally:
                    sampler.release()
                    vao.release()
                    buffer.release()
        return dict(
            created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            status='fail' if failures else 'pass', checks=checks, failures=failures,
            skips=skipped, renderer={k: ctx.info.get(k) for k in
                                    ('GL_VENDOR', 'GL_RENDERER', 'GL_VERSION')},
            moderngl_version=moderngl.__version__, backend='EGL', required_gl=330,
            anisotropy_supported=supported, max_anisotropy=maximum,
            tested_anisotropy=levels, repeats=repeats,
            settings={'texture':'1x1 RGBA8 opaque white, level zero only',
                      'sampler':'LINEAR_MIPMAP_LINEAR / LINEAR, REPEAT S/T, unit 0',
                      'target':'16x16 RGBA32F, center pixel (8,8)',
                      'shaders':'GLSL 330 core, interpolated UV, texture()'},
            cases=rows,
            scope='Synthetic actual GL sampling, not extracted engine GLSL or full game validation. '
                  'Constant-UV outcomes are diagnostic only; NaN is not required on any driver. '
                  'No hardware performance or universal driver-cause claim.')
    finally:
        ctx.release()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, help='Optional strict JSON report path')
    parser.add_argument('--repeats', type=int, default=5)
    args = parser.parse_args()
    if args.repeats < 1:
        parser.error('--repeats must be positive')
    report = run(args.repeats)
    text = json.dumps(report, indent=2, allow_nan=False)
    if args.report:
        args.report.write_text(text + '\n')
    print(text)
    return 1 if report['failures'] else 0


if __name__ == '__main__':
    raise SystemExit(main())
