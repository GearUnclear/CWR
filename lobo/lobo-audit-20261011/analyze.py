import json,pathlib,collections,re
root=pathlib.Path('tmp/lobo-audit-20261011')
rows=[]
for p in root.glob('*/audit.json'):
 try: j=json.loads(p.read_text())
 except Exception: continue
 for section in ('CfgAmmo','CfgWeapons','CfgVehicles'):
  for name,v in j.get(section,{}).items():
   if isinstance(v,dict): rows.append((p.parent.name,section,name,v))
keys=['hit','indirectHit','indirectHitRange','minRange','midRange','maxRange','maxSpeed','explosive','simulation','initSpeed','reloadTime','count','armor','armorStructural','displayName']
with (root/'values.tsv').open('w') as f:
 f.write('addon\tsection\tclass\t'+'\t'.join(keys)+'\n')
 for a,s,n,v in rows:
  if v: f.write('\t'.join([a,s,n]+[str(v.get(k,'')) for k in keys])+'\n')
for a,s,n,v in rows:
 if s=='CfgAmmo' and a.lower().startswith('lobo') and 'hit' in v: print(a,n,{k:v[k] for k in keys if k in v})
