import csv, glob, os, sys, re, statistics as st
R3='D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/critic_r3_artefacts/timing'
R4='D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/critic_r4_artefacts/timing'
def load(root):
    d={}
    for f in glob.glob(root+'/*-timing/timings.csv'):
        for r in csv.DictReader(open(f,encoding='utf-8-sig')):
            m=r['mode']; kind='tip' if m.startswith('tip') else 'ov'
            def g(k):
                try: return float(r[k])
                except: return None
            row={k:g(k) for k in ['gpu_frame_ms','gpu_world_ms','gpu_post_ms','gpu_ao_ms','gpu_composite_ms']}
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
            pg=r.get('poseidon_games','')
            if 'foreign' in pg and not pg.endswith('foreign0'): n+=1
    return n,tot
def mean(rows,k):
    v=[x[k] for x in rows if x[k] is not None]
    return st.mean(v) if v else float('nan')
r3=load(R3); r4=load(R4)
order=['noon_fields','lowsun_fp','forest_edge_golden','sunrise_ridge','coastline_sunset','woods_pine2_evening','blue_hour_coast','dawn_valley','pine_rays','town_night','town_fog_night','town_night_rain','jeep_close_noon','town_noon','forest_morning','overcast_broken','overcast_heavy','rain_day','noon_vista','sinai_lowsun_fp','sinai_desert_noon']
print('| view | min-frame r3 tip -> ov | r3 min delta | min-frame r4 tip -> ov | r4 min delta | r3 tip -> ov (ms) | r3 delta | r4 tip -> ov (ms) | r4 delta | r4 ratio ov/tip | r3 ratio | r4 world delta | r4 post delta | ao r3->r4 | air r3->r4 | composite r3->r4 | n |')
print('|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|')
d3s=[];d4s=[];rr3=[];rr4=[];md3=[];md4=[]
for v in order:
    if v not in r4: continue
    a=r3.get(v,{}); b=r4[v]
    t3,o3=mean(a.get('tip',[]),'gpu_frame_ms'),mean(a.get('ov',[]),'gpu_frame_ms')
    t4,o4=mean(b.get('tip',[]),'gpu_frame_ms'),mean(b.get('ov',[]),'gpu_frame_ms')
    wd=mean(b['ov'],'gpu_world_ms')-mean(b['tip'],'gpu_world_ms'); pd=mean(b['ov'],'gpu_post_ms')-mean(b['tip'],'gpu_post_ms')
    f=lambda x:f"{x:.2f}"
    m3t,m3o=mean(a.get('tip',[]),'minf'),mean(a.get('ov',[]),'minf'); m4t,m4o=mean(b['tip'],'minf'),mean(b['ov'],'minf')
    md3.append(m3o-m3t); md4.append(m4o-m4t)
    print(f"| {v} | {f(m3t)} -> {f(m3o)} | {m3o-m3t:+.2f} | {f(m4t)} -> {f(m4o)} | {m4o-m4t:+.2f} | {f(t3)} -> {f(o3)} | {o3-t3:+.2f} | {f(t4)} -> {f(o4)} | {o4-t4:+.2f} | {o4/t4:.3f} | {o3/t3:.3f} | {wd:+.2f} | {pd:+.2f} | {f(mean(a['ov'],'gpu_ao_ms'))} -> {f(mean(b['ov'],'gpu_ao_ms'))} | {f(mean(a['ov'],'air'))} -> {f(mean(b['ov'],'air'))} | {f(mean(a['ov'],'gpu_composite_ms'))} -> {f(mean(b['ov'],'gpu_composite_ms'))} | {len(b['tip'])}+{len(b['ov'])} |")
    d3s.append(o3-t3); d4s.append(o4-t4); rr3.append(o3/t3); rr4.append(o4/t4)
print()
print(f"mean delta r3 {st.mean(d3s):+.2f} ms, r4 {st.mean(d4s):+.2f} ms over {len(d4s)} views; mean ratio ov/tip r3 {st.mean(rr3):.3f}, r4 {st.mean(rr4):.3f}")
print(f"mean min-frame delta r3 {st.mean(md3):+.2f} ms, r4 {st.mean(md4):+.2f} ms")
print('foreign PoseidonGame samples r4: %d of %d; r3: %d of %d' % (foreign(R4)+foreign(R3)))
