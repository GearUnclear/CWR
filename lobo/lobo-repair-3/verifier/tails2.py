import sys, hashlib
from pathlib import Path
V = Path(sys.argv[1]); ns = {}
exec((V / 'verify.py').read_text().split("report = {}")[0], ns)
ISSUE = Path('D:/Arma_CWA/@LoBo/.ud-backups/issue-69/20261012T034354Z-0532e837')
A = Path('D:/Arma_CWA/@LoBo/addons')
for f in ['BDE_mir3.pbo', 'LoboF5.pbo', 'bwm_boot3.pbo', 'CoC_Arty.pbo']:
    for label, raw in [('before', (ISSUE / f).read_bytes()), ('after', (A / f).read_bytes())]:
        ents, order, props, tail = ns['parse'](raw)
        print(f, label, 'len', len(raw), 'tail', tail.hex(), 'sha1(raw[:-20])', hashlib.sha1(raw[:-20]).hexdigest(), 'sha1(raw[:-21])', hashlib.sha1(raw[:-21]).hexdigest(), 'props', props)
