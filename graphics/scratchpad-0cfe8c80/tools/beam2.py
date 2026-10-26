import math
def ss(e0,e1,x):
    t=min(max((x-e0)/(e1-e0),0),1); return t*t*(3-2*t)
def vert_cur(av):
    hv=(av+0.03)/0.05
    return math.exp(-hv*hv)+0.02*ss(-0.6,-0.1,av)
def vert_new(av, wlow, floor=0.02, knee=0.12):
    w = 0.05 if av>-0.03 else wlow
    hv=(av+0.03)/w
    a=max(-av,1e-4)
    fl = floor*min(1.0,(knee/a)**3)
    return math.exp(-hv*hv)+fl
def cut(av): return 1-ss(-0.005,0.025,av)
def on_retro(th):
    s=math.cos(th)**2; t=max(math.sin(th),0.1)
    return (0.72+0.33*s/t)/0.72
h=0.9; tilt=0.01
print("  x   theta   cur    w.03   w.025  w.02  | retro: cur  w.02")
for x in [2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,18,20,22.5,25,30,40,50,60]:
    th=math.atan(h/x); av=-(th-tilt)
    d2=x*x+h*h
    s=math.sin(th)
    f=lambda v: 800*v*cut(av)*1.15/(d2+0.25)*s
    ec=f(vert_cur(av)); e3=f(vert_new(av,0.03)); e25=f(vert_new(av,0.025)); e2=f(vert_new(av,0.02))
    r=on_retro(th)
    print(f"{x:5.1f} {math.degrees(th):5.2f} {ec:6.3f} {e3:6.3f} {e25:6.3f} {e2:6.3f} | {ec*r:6.3f} {e2*r:6.3f}")
