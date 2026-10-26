import sys, re
from pathlib import Path
V = Path(sys.argv[1])
ns = {}
exec((V / 'verify.py').read_text().split("report = {}")[0], ns)
parse = ns['parse']
A = Path('D:/Arma_CWA/@LoBo/addons')
for pbo, scripts in [('LoBoIL28.pbo', ['script/number.sqs']), ('LoBo_F4.pbo', ['scripts/number.sqs']),
                     ('LoBoF15.pbo', ['scr/number.sqs']), ('LoBoF16.pbo', ['scr/number.sqs', 'scr/number2.sqs'])]:
    ents = parse((A / pbo).read_bytes())[0]
    cfg = ents['config.cpp']['data'].decode('latin1')
    hs = set(re.findall(r'hiddenSelections\[\]\s*=\s*\{([^}]*)\}', cfg, re.I))
    print('=====', pbo, 'config compressed' if ents['config.cpp']['orig'] else '')
    for h in hs:
        print('  hiddenSelections:', h.strip())
    for s in scripts:
        t = ents[s]['data'].decode('latin1')
        idx = sorted(set(re.findall(r'setObjectTexture \[(\d+), "([^"]+)"', t)), key=lambda x: (int(x[0]), x[1]))
        slots = {}
        for i, p in idx:
            slots.setdefault(i, []).append(p)
        for i, ps in slots.items():
            missing = [p for p in ps if p.split('\\', 2)[2].lower().replace('\\', '/') not in ents]
            print('  ', s, 'slot', i, len(ps), 'textures; missing in pbo:', missing)
        labels = re.findall(r'^#(\w+)', t, re.M)
        gotos = set(re.findall(r'goto "(\w+)"', t))
        print('   labels', labels, 'gotos', sorted(gotos), 'unresolved', sorted(g for g in gotos if g.lower() not in {l.lower() for l in labels}))
        print('   uses _unit:', '_unit' in t, 'first line:', t.splitlines()[0])
