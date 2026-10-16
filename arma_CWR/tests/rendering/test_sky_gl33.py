#!/usr/bin/env python3
"""Render actual N2 sky GLSL extracted unchanged from the selected C++ source.

Requires numpy, moderngl and EGL/OpenGL 3.3. This tests shader behavior, not C++
activation/fallback gates, GL state restoration, visual acceptance or performance.
In particular clearSky=0 tests shader horizon-color identity, not engine weather
fallback (which may skip this shader entirely).
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import re

import moderngl
import numpy as np

SOURCE = Path(__file__).resolve().parents[2] / 'engine/PoseidonGL33/EngineGL33_ExperimentalSky.cpp'
BACKGROUND = np.array([.13, .17, .19, .37])
HORIZON = np.array([.55, .6, .7])


def extract(source, name):
    pattern = r'const\s+char\s*\*\s*' + re.escape(name) + r'\s*=\s*R"\((.*?)\)";'
    matches = re.findall(pattern, source, re.S)
    if len(matches) != 1:
        raise ValueError(f'Expected exactly one {name} raw GLSL literal, found {len(matches)}')
    return matches[0]


class SkyFixture:
    def __init__(self, source):
        self.context = moderngl.create_standalone_context(backend='egl', require=330)
        self.program = self.context.program(vertex_shader=extract(source, 'skyVS'),
                                            fragment_shader=extract(source, 'skyFS'))
        self.vao = self.context.vertex_array(self.program, [])
        self.texture = self.context.texture((32, 32), 4, dtype='f4')
        self.framebuffer = self.context.framebuffer([self.texture])
        self.framebuffer.use()
        self.context.viewport = (0, 0, 32, 32)
        self.checks = []
        for name, value in dict(rightRay=(0, 0, 0), upRay=(0, 0, 0),
                                sunRay=(0, .8, .6), horizonColor=tuple(HORIZON),
                                skyColor=(.3, .5, .8), sunColor=(.4, .3, .15),
                                clearSky=1).items():
            self.set(name, value)

    def set(self, name, value):
        self.program[name].value = value

    def draw(self):
        self.framebuffer.clear(*BACKGROUND)
        self.vao.render(vertices=3)
        return np.frombuffer(self.texture.read(), np.float32).reshape(32, 32, 4)

    def render(self, ray, clear=1):
        self.set('forwardRay', ray)
        self.set('clearSky', clear)
        return self.draw()

    def check(self, condition, label):
        self.checks.append(dict(check=label, passed=bool(condition)))

    def close(self):
        for resource in (self.framebuffer, self.texture, self.vao, self.program, self.context):
            resource.release()


def elevation_ray(angle):
    radians = math.radians(angle)
    return (0, math.sin(radians), math.cos(radians))


def run_checks(f):
    for angle in [-45, -1, 0, 1, 4.9, 5, 5.1, 20, 45, 89]:
        a = f.render(elevation_ray(angle))
        f.check(np.isfinite(a).all(), f'elevation {angle}: finite')
        if angle < 0:
            f.check(np.max(abs(a - BACKGROUND)) < 1e-6, f'elevation {angle}: discard lower hemisphere')
        elif angle == 0:
            f.check(np.max(abs(a[:, :, :3] - HORIZON)) < 2e-6, 'horizon identity')
        else:
            f.check(np.min(a) >= 0 and np.max(a) <= 1, f'elevation {angle}: bounded')
        a = f.render(elevation_ray(angle), 0)
        if angle >= 0:
            f.check(np.max(abs(a[:, :, :3] - HORIZON)) < 2e-6,
                    f'elevation {angle}: shader clearSky=0 identity, not engine fallback')

    # Perspective ray interpolation and offset viewport registration.
    for name, value in dict(rightRay=(.8, 0, 0), upRay=(0, .5, 0),
                            forwardRay=(0, 0, 1), clearSky=1).items():
        f.set(name, value)
    f.context.viewport = (8, 8, 16, 16)
    a = f.draw()
    f.check(np.max(abs(a[:8] - BACKGROUND)) < 1e-6, 'viewport: bottom untouched')
    f.check(np.max(abs(a[:, :8] - BACKGROUND)) < 1e-6, 'viewport: left untouched')
    f.check(np.max(abs(a[8:16, 8:24] - BACKGROUND)) < 1e-6, 'viewport: lower rays discarded')
    f.check(np.any(abs(a[16:24, 8:24, :3] - BACKGROUND[:3]) > .01), 'viewport: upper rays drawn')

    f.context.viewport = (0, 0, 32, 32)
    for pitch in [-.3, 0, .3]:
        forward = (0, math.sin(pitch), math.cos(pitch))
        up = np.array([0, math.cos(pitch), -math.sin(pitch)]) * .5
        masks = []
        for roll in [1, -1]:
            f.set('forwardRay', forward)
            f.set('rightRay', (roll * .8, 0, 0))
            f.set('upRay', tuple(roll * up))
            pixels = f.draw()
            f.check(np.isfinite(pixels).all(), f'pitch {pitch}, roll {roll}: finite')
            masks.append(pixels[:, :, 3] > .9)
        f.check(np.array_equal(masks[0], masks[1][::-1, ::-1]), f'pitch {pitch}: roll reverses hemisphere mask')

    f.set('rightRay', (0, 0, 0))
    f.set('upRay', (0, 0, 0))
    for scale in [.5, 1, 1.5]:
        f.set('skyColor', tuple(scale * np.array([.3, .5, .8])))
        f.set('sunColor', tuple(scale * np.array([.4, .3, .15])))
        pixels = f.render((0, 0, 1))
        f.check(np.max(abs(pixels[:, :, :3] - HORIZON)) < 2e-6,
                f'authored color scale {scale}: already-accommodated horizon unchanged')

    f.set('skyColor', (1.56, 1.56, 1.6))
    f.set('sunColor', (1.36, 1.2, .64))
    mission_horizon = np.array([.5960784, .6361055, .6988506])
    f.set('horizonColor', tuple(mission_horizon))
    rows = []
    for angle in [0, .001, .01, .1, 1, 2, 5, 10, 30, 60, 89]:
        pixel = f.render(elevation_ray(angle), .95862067)[0, 0, :3]
        f.check(np.isfinite(pixel).all(), f'mission elevation {angle}: finite')
        f.check(np.max(pixel) < .99, f'mission elevation {angle}: no broad white clipping')
        if angle in (0, .001):
            f.check(np.max(abs(pixel - mission_horizon)) < 2e-6, f'mission elevation {angle}: horizon continuity')
        rows.append(dict(elevation=angle, rgb=pixel.tolist()))
    f.check(rows[-1]['rgb'][2] > rows[-1]['rgb'][0] * 1.5, 'mission: blue zenith')
    f.check(sum(rows[5]['rgb']) > sum(rows[0]['rgb']), 'mission: luminous haze above horizon')
    for value in [0, 1e-9, .01, 1, 10, 1e4]:
        f.set('skyColor', (value,) * 3)
        f.set('sunColor', (0, 0, 0))
        for y in [0, .035, .5, 1]:
            pixels = f.render((0, y, 1))
            f.check(np.isfinite(pixels).all(), f'input {value}, ray y={y}: finite')
            f.check(np.min(pixels) >= 0 and np.max(pixels) <= 1, f'input {value}, ray y={y}: bounded')
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=SOURCE)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    source_bytes = args.source.read_bytes()
    fixture = SkyFixture(source_bytes.decode())
    try:
        rows = run_checks(fixture)
        failures = [c for c in fixture.checks if not c['passed']]
        report = dict(checks=len(fixture.checks), result='FAIL' if failures else 'PASS',
                      failures=failures, source=str(args.source),
                      source_sha256=hashlib.sha256(source_bytes).hexdigest(),
                      renderer={k: fixture.context.info.get(k) for k in
                                ('GL_VENDOR', 'GL_RENDERER', 'GL_VERSION')},
                      moderngl_version=moderngl.__version__, backend='EGL',
                      mission_samples=rows, assertions=fixture.checks, scope=__doc__)
        text = json.dumps(report, indent=2, allow_nan=False)
        if args.report:
            args.report.write_text(text + '\n')
        print(text)
        return 1 if failures else 0
    finally:
        fixture.close()


if __name__ == '__main__':
    raise SystemExit(main())
