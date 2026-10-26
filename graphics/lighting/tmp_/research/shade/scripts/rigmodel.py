"""Python port of LightRig.cpp ComputeLightRig + the shader's shadeRemastered
(sun + sky hemisphere, no shadows/AO) and of the classic per-vertex lighting,
to predict on-screen fractions of the texture brightness (after gamma 1.2).

Used to reason about the low-sun calibration before spending game runs."""
import math

G = 1.2  # display gamma pass
K = 1.6  # default brightness folded into the materials

def lum(c): return 0.2126*c[0] + 0.7152*c[1] + 0.0722*c[2]
def mul(c, s): return tuple(x*s for x in c)
def add(a, b): return tuple(x+y for x, y in zip(a, b))
def cmul(a, b): return tuple(x*y for x, y in zip(a, b))
def mix(a, b, t): return tuple(x + (y-x)*t for x, y in zip(a, b))
def norm(c):
    l = lum(c); return mul(c, 1/l) if l > 1e-6 else (1, 1, 1)
def clamp01(v): return min(1, max(0, v))
def smooth(e0, e1, x):
    t = clamp01((x-e0)/(e1-e0)); return t*t*(3-2*t)
def lin(c): return tuple(max(x, 0)**2.2 for x in c)
def disp(c): return tuple(max(x, 0)**(1/2.2) for x in c)

def airmass(e):
    e = max(e, 0); return 1/(math.sin(math.radians(e)) + 0.50572*(e+6.07995)**-1.6364)

def sky_day_fraction(s):
    day = min(max(s/0.866, 0), 1.15)**0.6
    return max(day, 0.12*smooth(-0.10, 0.03, s))

KEYS = [(0, (1, .40, .17)), (3, (1, .53, .28)), (8, (1, .71, .46)), (15, (1, .86, .70)), (30, (1, .96, .90)), (60, (1, .985, .96))]
def sun_chroma(e, warmth=1.0):
    e = max(e, 0); c = KEYS[-1][1]
    for i in range(5):
        if e <= KEYS[i+1][0]:
            t = (e-KEYS[i][0])/(KEYS[i+1][0]-KEYS[i][0]); c = mix(KEYS[i][1], KEYS[i+1][1], clamp01(t)); break
    c = mix((1, .985, .96), c, max(0, warmth))
    return norm(c)

def classic_sun(e):
    s = math.sin(math.radians(e))
    full = (0.85, 0.75, 0.4); sunset = (0.8, 0.30, 0.23)
    s25 = math.sin(math.radians(25))
    if s > s25: col = full
    else: col = mix(full, sunset, 1 - s/s25)
    amb = mul((0.55, 0.6, 0.8), min(max(1.5*s, 0.05), 1))
    return col, amb

def evening_chroma(e, classic, warmth=1.0, share=0.65):
    phys = sun_chroma(e, warmth)
    cl = lin(classic)
    w = (1 - smooth(5, 16, e))*clamp01(warmth)*share
    return norm(mix(phys, norm(cl), w))

def rig(e, fog=(0.75, 0.72, 0.78), tun=None):
    t = dict(sun=1, sky=1, bounce=1, warmth=1, exposure=1)
    if tun: t.update(tun)
    s = math.sin(math.radians(e))
    ext = math.exp(-0.05*(airmass(e)-1))
    clear_sun = 0.55*ext*smooth(-1.5, 1.0, e)
    csun, camb = classic_sun(e)
    sun = mul(evening_chroma(e, csun, t['warmth']), clear_sun*t['sun'])
    sky = max(0.15*sky_day_fraction(s), 0.0009)*t['sky']
    flat = clear_sun*max(s, 0) + 0.15*sky_day_fraction(s)
    meter = min(max(((1/K)/flat)**0.85, 1), 1.6) if flat > 1e-5 else 1
    meter = 1 + (meter-1)*smooth(0.08, 0.40, s)
    exposure = meter*t['exposure']
    fl = lin(fog)
    cap = lum(fl)/(K*exposure); tw = 1 - smooth(0.05, 0.25, s)
    if cap < sky: sky = max(sky + (cap-sky)*tw, 0.0009*t['sky'])
    zen = norm((0.6, 0.69, 0.88))
    sunward = norm(mix(norm(fl), (1, 1, 1), 0.35))
    r = {}
    r['sun'] = sun; r['exposure'] = exposure; r['sky'] = sky
    r['zenith'] = mul(zen, sky)
    bounce = 0.2*(lum(sun)*max(s, 0) + sky)*t['bounce']
    r['ground'] = mul(norm((1, 1, .96)), bounce)
    low = 1 - clamp01(s)
    r['hsun'] = add(mul(sunward, 0.55*sky*(1+0.4*low)), mul(r['ground'], 0.5))
    r['hanti'] = add(mul(zen, 0.55*sky*(1-0.25*low)), mul(r['ground'], 0.5))
    r['s'] = s
    return r

def sky_irr(r, n, sundir):
    h = math.hypot(n[0], n[2]); sl = math.hypot(sundir[0], sundir[2])
    toward = 0.5 + 0.5*(n[0]*sundir[0] + n[2]*sundir[2])/(h*sl) if h > 1e-3 and sl > 1e-3 else 0.5
    horizon = mix(r['hanti'], r['hsun'], toward)
    t = max(-1, min(1, n[1]))
    return mix(horizon, r['zenith'], t) if t >= 0 else mix(horizon, r['ground'], -t)

def tonemap(c):
    peak = max(c)
    if peak < 0.8: return c
    d = 0.2; newp = 1 - d*d/(peak + d - 0.8)
    c = mul(c, newp/peak)
    g = 1 - 1/(0.15*(peak-newp) + 1)
    return mix(c, (newp,)*3, g)

def remaster_px(e, n, sunvis=1.0, tun=None, sat=1.12, albedo=(0.5, 0.5, 0.5)):
    """On-screen value (post gamma) of a surface with display albedo `albedo`."""
    r = rig(e, tun=tun)
    s = r['s']; c = math.cos(math.radians(e))
    sundir = (0, s, c)  # sun toward +z
    nl = max(0, sum(a*b for a, b in zip(n, sundir)))
    direct = mul(r['sun'], K*nl*sunvis)
    amb = mul(sky_irr(r, n, sundir), K)
    a = lin(albedo)
    rad = mul(cmul(a, add(direct, amb)), r['exposure'])
    d = disp(tonemap(rad))
    grey = lum(d); d = tuple(max(grey + (x-grey)*sat, 0) for x in d)
    return tuple(x**(1/G) for x in d)

def classic_px(e, n, albedo=(0.5, 0.5, 0.5), shadow=1.0):
    col, amb = classic_sun(e)
    s = math.sin(math.radians(e)); c = math.cos(math.radians(e))
    nl = max(0, n[1]*s + n[2]*c)
    litc = tuple(min(1, K*(a + d*nl)) for a, d in zip(amb, col))
    p = cmul(albedo, litc)
    p = mul(p, shadow)
    return tuple(x**(1/G) for x in p)

def frac(px, albedo=(0.5, 0.5, 0.5)):
    return lum(px)/lum(tuple(x**(1/G) for x in albedo))

FACES = {
    'ground': (0, 1, 0),
    'wall_sun': (0, 0, 1),
    'wall_45': (math.sin(math.radians(45)), 0, math.cos(math.radians(45))),
    'wall_135': (math.sin(math.radians(135)), 0, math.cos(math.radians(135))),
    'wall_anti': (0, 0, -1),
}

if __name__ == '__main__':
    import sys
    tun = {}
    for a in sys.argv[1:]:
        k, v = a.split('='); tun[k] = float(v)
    for e in (60, 30, 20, 14, 12.7, 10, 7, 5):
        r = rig(e, tun=tun)
        row = [f'{e:5.1f} exp {r["exposure"]:.2f}']
        for name, n in FACES.items():
            cp = classic_px(e, n); rp = remaster_px(e, n, tun=tun)
            row.append(f'{name} C{frac(cp):.2f} R{frac(rp):.2f}')
        rs = remaster_px(e, FACES['ground'], sunvis=0, tun=tun)
        row.append(f'gshadow R{frac(rs):.2f}')
        print(' | '.join(row))
