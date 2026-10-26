"""Which test_remaster_lighting.cpp checks would the final rig (C++ shadeRig: hue
V20, fill V21) move? Uses rigmodel.py's port of ComputeLightRig, with the same
inputs as the unit test (fog 0.73/0.79/0.85, clear sky)."""
import math
import rigmodel as rm


def final_rig(e, classic=None):
    r = rm.rig(e, fog=(0.73, 0.79, 0.85))
    s = math.sin(math.radians(e))
    fill = (1 - rm.smooth(8, 40, e)) * rm.smooth(-2, 3, e)
    # hue: further share of classic's sunset hue
    if classic is None:
        classic, _ = rm.classic_sun(e)
    share = 0.4 * (1 - rm.smooth(6, 25, e)) + 0.3 * (1 - rm.smooth(4, 9, e))
    L = rm.lum(r['sun'])
    hue = rm.norm(rm.mix(rm.norm(r['sun']), rm.norm(rm.lin(classic)), share))
    sun = rm.mul(hue, L)
    # fill
    g0 = r['ground']
    g1 = rm.mul(g0, 1 + 1.3 * fill)
    low = 1 - rm.clamp01(s)
    skysun = rm.add(r['hsun'], rm.mul(g0, -0.5))
    skyanti = rm.add(r['hanti'], rm.mul(g0, -0.5))
    out = dict(r)
    out['sun'] = sun
    out['ground'] = g1
    out['hsun'] = rm.add(rm.mul(skysun, 1 + 0.5 * fill), rm.mul(g1, 0.5))
    out['hanti'] = rm.add(rm.mul(skyanti, 1 + 2.6 * fill), rm.mul(g1, 0.5))
    return out


def rb(c):
    return c[0] / max(c[2], 1e-6)


for name, f in (("shipped", lambda e: rm.rig(e, fog=(0.73, 0.79, 0.85))), ("final", final_rig)):
    low = f(7.0)
    noon = f(60.0)
    sunwall = rm.lum(low['sun']) * math.cos(math.radians(7)) + rm.lum(low['hsun'])
    print(f"{name}: 7 deg sunWall / 4*skyHorizon = {sunwall:.4f} / {4 * rm.lum(low['hanti']):.4f} -> "
          f"{'PASS' if sunwall > 4 * rm.lum(low['hanti']) else 'FAIL'} (ratio {sunwall / rm.lum(low['hanti']):.2f}); "
          f"r/b sun 7 deg {rb(low['sun']):.2f} (>1.6), noon {rb(noon['sun']):.2f} (<1.1); "
          f"r/b hsun {rb(low['hsun']):.2f} > hanti {rb(low['hanti']):.2f}")
