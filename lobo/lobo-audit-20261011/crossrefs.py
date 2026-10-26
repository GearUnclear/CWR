import pathlib,re,json,collections,difflib
root=pathlib.Path('tmp/lobo-audit-20261011'); docs=[]; known=collections.defaultdict(set)
for p in root.glob('*/audit.json'):
 try:j=json.loads(p.read_text())
 except Exception:continue
 j={next((s for s in ("CfgAmmo","CfgWeapons","CfgVehicles") if s.lower()==k.lower()),k):v for k,v in j.items()}; docs.append((p,j))
 for sec in ('CfgAmmo','CfgWeapons','CfgVehicles'):
  known[sec].update(n.lower() for n in j.get(sec,{}))
problems=[]
def walk(p,obj,loc):
 if not isinstance(obj,dict):return
 for k,v in obj.items():
  if isinstance(v,dict):walk(p,v,loc+'/'+k)
  if k.lower() not in ('ammo','weapons','magazines','crew'):continue
  sec={'ammo':'CfgAmmo','weapons':'CfgWeapons','magazines':'CfgWeapons','crew':'CfgVehicles'}[k.lower()]
  for n in v if isinstance(v,list) else [v]:
   if isinstance(n,str) and n.lower().startswith('lobo') and n.lower() not in known[sec]:problems.append(f'{p.parent.name} {loc}/{k}: {n} not defined in {sec}')
for p,j in docs:
 for s in ('CfgAmmo','CfgWeapons','CfgVehicles'):walk(p,j.get(s,{}),s)
(root/'unresolved-lobo.txt').write_text('\n'.join(problems))
print('\n'.join(problems[:55]));print('Unresolved candidates',len(problems))
issues=[]
for p in root.rglob('*.sqs'):
 t=p.read_text(errors='replace'); labels={m.group(1).lower() for m in re.finditer(r'^\s*#\s*([^\s;]+)',t,re.M)}
 for i,l in enumerate(t.splitlines(),1):
  if l.lstrip().startswith(';'):continue
  for v in re.findall(r'\bgoto\s+"([^"\n]+)"',l,re.I):
   if v.lower() not in labels:issues.append((str(p),i,v))
(root/'missing-labels.json').write_text(json.dumps(issues,indent=2))
print('Missing label candidate jumps:',len(issues),'in scripts:',len(set(p for p,i,v in issues)))
print('\n'.join(f'{p}:{i} -> {v}' for p,i,v in issues if v.lower() not in ('exit','end','ende','n2','n3'))[:4500])

