#!/bin/bash
set -u
app=$(CDPATH= cd -P "$(dirname "$0")/.." && pwd) || exit 1
parent=$(dirname "$app")
mkdir -p "$app/config" || exit 1
if [ -n "${SCUMMVM_RGDS_LOG_FILE:-}" ]; then
    log="$SCUMMVM_RGDS_LOG_FILE"
    exec >> "$log" 2>&1
else
    log="$app/config/ScummVM_Calibracao.txt"
    exec > "$log" 2>&1
fi
printf '\n=== Calibracao: %s ===\n' "$(date '+%Y%m%d-%H%M%S')"
export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/var/run}"
export WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-wayland-0}"
export SDL_VIDEODRIVER=wayland
export SDL_AUDIODRIVER=alsa
export SDL_RENDER_DRIVER=software
export SCUMMVM_RGDS_STOCK_VOLUME=1
export SCUMMVM_RGDS_DISPLAY=1
export SCUMMVM_RGDS_TOUCH_DEVICE=/dev/input/by-path/platform-fe5e0000.i2c-event
export SCUMMVM_RGDS_CALIBRATE="$app/config/touch-calibration.txt"
export SCUMMVM_RGDS_TOUCH_CALIBRATION=''
export LD_LIBRARY_PATH="$app/lib:/usr/lib:/lib:/mnt/vendor/lib"
unset LD_PRELOAD
. "$app/touch-control.sh" || exit 1
trap rgds_touch_restore EXIT
rgds_touch_enable /sys/class/anbernic_misc/tpctrl || exit 1
chmod +x "$app/scummvm" || exit 1
cd "$app" || exit 1
./scummvm --config="$app/config/calibration-session.ini" --fullscreen --themepath="$app/data" --extrapath="$app/data" &
child=$!
(
    sleep 180 &
    sleeper=$!
    trap 'kill -TERM "$sleeper" 2>/dev/null; exit 0' INT TERM HUP
    wait "$sleeper"
    printf '%s\n' 'Limite de tres minutos atingido.'
    kill -TERM "$child" 2>/dev/null
) &
watchdog=$!
trap 'kill -TERM "$child" "$watchdog" 2>/dev/null' INT TERM HUP
wait "$child"
result=$?
kill -TERM "$watchdog" 2>/dev/null
wait "$watchdog" 2>/dev/null
printf 'Codigo de saida: %s\n' "$result"
exit "$result"
