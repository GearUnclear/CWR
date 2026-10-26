import re,sys
cur=None; rows={}
for line in open(sys.argv[1],encoding='utf-8',errors='replace'):
    if line.startswith('== '): cur=line.split()[1]; continue
    if cur is None or not line.startswith('   '): continue
    label,_,ctx=line.strip().partition(' :: ')
    for m in re.finditer(r'[A-Za-z0-9_\/.\-]*(?:bas_lbs|ah6j|mh6j|mh-6|ah6|mh6|littlebird)[A-Za-z0-9_\/.\-]*',ctx,re.I):
        rows.setdefault((label,m.group(0).lower()),0); rows[(label,m.group(0).lower())]+=1
for (l,t),n in sorted(rows.items()): print(n,l,t)
