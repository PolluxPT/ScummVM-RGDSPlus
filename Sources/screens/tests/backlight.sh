#!/bin/bash
set -eu
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
mkdir -p "$work/backlight" "$work/backlight1" "$work/app/config"
printf '70\n' > "$work/backlight/brightness"
printf '51\n' > "$work/backlight1/brightness"
export RGDS_SCREEN_BACKLIGHT_ROOT=$work
. "$(dirname "$0")/../screen-control.sh"
app_dir="$work/app"
printf '0\n' > "$app_dir/config/top-backlight.txt"
printf 'off\n' > "$app_dir/config/screen-mode.txt"
rgds_screens_save
rgds_screens_apply
[ "$(cat "$work/backlight/brightness")" = 70 ]
[ "$SCUMMVM_RGDS_UPPER_BACKLIGHT" = "$work/backlight/brightness" ]
[ "$SCUMMVM_RGDS_LOWER_BACKLIGHT" = "$work/backlight1/brightness" ]
[ "$(cat "$work/backlight1/brightness")" = 51 ]
rgds_screens_restore
[ "$(cat "$work/backlight/brightness")" = 70 ]
printf '1\n' > "$app_dir/config/top-backlight.txt"
rgds_screens_save
rgds_screens_apply
[ "$(cat "$work/backlight/brightness")" = 70 ]
[ "$(cat "$work/backlight1/brightness")" = 51 ]
[ "$SCUMMVM_RGDS_UPPER_BACKLIGHT" = "$work/backlight1/brightness" ]
[ "$SCUMMVM_RGDS_LOWER_BACKLIGHT" = "$work/backlight/brightness" ]
rgds_screens_restore
printf 'mirror\n' > "$app_dir/config/screen-mode.txt"
printf '0\n' > "$work/backlight1/brightness"
rgds_screens_save
rgds_screens_apply
[ "$(cat "$work/backlight1/brightness")" = 70 ]
rgds_screens_restore
[ "$(cat "$work/backlight1/brightness")" = 0 ]
printf '5\n' > "$app_dir/config/top-backlight.txt"
if rgds_screens_apply; then exit 1; fi
printf 'Backlight mode/mapping/restore tests PASS\n'
