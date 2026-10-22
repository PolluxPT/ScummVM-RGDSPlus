#!/usr/bin/env python3
"""Test the shipped helper and actual launcher trap wiring on temporary files."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
helper = ROOT / 'APPS/ScummVM_RGDSPlus/tools/volume-control.sh'
launcher = (ROOT / 'APPS/ScummVM.sh').read_text()
assert launcher.index('rgds_volume_save /sys/') < launcher.index('./scummvm ')
cleanup = next(line for line in launcher.splitlines() if line.startswith('rgds_cleanup()'))
assert 'rgds_volume_restore' in cleanup

for level in range(11):
    for ending, status in [('exit 0', 0), ('exit 7', 7), ('kill -TERM $$', 143), ('kill -HUP $$', 143), ('kill -INT $$', 130)]:
        with tempfile.TemporaryDirectory() as tmp:
            node = Path(tmp) / 'volume'
            node.write_text(f'{level}\n')
            script = '''set -u
. "$1"
rgds_screens_restore() { :; }
rgds_touch_restore() { :; }
''' + cleanup + '''
trap rgds_cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM HUP
rgds_volume_save "$2" || exit 90
printf '9\\n' > "$2"
rgds_volume_save "$2" || exit 91
printf '1\\n' > "$2"
''' + ending
            result = subprocess.run(['bash', '-c', script, 'test', str(helper), str(node)], capture_output=True, text=True)
            assert result.returncode == status, result
            assert node.read_text() == f'{level}\n', (level, ending)

for invalid in ['', '-1', '11', 'broken']:
    with tempfile.TemporaryDirectory() as tmp:
        node = Path(tmp) / 'volume'
        node.write_text(invalid)
        script = '. "$1"; if rgds_volume_save "$2"; then exit 90; fi; rgds_volume_restore'
        subprocess.run(['bash', '-c', script, 'test', str(helper), str(node)], check=True, capture_output=True)
        assert node.read_text() == invalid
print('PASS: initial levels 0..10 restored after normal/error/signal exits; repeated save preserves original; invalid input unchanged')
