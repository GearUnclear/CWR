"""Independent read-only verification of LoBo repair batches (no writes outside scratchpad)."""
import hashlib, json, struct, subprocess, sys, os
from pathlib import Path

MOD = Path('D:/Arma_CWA/@LoBo')
ADDONS = MOD / 'addons'
ISSUE = MOD / '.ud-backups' / 'issue-69'
OUT = Path(sys.argv[1])
EVID = Path('D:/Arma_CWA/tmp/lobo-repair-4/impl')

def sha(p):
    h = hashlib.sha256()
    with open(p, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 22), b''):
            h.update(chunk)
    return h.hexdigest()

def parse(raw):
    pos = 0; entries = []; props = []
    first = True
    while True:
        end = raw.index(b'\0', pos); name = raw[pos:end]; pos = end + 1
        mime, orig, res, ts, size = struct.unpack_from('<5I', raw, pos); pos += 20
        if name == b'' and first and mime == 0x56657273:
            while True:
                e = raw.index(b'\0', pos); k = raw[pos:e]; pos = e + 1
                if k == b'': break
                e = raw.index(b'\0', pos); v = raw[pos:e]; pos = e + 1
                props.append((k, v))
            first = False
            continue
        first = False
        if name == b'':
            break
        entries.append([name, mime, orig, res, ts, size])
    off = pos; out = {}
    order = []
    for name, mime, orig, res, ts, size in entries:
        key = name.decode('latin1').replace('\\', '/').lower()
        out[key] = dict(name=name, mime=mime, orig=orig, res=res, ts=ts, size=size, data=raw[off:off + size])
        order.append(key)
        off += size
    return out, order, props, raw[off:]

report = {}
# 1. installed hashes
installed = {p.name: sha(p) for p in sorted(ADDONS.glob('*.pbo'))}
report['installed_count'] = len(installed)

# 2. pristine LFS oids from @LoBo git HEAD (read-only)
env = dict(os.environ, GIT_OPTIONAL_LOCKS='0')
ls = subprocess.run(['git', '-C', str(MOD), '--no-optional-locks', 'ls-tree', 'HEAD', 'addons/'], capture_output=True, text=True, env=env, check=True).stdout
blobs = {}
for line in ls.splitlines():
    meta, path = line.split('\t', 1)
    if path.lower().endswith('.pbo'):
        blobs[path.split('/', 1)[1]] = meta.split()[2]
inp = ''.join(b + '\n' for b in blobs.values())
cat = subprocess.run(['git', '-C', str(MOD), '--no-optional-locks', 'cat-file', '--batch'], input=inp.encode(), capture_output=True, env=env, check=True).stdout
pristine = {}
pos = 0
names = list(blobs)
for n in names:
    hdr_end = cat.index(b'\n', pos); hdr = cat[pos:hdr_end].split(); size = int(hdr[2])
    body = cat[hdr_end + 1:hdr_end + 1 + size]; pos = hdr_end + 1 + size + 1
    txt = body.decode('latin1')
    if txt.startswith('version https://git-lfs'):
        oid = [l for l in txt.splitlines() if l.startswith('oid sha256:')][0].split(':', 1)[1]
        pristine[n] = oid
    else:
        pristine[n] = hashlib.sha256(body).hexdigest()
report['pristine_count'] = len(pristine)
report['installed_not_in_git'] = sorted(set(installed) - set(pristine))
report['git_not_installed'] = sorted(set(pristine) - set(installed))

# 3. manifests
batches = {}
for d in sorted(ISSUE.iterdir()):
    m = json.loads((d / 'manifest.json').read_text(encoding='utf8'))
    files = sorted(p.name for p in d.iterdir() if p.name != 'manifest.json')
    bk = {}
    for a in m['archives']:
        bp = d / a['file']
        bk[a['file']] = dict(backup_sha=sha(bp) if bp.exists() else None, before=a['before_sha256'], after=a['after_sha256'])
    batches[d.name] = dict(state=m['state'], version=m.get('repair_version'), rule=m.get('rule_sha256'), compat=m.get('compatibility'),
                           n_archives=len(m['archives']), n_members=len(m['members']),
                           n_added=sum(1 for r in m['members'] if r.get('added')),
                           extra_files=sorted(set(files) - {a['file'] for a in m['archives']}),
                           missing_files=sorted({a['file'] for a in m['archives']} - set(files)),
                           backups_match_before=all(v['backup_sha'] == v['before'] for v in bk.values()),
                           mismatches=[k for k, v in bk.items() if v['backup_sha'] != v['before']],
                           attribute_warnings=m.get('attribute_warnings'), rollback_conflicts=m.get('rollback_conflicts'),
                           rollback_errors=m.get('rollback_errors'), archives=bk, members=m['members'])
report['batches'] = {k: {kk: vv for kk, vv in v.items() if kk not in ('archives', 'members')} for k, v in batches.items()}

# 4. chain per archive over applied batches (chronological)
applied = [k for k in sorted(batches) if batches[k]['state'] == 'applied']
report['applied_batches'] = applied
chain = {}
for b in applied:
    for f, v in batches[b]['archives'].items():
        chain.setdefault(f, []).append((b, v['before'], v['after']))
doctor_orig = {p.name: sha(p) for p in (MOD / '_ud-orig').glob('*.pbo')}
report['doctor_orig'] = {k: dict(sha=v, pristine=pristine.get(k), equal=(v == pristine.get(k))) for k, v in doctor_orig.items()}
chain_issues = []
latest_ok = []
for f, steps in chain.items():
    first_before = steps[0][1]
    if first_before != pristine.get(f):
        chain_issues.append(f'{f}: first batch {steps[0][0]} before != pristine LFS oid ({first_before[:16]} vs {str(pristine.get(f))[:16]})')
    for (b1, _, a1), (b2, bf2, _) in zip(steps, steps[1:]):
        if a1 != bf2:
            chain_issues.append(f'{f}: {b1}.after != {b2}.before')
    if installed.get(f) != steps[-1][2]:
        chain_issues.append(f'{f}: installed != latest batch {steps[-1][0]} after')
    else:
        latest_ok.append(f)
report['chain_issues'] = chain_issues
report['archives_in_any_applied_batch'] = len(chain)
report['latest_after_matches_installed'] = len(latest_ok)

# 5. untouched-by-.3 archives
dot3 = [b for b in applied if batches[b]['version'] == '2026-10-11.4']
report['dot3_batches'] = dot3
touched3 = set()
for b in dot3:
    touched3 |= set(batches[b]['archives'])
report['dot3_archive_count'] = len(touched3)
pre3 = {}
for f in installed:
    steps = [s for s in chain.get(f, []) if s[0] not in dot3]
    if steps:
        pre3[f] = (steps[-1][2], 'after of ' + steps[-1][0])
    elif f in doctor_orig:
        pre3[f] = (None, 'mod doctor patched (original in _ud-orig)')
    else:
        pre3[f] = (pristine.get(f), 'pristine LFS oid')
untouched_ok = []; untouched_bad = []; doctor_files = {}
for f in installed:
    if f in touched3:
        continue
    exp, src = pre3[f]
    if exp is None:
        doctor_files[f] = installed[f]
        continue
    (untouched_ok if installed[f] == exp else untouched_bad).append((f, src))
report['untouched_by_dot3'] = len(installed) - len(touched3)
report['untouched_match'] = len(untouched_ok)
report['untouched_mismatch'] = untouched_bad
report['untouched_by_source'] = {}
for f, src in untouched_ok:
    key = src.split(' of ')[0] if 'after' in src else src
    report['untouched_by_source'][src] = report['untouched_by_source'].get(src, 0) + 1
report['doctor_patched_installed'] = doctor_files
# .3 started from correct pre-.3 state
start_bad = []
for f in touched3:
    first3 = [s for s in chain[f] if s[0] in dot3][0]
    if first3[1] != pre3[f][0]:
        start_bad.append((f, pre3[f][1]))
report['dot3_before_equals_pre3_state'] = (len(touched3) - len(start_bad), start_bad)

# 6. member-level checks for .3 archives: before = earliest .3 backup, after = installed
member_report = {}
problems = []
for f in sorted(touched3):
    steps3 = [s for s in chain[f] if s[0] in dot3]
    bpath = ISSUE / steps3[0][0] / f
    before, border, bprops, btail = parse(bpath.read_bytes())
    after, aorder, aprops, atail = parse((ADDONS / f).read_bytes())
    stem = f[:-4].lower()
    recs = {}
    for b in dot3:
        for r in batches[b]['members']:
            if r['member'].split('/', 1)[0] == stem:
                recs[r['member'].split('/', 1)[1]] = r  # later batch overrides
    changed = set(recs)
    untouched_same = 0; untouched_diff = []
    for k in border:
        if k in changed:
            continue
        x, y = before[k], after.get(k)
        if y is None or x['data'] != y['data'] or (x['mime'], x['orig'], x['ts'], x['size'], x['name']) != (y['mime'], y['orig'], y['ts'], y['size'], y['name']):
            untouched_diff.append(k)
        else:
            untouched_same += 1
    removed = [k for k in border if k not in after]
    extra = [k for k in aorder if k not in before and not recs.get(k, {}).get('added')]
    order_ok = [k for k in aorder if k in before] == border
    after_hash_bad = []; after_compressed = []
    for k, r in recs.items():
        e = after.get(k)
        if e is None:
            after_hash_bad.append(k + ' missing'); continue
        if e['orig'] and e['orig'] != e['size']:
            after_compressed.append(k); continue
        if hashlib.sha256(e['data']).hexdigest() != r['after_sha256']:
            after_hash_bad.append(k)
    # before hash: only check uncompressed before entries
    before_hash_bad = []; before_compressed = 0
    for b in dot3:
        for r in batches[b]['members']:
            if r['member'].split('/', 1)[0] != stem or r.get('added'):
                continue
            k = r['member'].split('/', 1)[1]
            src = ISSUE / b / f
            ent = parse(src.read_bytes())[0].get(k)
            if ent is None:
                before_hash_bad.append(k + ' missing'); continue
            if ent['orig'] and ent['orig'] != ent['size']:
                before_compressed += 1; continue
            if hashlib.sha256(ent['data']).hexdigest() != r['before_sha256']:
                before_hash_bad.append(f'{b}:{k}')
    member_report[f] = dict(members_before=len(border), members_after=len(aorder), changed=len(changed),
                            added=sum(1 for r in recs.values() if r.get('added')), untouched_same=untouched_same,
                            untouched_diff=untouched_diff, removed=removed, unexpected_new=extra, order_preserved=order_ok,
                            props_same=(bprops == aprops), after_hash_bad=after_hash_bad, after_compressed=after_compressed,
                            before_hash_bad=before_hash_bad, before_compressed_skipped=before_compressed)
    if untouched_diff or removed or extra or not order_ok or after_hash_bad or before_hash_bad or bprops != aprops:
        problems.append(f)
report['member_problems'] = problems
report['member_totals'] = dict(
    archives=len(member_report),
    changed=sum(v['changed'] for v in member_report.values()),
    added=sum(v['added'] for v in member_report.values()),
    untouched_same=sum(v['untouched_same'] for v in member_report.values()),
    untouched_diff=sum(len(v['untouched_diff']) for v in member_report.values()),
    after_compressed=sum(len(v['after_compressed']) for v in member_report.values()),
    before_compressed_skipped=sum(v['before_compressed_skipped'] for v in member_report.values()))
report['member_detail'] = member_report

# 7. rolled-back batch vs 0532e837 backups
rb = [k for k in batches if batches[k]['state'] != 'applied']
rbinfo = {}
for k in rb:
    others = {}
    for b in dot3:
        for f, v in batches[b]['archives'].items():
            others.setdefault(f, v['before'])
    same = [f for f, v in batches[k]['archives'].items() if v['backup_sha'] == others.get(f)]
    rbinfo[k] = dict(state=batches[k]['state'], archives=batches[k]['n_archives'], backups_equal_dot3_before=len(same))
report['non_applied_batches'] = rbinfo

# 8. read-only attributes on installed
ro = sorted(p.name for p in ADDONS.glob('*.pbo') if not os.access(p, os.W_OK))
report['readonly_installed'] = ro


# 9. composition totals over all applied batches (latest record per member wins)
union = {}
for b in applied:
    for r in batches[b]['members']:
        union[r['member']] = r
report['composition_totals'] = dict(
    archives=len({k.split('/', 1)[0] for k in union}),
    archive_files=len(chain),
    changed=sum(1 for r in union.values() if not r.get('added')),
    added=sum(1 for r in union.values() if r.get('added')))

OUT.write_text(json.dumps(report, indent=1, default=str), encoding='utf8')
summary = {k: v for k, v in report.items() if k not in ('member_detail',)}
print(json.dumps(summary, indent=1, default=str)[:20000])
