import json, glob, re, os
O = r'D:/Arma_CWA/tmp/lobo-repair-3/config-identifiers'
names = set()
t = open(os.path.join(O, 'remaster-config.cpp'), errors='replace').read()
s = t.find('class CfgVehicles')
e = t.find('\nclass ', s + 10)
names |= set(m.group(1).lower() for m in re.finditer(r'\n\tclass (\w+)', t[s:e]))
crews = []
for f in glob.glob(os.path.join(O, 'json', '*.json')):
    d = json.load(open(f, encoding='utf-8', errors='replace'))
    cv = [v for k, v in d.items() if k.lower() == 'cfgvehicles']
    if not cv:
        continue
    for c, b in cv[0].items():
        names.add(c.lower())
        if isinstance(b, dict):
            for k, v in b.items():
                if k.lower() == 'crew' and isinstance(v, str):
                    crews.append((os.path.basename(f), c, v))
miss = [x for x in crews if x[2].lower() not in names]
for m in miss:
    print(m)
print(len(crews), 'crew entries;', len(miss), 'missing')
