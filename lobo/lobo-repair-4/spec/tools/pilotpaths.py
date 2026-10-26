import json
import re
from pathlib import Path

here = Path(__file__).parent
idx = json.loads((here / 'lobo-index.json').read_text())
idx.update(json.loads((here / 'base-index.json').read_text()))
t = (here / 'pilot-config.txt').read_text(encoding='latin1')
BS = chr(92)
for m in sorted(set(re.findall(r'"(' + re.escape(BS) + r'LoBo_Pilot[^"]+)"', t))):
    k = m.replace(BS, '/').lower().lstrip('/')
    alt = k[:-4] + ('.paa' if k.endswith('.pac') else '.pac')
    state = 'ok     ' if k in idx else 'MISSING'
    extra = '' if k in idx else ('(alt exists: ' + alt + ')' if alt in idx else '(no alt)')
    print(state, m, extra, 'occurrences=%d' % t.count('"' + m + '"'))
