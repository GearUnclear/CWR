"""Variants of the shader-side rig changes, evaluated with rigmodel's port.
Prints on-screen fractions F (after gamma 1.2) of faces and the display light
colour on sunlit ground and a 45-degree lit face, per variant and elevation."""
import math, sys
import rigmodel as R

def classic_hue_lin(e):
    s = math.sin(math.radians(max(e, 0)))
    t = R.clamp01(1 - s/0.42262)
    c = R.mix((0.85, 0.75, 0.4), (0.8, 0.30, 0.23), t)
    return R.norm(R.lin(c))

def variant_rig(e, V, p):
    r = R.rig(e)
    if V >= 2:
        L = R.lum(r['sun'])
        share = p.get('share', 0.65)*(1 - R.smooth(p.get('h0', 6), p.get('h1', 25), e))
        hue = R.norm(R.mix(R.norm(r['sun']), classic_hue_lin(e), share))
        r['sun'] = R.mul(hue, L)
    if V >= 3:
        low = 1 - R.smooth(p.get('l0', 8), p.get('l1', 30), e)
        ga = 1 + p.get('bounce', 0.5)*low
        gr = R.mul(r['ground'], ga)
        anti_sky = R.add(r['hanti'], R.mul(r['ground'], -0.5))
        sun_sky = R.add(r['hsun'], R.mul(r['ground'], -0.5))
        r['hanti'] = R.add(R.mul(anti_sky, 1 + p.get('anti', 1.0)*low), R.mul(gr, 0.5))
        r['hsun'] = R.add(R.mul(sun_sky, 1 + p.get('sunb', 0.3)*low), R.mul(gr, 0.5))
        r['ground'] = gr
    return r

def px(e, n, V, p, sunvis=1.0, albedo=(0.5, 0.5, 0.5), sat=1.12):
    r = variant_rig(e, V, p)
    s = r['s']; c = math.cos(math.radians(e))
    sundir = (0, s, c)
    nl = max(0, sum(a*b for a, b in zip(n, sundir)))
    direct = R.mul(r['sun'], R.K*nl*sunvis)
    amb = R.mul(R.sky_irr(r, n, sundir), R.K)
    a = R.lin(albedo)
    rad = R.mul(R.cmul(a, R.add(direct, amb)), r['exposure'])
    d = R.disp(R.tonemap(rad))
    grey = R.lum(d); d = tuple(max(grey + (x-grey)*sat, 0) for x in d)
    return tuple(x**(1/R.G) for x in d)

def light(pxv, albedo=(0.5, 0.5, 0.5)):
    return tuple((pxv[i]/albedo[i]**(1/R.G))**1.2 for i in range(3))

if __name__ == '__main__':
    variants = [(1, {}), (2, {}), (3, {}), (3, {'anti': 1.5, 'bounce': 0.8}), (3, {'anti': 2.0, 'bounce': 1.0, 'sunb': 0.5})]
    for e in (60, 30, 20, 12.7, 7):
        print(f'--- sun {e}')
        cl_g = R.classic_px(e, R.FACES['ground']); cl45 = R.classic_px(e, R.FACES['wall_45']); cl135 = R.classic_px(e, R.FACES['wall_135'])
        Lg = light(cl_g); L45 = light(cl45)
        print(f'  classic   ground F{R.frac(cl_g):.2f} 45 F{R.frac(cl45):.2f} 135 F{R.frac(cl135):.2f} 135/45 {R.frac(cl135)/R.frac(cl45):.2f}  ground L({Lg[0]:.2f},{Lg[1]:.2f},{Lg[2]:.2f}) g/r{Lg[1]/Lg[0]:.2f} r/b{Lg[0]/Lg[2]:.2f}  45 L g/r{L45[1]/L45[0]:.2f} r/b{L45[0]/L45[2]:.2f}')
        for V, p in variants:
            g = px(e, R.FACES['ground'], V, p); gs = px(e, R.FACES['ground'], V, p, sunvis=0)
            f45 = px(e, R.FACES['wall_45'], V, p); f135 = px(e, R.FACES['wall_135'], V, p)
            Lg = light(g); L45 = light(f45)
            print(f'  V{V} {str(p):40s} ground F{R.frac(g):.2f} shadow/lit {R.frac(gs)/R.frac(g):.2f} 45 F{R.frac(f45):.2f} 135 F{R.frac(f135):.2f} 135/45 {R.frac(f135)/R.frac(f45):.2f} 135/ground {R.frac(f135)/R.frac(g):.2f} ground g/r{Lg[1]/Lg[0]:.2f} r/b{Lg[0]/Lg[2]:.2f} 45 g/r{L45[1]/L45[0]:.2f} r/b{L45[0]/L45[2]:.2f}')
