import sys, struct, os, glob, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pbo import entries
def lzss(data, outlen):
    out=bytearray(); i=0; n=len(data)
    while len(out)<outlen and i<n:
        flags=data[i]; i+=1
        for b in range(8):
            if len(out)>=outlen or i>=n: break
            if flags & (1<<b):
                out.append(data[i]); i+=1
            else:
                if i+1>=n: break
                lo=data[i]; hi=data[i+1]; i+=2
                rpos=lo | ((hi&0xF0)<<4)
                rlen=(hi&0x0F)+3
                start=len(out)-rpos
                for k in range(rlen):
                    if len(out)>=outlen: break
                    idx=start+k
                    out.append(out[idx] if idx>=0 else 0x20)
    return bytes(out)
def main():
    pats=[p.encode().lower() for p in sys.argv[1].split(',')]
    files=sys.argv[2:]
    for p in files:
        try:
            es,base=entries(p)
        except Exception as e:
            print('ERR',p,e); continue
        off=base
        with open(p,'rb') as f:
            data=f.read()
        hits=[]
        ncomp=0
        for name,packing,orig,size in es:
            blob=data[off:off+size]; off+=size
            lname=name.lower().encode('latin1')
            for pt in pats:
                if pt in lname: hits.append(('NAME',name,pt.decode()))
            if packing==0x43707273 or (orig and orig!=size and packing!=0):
                ncomp+=1
                try: blob=lzss(blob,orig)
                except Exception as e: hits.append(('LZERR',name,str(e)))
            lb=blob.lower()
            for pt in pats:
                c=lb.count(pt)
                if c:
                    # context
                    i=lb.find(pt)
                    ctx=blob[max(0,i-60):i+80].replace(b'\0',b' ')
                    hits.append(('BYTES',name,pt.decode(),c,ctx.decode('latin1','replace')))
        if hits:
            print('==',os.path.basename(p),'entries',len(es),'compressed',ncomp)
            for h in hits[:40]: print('  ',h)
if __name__=="__main__": main()
