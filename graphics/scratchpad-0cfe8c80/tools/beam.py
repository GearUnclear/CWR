import math
def ss(e0,e1,x):
    t=min(max((x-e0)/(e1-e0),0),1); return t*t*(3-2*t)
def vert_cur(av):
    hv=(av+0.03)/0.05
    return math.exp(-hv*hv)+0.02*ss(-0.6,-0.1,av)
def vert_new(av, wlow=0.03, floor=0.02, fstart=0.12, p=3.0):
    w = 0.05 if av>-0.03 else wlow
    hv=(av+0.03)/w
    f = floor*ss(-0.6*0+(-1.2), -0.1, av)  # not used
    a=abs(av)
    fl = floor*ss(0.03, 0.09, a)*min(1.0,(fstart/max(a,1e-6))**p)
    return math.exp(-hv*hv)+fl
def cut(av): return 1-ss(-0.005,0.025,av)
h=0.9; tilt=0.01
print(" x    theta  Ecur      Enew(0.03)  Enew(0.035) ratio")
for x in [2,3,4,5,6,7,8,9,10,12,14,16,18,20,22.5,25,30,40,50,60]:
    th=math.atan(h/x); av=-(th-tilt)
    d2=x*x+h*h
    s=math.sin(th)
    ec=800*vert_cur(av)*cut(av)*1.15/(d2+0.25)*s
    en=800*vert_new(av,0.03)*cut(av)*1.15/(d2+0.25)*s
    en2=800*vert_new(av,0.035)*cut(av)*1.15/(d2+0.25)*s
    print(f"{x:5.1f} {math.degrees(th):5.2f} {ec:8.4f} {en:8.4f} {en2:8.4f} {ec/max(en,1e-9):6.2f}")
