"""Read-only: print member list summary and unique path-like strings containing a token from chosen pbo members.
Usage: python -I paths.py <pbo> <member-substring|*> <token> [<token>...]"""
import re, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from scan_lib import members, lzss

pbo, msub, toks = sys.argv[1], sys.argv[2].lower(), [t.lower().encode() for t in sys.argv[3:]]
data = open(pbo, 'rb').read()
for name, fl, off in members(data):
    if msub != '*' and msub not in name.lower(): continue
    raw = data[off:off + fl[4]]
    blob = lzss(raw, fl[1])[0] if fl[1] and fl[1] != fl[4] else raw
    found = {}
    for m in re.finditer(rb'[\x20-\x7e]{3,}', blob):
        s = m.group(0); sl = s.lower()
        if any(t in sl for t in toks):
            found[s] = found.get(s, 0) + 1
    if found:
        print('==', name, 'comp' if fl[1] and fl[1] != fl[4] else 'raw', len(found), 'unique strings')
        for s, c in sorted(found.items(), key=lambda x: -x[1])[:int(os.environ.get('TOPN', '40'))]:
            print('   %5d  %s' % (c, s.decode('latin1')[:200]))
