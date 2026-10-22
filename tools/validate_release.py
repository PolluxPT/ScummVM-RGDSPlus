#!/usr/bin/env python3
import hashlib,sys,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
 directory=Path(sys.argv[1]) if len(sys.argv)>1 else ROOT/'dist'
 version=(ROOT/'VERSION').read_text().strip()
 expected=set((ROOT/'packaging/runtime-files.txt').read_text().splitlines())
 runtime=directory/f'ScummVM_RGDSPlus_v{version}.zip'
 with zipfile.ZipFile(runtime) as z:
  assert z.testzip() is None
  assert set(z.namelist())==expected,'Unexpected or missing installation files'
  assert all(n.startswith('APPS/') for n in z.namelist())
  for name in ('APPS/ScummVM.sh', 'APPS/ScummVM_RGDSPlus/tools/volume-control.sh', 'APPS/ScummVM_RGDSPlus/tools/rgds-screen-menu'):
   assert z.read(name)==(ROOT/name).read_bytes(), 'Stale release overlay: '+name
   assert z.getinfo(name).external_attr>>16 & 0o111, 'Missing executable permission: '+name
  menu=z.read('APPS/ScummVM_RGDSPlus/tools/rgds-screen-menu')
  assert menu[:6]==b'\x7fELF\x02\x01' and int.from_bytes(menu[18:20], 'little')==183
  assert z.read('APPS/Imgs/ScummVM.png')==(ROOT/'APPS/Imgs/ScummVM.png').read_bytes()
  assert hashlib.sha256(z.read('APPS/ScummVM_RGDSPlus/scummvm')).hexdigest()=='5d630ce6c2d1ecb947a1c640f224e18c3e6ff9ebb442863e9c27d58abbe139b6'
  assert len([n for n in z.namelist() if n.startswith('APPS/ScummVM_RGDSPlus/plugins/') and n.endswith('.so')])==113
  assert len([n for n in z.namelist() if n.startswith('APPS/') and n.count('/')==1 and n.endswith('.sh')])==1
 with zipfile.ZipFile(directory/f'ScummVM_RGDSPlus_v{version}_Sources.zip') as z:
  assert z.testzip() is None
  for name in ('Sources/screens/menu.c', 'Sources/volume-test.cpp', 'APPS/ScummVM.sh', 'APPS/ScummVM_RGDSPlus/tools/volume-control.sh'):
   assert z.read(name)==(ROOT/name).read_bytes(), 'Stale source package: '+name
  assert hashlib.sha256(z.read('Sources/scummvm-2026.4.0.tar.xz')).hexdigest()=='cd660b34104da7f85ed0a11b7c6c2320c629bb87ee297935711ebc1e09731f12'
 for line in (directory/'SHA256SUMS.txt').read_text().splitlines():
  expected_hash,name=line.split('  ',1);assert hashlib.sha256((directory/name).read_bytes()).hexdigest()==expected_hash
 print('PASS: clean APPS-only ZIP, supplied icon, validated executable, 113 plugins, complete sources and checksums')
if __name__=='__main__':main()
