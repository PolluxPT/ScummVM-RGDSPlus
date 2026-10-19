from pathlib import Path
import struct,sys
from unicorn import Uc,UC_ARCH_ARM64,UC_MODE_ARM
from unicorn.arm64_const import *
code=Path(__file__).with_name('curve.bin').read_bytes()
u=Uc(UC_ARCH_ARM64,UC_MODE_ARM)
for addr,size in [(0x3d8000,4096),(0xf80000,4096),(0xf86000,4096),(0x2000000,0x40000),(0x3000000,4096)]:u.mem_map(addr,size)
u.mem_write(0x3d8134,code)
samples=list(range(-32768,32768));raw=struct.pack('<65536h',*samples)
for enabled in (0,1):
 u.mem_write(0xf860c1,bytes([enabled]))
 for level in range(11):
  u.mem_write(0xf80078,struct.pack('<i',level*10));u.mem_write(0x2000000,raw+b'\x5a')
  u.reg_write(UC_ARM64_REG_X0,0x2000000);u.reg_write(UC_ARM64_REG_X1,len(raw)+1);u.reg_write(UC_ARM64_REG_X30,0x3000000)
  u.emu_start(0x3d8134,0x3000000)
  got=struct.unpack('<65536h',bytes(u.mem_read(0x2000000,len(raw))))
  for sample,actual in zip(samples,got):
   n=sample*35*level*level;expected=abs(n)//10000*(-1 if n<0 else 1) if enabled else sample
   assert actual==expected,(enabled,level,sample,actual,expected)
  assert u.mem_read(0x2000000+len(raw),1)==b'\x5a'
for length in (0,1,-1,-100):
 u.reg_write(UC_ARM64_REG_X0,0xdead0000);u.reg_write(UC_ARM64_REG_X1,length&0xffffffff);u.reg_write(UC_ARM64_REG_X30,0x3000000)
 u.emu_start(0x3d8134,0x3000000)
print('AArch64 machine code PASS: every signed 16-bit sample, all 11 levels, enabled/disabled, odd tail and empty/negative lengths')
