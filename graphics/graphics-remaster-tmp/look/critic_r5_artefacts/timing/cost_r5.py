import csv, glob, re, statistics as st
R={'r3':'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/critic_r3_artefacts/timing',
   'r4':'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/critic_r4_artefacts/timing',
   'r5':'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/critic_r5_artefacts/timing'}
R2='D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/director/round2/cost.md'
def load(root):
    d={}
    for f in glob.glob(root+'/*-timing/timings.csv'):
        for r in csv.DictReader(open(f,encoding='utf-8-sig')):
            kind='tip' if r['mode'].startswith('tip') else 'ov'
            def g(k):
                try: return float(r[k])
                except: return None
            row={k:g(k) for k in ['gpu_frame_ms','gpu_world_ms','gpu_post_ms','gpu_ao_ms','gpu_composite_ms','cpu_frame_ms']}
            raw=r.get('gpu_raw','')
            m1=re.search(r' air ([\d.]+)',raw); row['air']=float(m1.group(1)) if m1 else None
            m2=re.search(r'min frame=([\d.]+)',raw); row['minf']=float(m2.group(1)) if m2 else None
            d.setdefault(r['view'],{}).setdefault(kind,[]).append(row)
    return d
def foreign(root):
    n=tot=0
    for f in glob.glob(root+'/*-timing/gpu_samples.csv'):
        for r in csv.DictReader(open(f,encoding='utf-8-sig')):
            tot+=1
            if not r.get('poseidon_games','').endswith('foreign0'): n+=1
    return n,tot
def mean(rows,k):
    v=[x[k] for x in rows if x.get(k) is not None]
    return st.mean(v) if v else float('nan')
r2={}
for line in open(R2,encoding='utf-8'):
    m=re.match(r'\| (\w+) \| ([\d.]+) -> ([\d.]+) \| ([+-][\d.]+) \|',line)
    if m: r2[m.group(1)]=(float(m.group(2)),float(m.group(3)))
D={k:load(v) for k,v in R.items()}
order=['noon_fields','lowsun_fp','forest_edge_golden','sunrise_ridge','coastline_sunset','woods_pine2_evening','blue_hour_coast','dawn_valley','pine_rays','town_night','town_fog_night','town_night_rain','jeep_close_noon','town_noon','forest_morning','overcast_broken','overcast_heavy','rain_day','noon_vista','sinai_lowsun_fp','sinai_desert_noon']
print('| view | r2 tip -> ov | r2 d | r3 tip -> ov | r3 d | r5 tip -> ov | r5 d | r5 ratio | r5 min-frame d | r3 min-frame d | r5 world d | r5 post d | ao r3->r5 | air r3->r5 | composite r3->r5 | r5 ov cpu ms | n |')
print('|'+'---|'*17)
S={k:[] for k in ['d2','d3','d5','q2','q3','q5','m3','m5','d4']}
for v in order:
    a=D['r3'].get(v,{}); b=D['r5'].get(v,{}); c=D['r4'].get(v,{})
    if not b: continue
    t3,o3=mean(a['tip'],'gpu_frame_ms'),mean(a['ov'],'gpu_frame_ms')
    t5,o5=mean(b['tip'],'gpu_frame_ms'),mean(b['ov'],'gpu_frame_ms')
    t4,o4=mean(c['tip'],'gpu_frame_ms'),mean(c['ov'],'gpu_frame_ms')
    t2,o2=r2.get(v,(float('nan'),float('nan')))
    m3=mean(a['ov'],'minf')-mean(a['tip'],'minf'); m5=mean(b['ov'],'minf')-mean(b['tip'],'minf')
    wd=mean(b['ov'],'gpu_world_ms')-mean(b['tip'],'gpu_world_ms'); pd=mean(b['ov'],'gpu_post_ms')-mean(b['tip'],'gpu_post_ms')
    f=lambda x:f"{x:.2f}"
    print(f"| {v} | {f(t2)} -> {f(o2)} | {o2-t2:+.2f} | {f(t3)} -> {f(o3)} | {o3-t3:+.2f} | {f(t5)} -> {f(o5)} | {o5-t5:+.2f} | {o5/t5:.3f} | {m5:+.2f} | {m3:+.2f} | {wd:+.2f} | {pd:+.2f} | {f(mean(a['ov'],'gpu_ao_ms'))} -> {f(mean(b['ov'],'gpu_ao_ms'))} | {f(mean(a['ov'],'air'))} -> {f(mean(b['ov'],'air'))} | {f(mean(a['ov'],'gpu_composite_ms'))} -> {f(mean(b['ov'],'gpu_composite_ms'))} | {f(mean(b['ov'],'cpu_frame_ms'))} | {len(b['tip'])}+{len(b['ov'])} |")
    S['d2'].append(o2-t2); S['d3'].append(o3-t3); S['d5'].append(o5-t5); S['d4'].append(o4-t4)
    S['q2'].append(o2/t2); S['q3'].append(o3/t3); S['q5'].append(o5/t5); S['m3'].append(m3); S['m5'].append(m5)
print()
print(f"mean GPU frame delta (overhaul - landed tip, same build): r2 {st.mean(S['d2']):+.2f} ms, r3 {st.mean(S['d3']):+.2f} ms, r5 {st.mean(S['d5']):+.2f} ms (r4, not quiet: {st.mean(S['d4']):+.2f} ms) over {len(S['d5'])} views")
print(f"median delta: r2 {st.median(S['d2']):+.2f}, r3 {st.median(S['d3']):+.2f}, r5 {st.median(S['d5']):+.2f}; max delta r5 {max(S['d5']):+.2f}")
print(f"mean ratio overhaul/tip: r2 {st.mean(S['q2']):.3f}, r3 {st.mean(S['q3']):.3f}, r5 {st.mean(S['q5']):.3f}")
print(f"mean min-frame delta: r3 {st.mean(S['m3']):+.2f} ms, r5 {st.mean(S['m5']):+.2f} ms")
for k in ['r3','r4','r5']:
    n,t=foreign(R[k]); print(f"foreign PoseidonGame samples {k}: {n} of {t}")
