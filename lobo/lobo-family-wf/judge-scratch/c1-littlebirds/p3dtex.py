import sys,re,importlib.util,collections
spec=importlib.util.spec_from_file_location('scan_mod',sys.argv[1]);
src=open(sys.argv[1]).read().replace('\nmain()\n','\n')
ns={}; exec(compile(src,'scan','exec'),ns)
data=open(sys.argv[2],'rb').read()
mem=ns['pbo_members'](data)
pat=sys.argv[3].lower() if len(sys.argv)>3 else ''
for name,blob,how in mem:
    if pat and pat not in name.lower(): continue
    if name.lower().endswith('.p3d'):
        tex=collections.Counter(m.group(0).decode('latin1').lower() for m in re.finditer(rb'[A-Za-z0-9_\/.\-]{3,}\.(?:pac|paa)',blob))
        print('##',name,how,len(blob))
        for t,n in sorted(tex.items()): print('   ',n,t)
    else:
        print('--',name,how,len(blob))
