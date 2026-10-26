import re,glob,sys,os
base=os.path.dirname(os.path.abspath(__file__))
pat=sys.argv[1] if len(sys.argv)>1 else 'guerrilla_forces_heli_insertion'
for d in sorted(glob.glob(os.path.join(base,pat+'-*')), key=lambda p:int(re.findall(r'-(\d+)$',p)[0]) if re.findall(r'-(\d+)$',p) else 0):
    if not os.path.isdir(d): continue
    logs=glob.glob(os.path.join(d,'*','game_stdout.log'))
    if not logs: continue
    txt=open(logs[0],encoding='utf-8',errors='replace').read()
    txt=re.sub(r'\x1b\[[0-9;]*m','',txt)
    tri=open(d+'.tri.txt',encoding='utf-8',errors='replace').read() if os.path.exists(d+'.tri.txt') else ''
    res='PASS' if re.search(r'passed\S* 1/1',tri) else ('FAIL' if 'failed' in tri else 'running')
    fails=re.findall(r'FAIL\x1b?\S*\s+\S+ -- (.*)',re.sub(r'\x1b\[[0-9;]*m','',tri))
    print('==',os.path.basename(d),res, (fails[0][:160] if fails else ''))
    for line in re.findall(r'OccupierAir: (?!trace)(.*)',txt):
        print('   ',line[:200])
