import re, sys, collections, glob
keys = r'(color|ambient|lightColor|smokeColor|cloudletColor|colorDiffuse|colorAmbient)'
pat = re.compile(r'\b' + keys + r'\[\]\s*=\s*\{([^{}]*)\}', re.I)
for path in sys.argv[1:]:
    for f in glob.glob(path):
        try:
            txt = open(f, encoding='latin-1').read()
        except Exception as e:
            print(f, e); continue
        cnt = collections.Counter()
        odd = []
        # track class stack roughly
        for m in pat.finditer(txt):
            n = len([x for x in m.group(2).split(',') if x.strip() != ''])
            cnt[(m.group(1), n)] += 1
            if n != 4:
                # find enclosing class names
                pre = txt[:m.start()]
                cls = re.findall(r'class\s+(\w+)', pre[-3000:])
                odd.append((m.group(0).strip(), cls[-3:] if cls else []))
        print('==', f, dict(cnt))
        for o in odd[:40]:
            print('   ', o)
