# usage: resolve.py file spec1 spec2 ...  spec: ours|theirs|ot|to|@path (custom text, LF ok)
import sys
path=sys.argv[1]; specs=sys.argv[2:]
raw=open(path,'rb').read().decode('utf-8')
lines=raw.split('\n')
trail = lines[-1]==''
if trail: lines=lines[:-1]
lines=[l.rstrip('\r') for l in lines]
out=[];i=0;n=0
while i<len(lines):
    l=lines[i]
    if l.startswith('<<<<<<<'):
        ours=[];theirs=[];j=i+1
        while not lines[j].startswith('======='): ours.append(lines[j]); j+=1
        j+=1
        while not lines[j].startswith('>>>>>>>'): theirs.append(lines[j]); j+=1
        s=specs[n]; n+=1
        if s=='ours': out+=ours
        elif s=='theirs': out+=theirs
        elif s=='ot': out+=ours+theirs
        elif s=='to': out+=theirs+ours
        elif s.startswith('@'):
            t=open(s[1:],'rb').read().decode('utf-8').replace('\r\n','\n')
            if t.endswith('\n'): t=t[:-1]
            out+=t.split('\n')
        else: raise SystemExit('bad spec '+s)
        i=j+1; continue
    out.append(l); i+=1
assert n==len(specs), (n,len(specs))
open(path,'wb').write(('\r\n'.join(out)+('\r\n' if trail else '')).encode('utf-8'))
print('resolved',n,'in',path)
