import json,sys
idx=json.load(open('lobo-index.json'));b=json.load(open('base-index.json'))
pats=sys.argv[1:]
for name,src in (('lobo',idx),('base',b)):
    for k in sorted(src):
        if any(p in k for p in pats): print(name,k,[x.replace(chr(92),'/').split('/')[-1] for x in src[k]])
