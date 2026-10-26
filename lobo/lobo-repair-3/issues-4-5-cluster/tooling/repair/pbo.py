"""Preserve PBO member bytes/metadata while replacing explicitly selected members."""
from dataclasses import dataclass
import hashlib
from pathlib import Path
import struct
import subprocess

VERS=0x56657273
CPRS=0x43707273
@dataclass(frozen=True)
class Entry:
    name: bytes
    fields: tuple
    offset: int
    @property
    def key(self):return normalize(self.name.decode('latin1'))

def normalize(name):
    value=name.replace('\\','/').lower()
    if value.startswith('/') or ':' in value or any(p in ('..','.', '') for p in value.split('/')):
        raise ValueError(f'Unsafe PBO member: {name}')
    return value

class Archive:
    def __init__(self,data):
        self.data=data;self.entries=[];p=0;prefix_end=0;seen=set()
        while True:
            z=data.find(b'\0',p)
            if z<0 or z+21>len(data):raise ValueError('Truncated PBO header')
            name=data[p:z];p=z+1;fields=struct.unpack_from('<5I',data,p);p+=20
            if not name:
                if fields[0]==VERS and not self.entries and not prefix_end:
                    while True:
                        z=data.find(b'\0',p)
                        if z<0:raise ValueError('Truncated PBO property key')
                        empty=z==p;p=z+1
                        if empty:break
                        z=data.find(b'\0',p)
                        if z<0:raise ValueError('Truncated PBO property value')
                        p=z+1
                    prefix_end=p;continue
                self.terminator=data[p-21:p];break
            key=normalize(name.decode('latin1'))
            if key in seen:raise ValueError(f'Duplicate PBO member: {key}')
            seen.add(key);self.entries.append(Entry(name,fields,0))
        self.prefix=data[:prefix_end];self.header_end=p
        for i,e in enumerate(self.entries):
            self.entries[i]=Entry(e.name,e.fields,p);p+=e.fields[4]
            if p>len(data):raise ValueError('PBO payload exceeds archive size')
        self.trailer=data[p:];self.payload_end=p
        self.by_key={e.key:e for e in self.entries}
        self.checksummed=len(self.trailer)==21 and self.trailer[:1]==b'\0'
        self.stuffpbo=len(self.trailer)==17 and self.trailer[2:]==b'\x01StuffPBO V1.0b'
        if self.checksummed:
            if hashlib.sha1(data[:p]).digest()!=self.trailer[1:]:raise ValueError('PBO checksum mismatch')
        elif self.trailer:
            # Original OFP textual packer signatures are valid. A binary tail,
            # including a checksum with its delimiter consumed by truncated
            # payload, cannot safely be treated as a signature.
            if not self.stuffpbo and not all(32<=b<=126 or b in (9,10,13) for b in self.trailer):
                raise ValueError('Malformed PBO checksum or unsupported binary trailer')
        # Some original OFP packers append textual signatures. Keep these opaque.

    def stored(self,entry):return memoryview(self.data)[entry.offset:entry.offset+entry.fields[4]]

    def read(self,key,path=None,tools=None):
        e=self.by_key[normalize(key)]
        if e.fields[0]==0:return bytes(self.stored(e))
        if tools is None or path is None:raise ValueError(f'Compressed member {key} needs PoseidonTools')
        if Path(path).read_bytes()!=self.data:raise ValueError(f'Archive changed before compressed read: {path}')
        result=subprocess.run([str(tools),'pbo','show',str(path),e.name.decode('latin1')],capture_output=True)
        if Path(path).read_bytes()!=self.data:raise ValueError(f'Archive changed during compressed read: {path}')
        if result.returncode:raise ValueError(f'Cannot decode {path}/{key}: {result.stderr.decode(errors="replace")}')
        if len(result.stdout)!=e.fields[1]:raise ValueError(f'Decoded size mismatch: {path}/{key}')
        return result.stdout

    def replace(self,replacements):
        if replacements and self.stuffpbo:raise ValueError('Legacy StuffPBO checksum rewriting is unsupported')
        normalized={}
        for key,value in replacements.items():
            key=normalize(key)
            if key in normalized:raise ValueError(f'Duplicate replacement member: {key}')
            normalized[key]=value
        replacements=normalized
        unknown=set(replacements)-self.by_key.keys()
        if unknown:raise ValueError(f'Cannot add unknown PBO members: {sorted(unknown)}')
        headers=[self.prefix];payloads=[]
        for e in self.entries:
            if e.key in replacements:
                payload=replacements[e.key]
                if not isinstance(payload,bytes):raise TypeError('PBO replacement must be bytes')
                # Converted members are uncompressed; other records remain exact.
                fields=(0,0,e.fields[2],e.fields[3],len(payload))
            else:payload=self.stored(e);fields=e.fields
            headers.extend((e.name+b'\0',struct.pack('<5I',*fields)));payloads.append(payload)
        body=b''.join(headers+[self.terminator]+payloads)
        tail=b'\0'+hashlib.sha1(body).digest() if self.checksummed else self.trailer
        result=body+tail
        after=Archive(result)
        if list(after.by_key)!=list(self.by_key):raise ValueError('PBO member order changed')
        for e in self.entries:
            if e.key not in replacements:
                a=after.by_key[e.key]
                if e.fields!=a.fields or self.stored(e)!=after.stored(a):raise ValueError(f'Untouched member changed: {e.key}')
        return result
