import math,re,sys
import numpy as np
sys.path.insert(0,r'D:\Arma_CWA_worktrees\lighting\arma_CWR\tests\rendering\captures')
from sunplan import sun
from hm import heights
from parse import load
TREE=re.compile(r'^data3d.(str|les|jablon|oliva|sakura|smrk|borovice|dub|krovi)')
class Isl:
    def __init__(s,w):
        s.H=heights(w); s.n=s.H.shape[0]; s.objs=load(w)
        s.trees=np.array([(o[1],o[3]) for o in s.objs if TREE.search(o[0])])
        s.all=np.array([(o[1],o[3]) for o in s.objs])
    def h(s,x,z):
        # grid vertex i at x=i*50; image row = n-1-j
        fx=x/50; fz=z/50; i=int(math.floor(fx)); j=int(math.floor(fz)); ax=fx-i; az=fz-j
        def g(i,j):
            i=max(0,min(s.n-1,i)); j=max(0,min(s.n-1,j)); return s.H[s.n-1-j,i]
        return (g(i,j)*(1-ax)*(1-az)+g(i+1,j)*ax*(1-az)+g(i,j+1)*(1-ax)*az+g(i+1,j+1)*ax*az)
    def horizon(s,x,z,az,eye=1.7,dmin=200,dmax=5000):
        hc=s.h(x,z)+eye; best=-90; bd=0
        u=(math.sin(math.radians(az)),math.cos(math.radians(az)))
        for d in range(dmin,dmax,25):
            a=math.degrees(math.atan2(s.h(x+u[0]*d,z+u[1]*d)-hc,d))
            if a>best: best=a; bd=d
        return best,bd
    def count(s,arr,x,z,r):
        if len(arr)==0: return 0
        return int(np.sum((arr[:,0]-x)**2+(arr[:,1]-z)**2<r*r))
    def cone(s,arr,x,z,az,half,r0,r1):
        d=arr-np.array([x,z]); dist=np.hypot(d[:,0],d[:,1]); ang=(np.degrees(np.arctan2(d[:,0],d[:,1]))-az+540)%360-180
        return int(np.sum((dist>r0)&(dist<r1)&(abs(ang)<half)))
    def slope(s,x,z):
        return math.degrees(math.atan(math.hypot(s.h(x+10,z)-s.h(x-10,z),s.h(x,z+10)-s.h(x,z-10))/20))
