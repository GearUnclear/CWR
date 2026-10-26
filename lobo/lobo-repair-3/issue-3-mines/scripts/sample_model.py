"""Geometric sampling model for LoBo mine poll intervals (estimate, not a runtime test).
Detection probability for a straight crossing with random sample phase:
P = min(1, chord / (v * T_side)), averaged uniformly over crossing offsets that intersect the zone."""
import math
speeds = {'walk 1.5': 1.5, 'run 4.2': 4.2, 'max 6.67': 6.67}
def avg(chords, step):
    return sum(chords)/len(chords) if chords else 0
def ap(h, T, v):
    # query point 2 m above ground; mine centre ~0.05 m above ground -> trigger if closer than 1.95 m
    rmax = math.sqrt(max(0, 1.95**2 - (2-h)**2)); n=400; ps=[]
    for i in range(n):
        b = (i+0.5)/n*rmax; chord = 2*math.sqrt(rmax**2-b**2)
        ps.append(min(1, chord/(v*T)))
    return rmax, sum(ps)/n
def flare_halfwidth(h, x):
    # foci at (0,0,0) mine and (4,0,0) probe helper; man centre at height h; sum of 3D distances < 4.5
    lo, hi = 0.0, 3.0
    f = lambda y: math.sqrt(x*x+y*y+h*h)+math.sqrt((4-x)**2+y*y+h*h)
    if f(0) >= 4.5: return 0.0
    for _ in range(60):
        mid=(lo+hi)/2
        if f(mid) < 4.5: lo=mid
        else: hi=mid
    return lo
def flare(h, Tside, v):
    xs=[i*0.01 for i in range(0,401)]; ws=[2*flare_halfwidth(h,x) for x in xs]; inside=[w for w in ws if w>0]
    ps=[min(1, w/(v*Tside)) for w in inside]
    return (max(ws), len(inside)*0.01, sum(ps)/len(ps) if ps else 0)
for h in (0.8, 0.9, 1.0):
    print(f'--- man centre height h={h} m (estimate)')
    for label, T in (('60fps', 1/60), ('30fps', 1/30), ('~0.05', 0.05), ('~0.1', 0.1), ('~0.25', 0.25)):
        r, _ = ap(h, T, 1)
        row = [f'AP r={r:.2f}m {label:6}'] + [f'{k}:{ap(h,T,v)[1]:.2f}' for k,v in speeds.items()]
        print('  '.join(row))
    for label, Tpass in (('60fps', 1/60), ('30fps', 1/30), ('~0.05', 0.05), ('~0.1', 0.1), ('~0.25', 0.25)):
        Tside = 2*Tpass  # sides alternate every pass
        w, L, _ = flare(h, Tside, 1)
        row = [f'FLARE width<={w:.2f}m len={L:.2f}m {label:6}'] + [f'{k}:{flare(h,Tside,v)[2]:.2f}' for k,v in speeds.items()]
        print('  '.join(row))
# vehicle displacement during detection lag (AT mines: crush/damage triggered)
for kmh in (20, 40, 70, 100):
    v = kmh/3.6
    print(f'vehicle {kmh} km/h: lag displacement 60fps {v/60:.2f} m, ~0.1 {v*0.1:.2f} m, ~0.25 {v*0.25:.2f} m')
