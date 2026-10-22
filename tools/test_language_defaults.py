#!/usr/bin/env python3
"""Exercise the actual launcher config block without touching console hardware."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
launcher = (ROOT / 'APPS/ScummVM.sh').read_text()
block = launcher[launcher.index('cfg="$app_dir/config/scummvm.ini"'):launcher.index('theme_args=()')]

def check(initial, migrated, expected):
    with tempfile.TemporaryDirectory() as tmp:
        app = Path(tmp)
        (app / 'config').mkdir()
        cfg = app / 'config/scummvm.ini'
        if initial is not None:
            cfg.write_text(initial)
        if migrated:
            (app / 'config/gui-2026-migrated').write_text('ok\n')
        subprocess.run(['bash', '-eu', '-c', 'app_dir="$1"\n' + block, 'test', tmp], check=True)
        text = cfg.read_text()
        global_section = text.split('[scummvm]\n', 1)[1].split('\n[', 1)[0]
        assert f'gui_language={expected}\n' in global_section + '\n', text
        if initial and '[samnmax]' in initial:
            assert text.split('[samnmax]', 1)[1].split('\n[', 1)[0].rstrip() == initial.split('[samnmax]', 1)[1].rstrip()
        # A second startup must preserve the result byte for byte.
        subprocess.run(['bash', '-eu', '-c', 'app_dir="$1"\n' + block, 'test', tmp], check=True)
        assert cfg.read_text() == text

check(None, False, 'en')
for lang in ['pt_PT', 'en', 'de_DE']:
    for migrated in [False, True]:
        check(f'[scummvm]\ngui_language={lang}\nvolume=170\n[samnmax]\npath=/games/Sam and Max\nlanguage=pt\n', migrated, lang)
check('[scummvm]\nvolume=170\n[samnmax]\ngui_language=pt_PT\npath=/games/Sam\n', False, 'en')
check('[samnmax]\npath=/games/Sam\n', False, 'en')
print('PASS: fresh EN, saved PT/EN/DE, legacy migration, missing global language, game entries and repeated startup')
