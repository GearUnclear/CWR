import glob,re,collections
for f in sorted(glob.glob('*.p3d')):
    b=open(f,'rb').read().lower()
    c=collections.Counter(m.group(0) for m in re.finditer(rb'lobo_pilot\[a-z0-9\_]*(back|legsbk)[a-z_]*\.pa[ac]', b))
    print(f, dict(c))
