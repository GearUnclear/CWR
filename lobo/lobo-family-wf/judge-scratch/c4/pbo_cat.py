import sys,importlib.util
spec=importlib.util.spec_from_file_location('p',r'D:/Arma_CWA/tmp/lobo-family-wf/judge-scratch/c4/pbo_ls.py')
src=open(r'D:/Arma_CWA/tmp/lobo-family-wf/judge-scratch/c4/pbo_ls.py').read().split('for p in sys.argv')[0]
ns={}; exec(src,ns)
for n,m,o,s,b in ns['read_pbo'](sys.argv[1]):
    if n.lower()==sys.argv[2].lower():
        sys.stdout.write(b.decode('latin1'))
