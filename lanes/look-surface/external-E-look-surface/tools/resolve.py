import re,sys
def resolve(path, fn):
    s=open(path,encoding='utf-8',newline='').read()
    pat=re.compile(r'<<<<<<< ours\r?\n(.*?)=======\r?\n(.*?)>>>>>>> theirs\r?\n',re.S)
    i=[0]
    def rep(m):
        r=fn(i[0],m.group(1),m.group(2)); i[0]+=1; return r
    s2=pat.sub(rep,s)
    open(path,'w',encoding='utf-8',newline='').write(s2)
    print(path,i[0])
both=lambda k,o,t:o+t
ours=lambda k,o,t:o
