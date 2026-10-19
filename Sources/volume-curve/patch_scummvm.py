#!/usr/bin/env python3
"""Install the compiled curve in the validated ScummVM 2026.4.0 function slot.
No ELF layout, symbols, plugins, input logic or system files are changed.
"""
import hashlib,struct,sys
from pathlib import Path
ORIGINAL='b664f9e133b3f63f5a215bc7763c4ee2f572d70d06dd46db057b78455b75c862'
ADDRESS,SIZE=0x3d8134,96

def patch(data,code):
 assert hashlib.sha256(data).hexdigest()==ORIGINAL,'Unsupported input executable'
 assert data[:6]==b'\x7fELF\x02\x01' and struct.unpack_from('<H',data,18)[0]==183
 assert len(code)<=SIZE and len(code)%4==0
 e=struct.unpack_from('<16sHHIQQQIHHHHHH',data,0)
 def segment(addr,size,flag):
  for i in range(e[10]):
   p=struct.unpack_from('<IIQQQQQQ',data,e[5]+i*e[9])
   if p[0]==1 and p[1]&flag and p[3]<=addr and addr+size<=p[3]+p[6]:return p
  raise AssertionError('Required ELF segment missing')
 p=segment(ADDRESS,SIZE,1);segment(0xf80078,4,2);segment(0xf860c1,1,2)
 offset=p[2]+ADDRESS-p[3]
 assert data[offset:offset+16]==struct.pack('<4I',0x90005d42,0x9101c042,0x91002042,0x88dffc44)
 replacement=code+struct.pack('<I',0xd503201f)*((SIZE-len(code))//4)
 out=data[:offset]+replacement+data[offset+SIZE:]
 assert len(out)==len(data) and out[:offset]==data[:offset] and out[offset+SIZE:]==data[offset+SIZE:]
 return out
if __name__=='__main__':
 if len(sys.argv)!=3:raise SystemExit('Usage: patch_scummvm.py original-scummvm output-scummvm')
 result=patch(Path(sys.argv[1]).read_bytes(),Path(__file__).with_name('curve.bin').read_bytes())
 out=Path(sys.argv[2]);out.write_bytes(result);out.chmod(0o755);print(hashlib.sha256(result).hexdigest())
