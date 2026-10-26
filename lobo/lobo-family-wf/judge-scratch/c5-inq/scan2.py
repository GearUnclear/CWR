import sys,re,glob,os
src=open('tmp/lobo-family-wf/judge-scratch/c5-inq/scan.py').read().split("pat=re.compile")[0]
exec(src)
BS=chr(92)*2
pat=re.compile(('('+BS+'inq|inq_|"inq|inq'+BS+'|inquisitor|inqw|inqak)').encode(),re.I)
files=glob.glob('@LoBo/addons/*.pbo')+glob.glob('@LoBo/Missions/*.pbo')+glob.glob('D:/LoBo-Family/stage/UD-LoBo-Family-2026-10-11/@LoBo_Deps/Addons/*.pbo')
print(len(files),'pbos')
for p in files:
    try: ents=parse(p)
    except Exception as ex: print('PARSEFAIL',p,ex); continue
    for name,raw in ents:
        for m in pat.finditer(raw):
            s=raw[max(0,m.start()-60):m.end()+60].decode('latin1','replace')
            print('HIT',os.path.basename(p),name,repr(s))
