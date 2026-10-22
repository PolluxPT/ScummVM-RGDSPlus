#!/usr/bin/env python3
"""Package validated runtime files; does not compile or download executables."""
import argparse,hashlib,zipfile
from pathlib import Path,PurePosixPath
ROOT=Path(__file__).resolve().parents[1]

def safe(name):
 p=PurePosixPath(name)
 if p.is_absolute() or '..' in p.parts:raise ValueError('Unsafe path: '+name)
 return name

def write_zip(path,entries):
 with zipfile.ZipFile(path,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
  for name,data in sorted(entries.items()):
   safe(name);i=zipfile.ZipInfo(name,(2026,10,22,0,0,0));i.compress_type=zipfile.ZIP_DEFLATED
   executable=name.endswith('.sh') or name in ('APPS/ScummVM_RGDSPlus/scummvm','APPS/ScummVM_RGDSPlus/tools/rgds-screen-menu')
   i.external_attr=(0o100755 if executable else 0o100644)<<16;z.writestr(i,data)

def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('--runtime',type=Path,required=True,help='Validated application ZIP with APPS at its root')
 ap.add_argument('--sources',type=Path,help='Complete source ZIP if not included in runtime ZIP')
 ap.add_argument('--output',type=Path,default=ROOT/'dist')
 args=ap.parse_args();version=(ROOT/'VERSION').read_text().strip()
 if not version or any(c not in '0123456789.-' for c in version):raise ValueError('Invalid VERSION')
 names=(ROOT/'packaging/runtime-files.txt').read_text().splitlines()
 if len(names)!=len(set(names)):raise ValueError('Duplicate runtime manifest paths')
 app={};sources={}
 with zipfile.ZipFile(args.runtime) as z:
  for name in names:
   safe(name)
   if not name.startswith('APPS/') or set(PurePosixPath(name).parts)&{'config','saves','games','cache','logs-anteriores'}:raise ValueError('Non-runtime manifest entry')
   overlay=ROOT/name
   app[name]=overlay.read_bytes() if overlay.is_file() else z.read(name)
  for name in z.namelist():
   if name.startswith('Sources/') and not name.endswith('/'):sources[safe(name)]=z.read(name)
 if args.sources:
  with zipfile.ZipFile(args.sources) as z:
   for name in z.namelist():
    if name.startswith('Sources/') and not name.endswith('/'):sources[safe(name)]=z.read(name)
 for p in (ROOT/'Sources').rglob('*'):
  if p.is_file():sources[p.relative_to(ROOT).as_posix()]=p.read_bytes()
 upstream='Sources/scummvm-2026.4.0.tar.xz'
 if upstream not in sources:raise ValueError('Complete upstream sources missing; use --sources')
 if hashlib.sha256(sources[upstream]).hexdigest()!='cd660b34104da7f85ed0a11b7c6c2320c629bb87ee297935711ebc1e09731f12':raise ValueError('Unexpected upstream source archive')
 for name,data in app.items():
  if name.endswith('.sh') or '/LICENSES/' in name or name.endswith('/COPYING') or name=='APPS/Imgs/ScummVM.png':sources[name]=data
 sources['SOURCE_PACKAGE.md']=(ROOT/'docs/DEVELOPMENT_PT_EN.md').read_bytes()
 args.output.mkdir(parents=True,exist_ok=True)
 runtime=args.output/f'ScummVM_RGDSPlus_v{version}.zip'
 source=args.output/f'ScummVM_RGDSPlus_v{version}_Sources.zip'
 write_zip(runtime,app);write_zip(source,sources)
 checks=''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.name+'\n' for p in (runtime,source))
 (args.output/'SHA256SUMS.txt').write_text(checks,encoding='utf-8')
 print('Ready:',runtime.name,source.name,'SHA256SUMS.txt')
if __name__=='__main__':main()
