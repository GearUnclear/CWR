import sys,math,collections
from parse import load
w=sys.argv[1]; x=float(sys.argv[2]); z=float(sys.argv[3]); r=float(sys.argv[4])
rows=[o for o in load(w) if math.hypot(o[1]-x,o[3]-z)<=r]
c=collections.Counter(o[0] for o in rows)
print(len(rows),'objects'); 
for k,v in c.most_common(25): print(' ',v,k)
ys=[o[2] for o in rows if 'silnice' not in o[0] and 'cesta' not in o[0]]
if ys: print('Y min/max',min(ys),max(ys))
# nearest 10
for o in sorted(rows,key=lambda o:math.hypot(o[1]-x,o[3]-z))[:int(sys.argv[5]) if len(sys.argv)>5 else 8]: print('  near',o, round(math.hypot(o[1]-x,o[3]-z),1))
