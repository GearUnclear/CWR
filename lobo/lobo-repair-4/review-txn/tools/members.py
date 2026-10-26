import hashlib, json, sys
from pathlib import Path
sys.path.insert(0, 'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
TOOLS = Path('D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
B = Path('D:/Arma_CWA/@LoBo/.ud-backups/issue-69/20261012T085301Z-fdbc36a3')
A = Path('D:/Arma_CWA/@LoBo/addons')
m = json.loads((B/'manifest.json').read_text(encoding='utf8'))
sha = lambda b: hashlib.sha256(b).hexdigest()
spec = {
 'loboil28/script/tailgun.sqs': ('9b92d408', '0151', 'a2aed3ca23bb70b997e3bfdadb02e4df2547803426e110057a93304dd7bb98a0'),
 'lobomig/config.cpp': ('599760d9', '4eb7', '1ca20ca72027a21fccc47c569184611f8e401781f39a93dd30edff0e7ab24664'),
 'lobo_pilot/config.cpp': ('db7da9fb', '6e28', '47ac0111b99b0f9d088a4fa56b7426293d75cbed8433f925f4422196069f80d6'),
 'loboter/lobo_terhilux.p3d': ('731a5d77540ad069678e13d86275c78f3dc52ff62c045c13818bcdd800dc64e0', '', 'b96ec642ae6040067d1893bcad6e60783c50698eca9c3a9957f8e17a23e6607d'),
 'loboter/lobo_terhiluxmg.p3d': ('6cb798ee256741971cef9b84fbaf672f0b56eeee2826f1b4f4f3f563faf423b2', '', '09ec6792486e02f733579752672b3a61303e8c1d1223fe975917ee4334bab116'),
 'lobo_pilot/lobo_epilot.p3d': ('bdb8ebbf99c7f1d16b3df6c63f32642a5ca0ef39d3d30b29bd101c9f546316e5', '', '18422e9da1a384b126be0574a8b13ad58a63efc60e5e658c465344cfaed94f71'),
 'lobo_airammo/lobo_aim9.p3d': ('0b294d9e18111eb549a138cdd313b8c60de20121433bf24648a25491e3e665e9', '', '4e7f2e733e882174ebfbbe4b2700e0147c9a79461d84f5a7587d7773f43ee6b5'),
}
recs = {r['member']: r for r in m['members']}
assert set(recs) == set(spec), set(recs) ^ set(spec)
out = {}
for a in m['archives']:
    bp, ap = B/a['file'], A/a['file']
    braw, araw = bp.read_bytes(), ap.read_bytes()
    assert sha(braw) == a['before_sha256'] and sha(araw) == a['after_sha256'], a['file']
    ba, aa = Archive(braw), Archive(araw)
    stem = a['file'][:-4].lower()
    assert ba.prefix == aa.prefix and ba.terminator == aa.terminator and ba.trailer.__len__() == aa.trailer.__len__()
    assert list(ba.by_key) == list(aa.by_key)
    same = diff = 0
    for e in ba.entries:
        full = stem + '/' + e.key; f = aa.by_key[e.key]
        if full in recs:
            bdata = ba.read(e.key, bp, TOOLS); adata = aa.read(e.key, ap, TOOLS)
            r = recs[full]; s = spec[full]
            ok = (sha(bdata) == r['before_sha256'] and sha(adata) == r['after_sha256'] and len(bdata) == r['before_bytes']
                  and len(adata) == r['after_bytes'] and sha(bdata).startswith(s[0]) and sha(bdata).endswith(s[1]) and sha(adata) == s[2]
                  and f.fields[0] == 0)
            nd = sum(1 for x, y in zip(bdata, adata) if x != y) if len(bdata) == len(adata) else None
            out[full] = dict(ok=ok, before_compressed=e.fields[0] != 0, before_fields=e.fields, after_fields=f.fields, bytes_differ=nd)
        else:
            if e.fields == f.fields and bytes(ba.stored(e)) == bytes(aa.stored(f)): same += 1
            else: diff += 1
    out[a['file']] = dict(untouched_same=same, untouched_diff=diff, checksummed=ba.checksummed, trailer=ba.trailer.hex()[:60])
print(json.dumps(out, indent=1, default=str))
