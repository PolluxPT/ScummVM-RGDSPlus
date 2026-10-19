#!/bin/bash
# Portable ScummVM test build. All app data stays next to this launcher on TF2.
set -u
app_parent=$(CDPATH= cd -P "$(dirname "$0")" 2>/dev/null && pwd) || exit 1
app_dir="$app_parent/ScummVM_RGDSPlus"
stamp=$(date '+%Y%m%d-%H%M%S' 2>/dev/null)
log="$app_dir/ScummVM_Arranque.txt"
export SCUMMVM_RGDS_LOG_FILE="$log"
umask 077
# Keep existing timestamped reports inside the app, without deleting user files.
mkdir -p "$app_dir/config" || exit 1
shopt -s nullglob
old_reports=("$app_parent"/ScummVM_Arranque_*.txt "$app_parent"/ScummVM_Calibracao_*.txt)
if [ -f "$app_parent/ScummVM_Arranque.txt" ]; then
    old_reports+=("$app_parent/ScummVM_Arranque.txt")
fi
if [ "${#old_reports[@]}" -gt 0 ]; then
    mkdir -p "$app_dir/config/logs-anteriores" || exit 1
    for old_report in "${old_reports[@]}"; do
        archive="$app_dir/config/logs-anteriores/$(basename "$old_report")"
        [ -e "$archive" ] && archive="$archive.$$"
        mv "$old_report" "$archive" || exit 1
    done
fi
shopt -u nullglob
: > "$log" || exit 1
exec >> "$log" 2>&1
printf 'ScummVM RG DS Plus 2026.4.0 - Versao final 4 - Stock RG DS Plus / registo unico\nData: %s\n' "$stamp"
printf 'Pasta: %s\n' "$app_dir"
printf 'RGDS volume: curve=quadratic ceiling=0.35; gain=0.35*(level/10)^2\n'
if [ ! -f "$app_dir/scummvm" ]; then
    printf '%s\n' 'Falta a pasta ScummVM_RGDSPlus junto de ScummVM.sh.'
    exit 1
fi
case "$(uname -m)" in
    aarch64) ;;
    *) printf '%s\n' 'Este executavel requer Linux ARM de 64 bits.'; exit 1 ;;
esac
if ! mkdir -p "$app_dir/config" "$app_dir/saves" "$app_dir/games" "$app_dir/cache"; then
    printf '%s\n' 'Nao foi possivel criar as pastas de dados no TF2.'
    exit 1
fi
chmod +x "$app_dir/scummvm" || exit 1
export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/var/run}"
export WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-wayland-0}"
export SDL_VIDEODRIVER=wayland
export SDL_AUDIODRIVER=alsa
export SCUMMVM_RGDS_DISPLAY="${SCUMMVM_RGDS_DISPLAY:-1}"
if [ -z "${SCUMMVM_RGDS_TOUCH_DEVICE:-}" ]; then
    if [ -e /dev/input/by-path/platform-fe5e0000.i2c-event ]; then
        export SCUMMVM_RGDS_TOUCH_DEVICE=/dev/input/by-path/platform-fe5e0000.i2c-event
    else
        export SCUMMVM_RGDS_TOUCH_DEVICE=/dev/input/event1
    fi
fi
export XDG_CONFIG_HOME="$app_dir/config"
export XDG_CACHE_HOME="$app_dir/cache"
export SCUMMVM_RGDS_TOUCH_CALIBRATION="$app_dir/config/touch-calibration.txt"
# Firmware libraries only. Do not load NNDDSS's Android compatibility shims.
unset LD_PRELOAD
export LD_LIBRARY_PATH="$app_dir/lib:/usr/lib:/lib:/mnt/vendor/lib"
export SDL_MOUSE_TOUCH_EVENTS=0
export SDL_RENDER_DRIVER=software
export SCUMMVM_RGDS_STOCK_VOLUME=1
cfg="$app_dir/config/scummvm.ini"
if [ ! -f "$cfg" ]; then
    (set -C; cat > "$cfg" <<EOF
[scummvm]
fullscreen=true
gui_theme=scummremastered
gui_scale=125
gui_language=pt_PT
gfx_mode=default
themepath=$app_dir/data
pluginspath=$app_dir/plugins
aspect_ratio=true
filtering=false
joystick_num=0
touchpad_mouse_mode=false
savepath=$app_dir/saves
extrapath=$app_dir/data
vkeybdpath=$app_dir/data
vkeybd_pack_name=vkeybd_default
EOF
    ) || exit 1
fi
if [ ! -f "$app_dir/config/gui-2026-migrated" ]; then
    cp "$cfg" "$app_dir/config/scummvm-before-2026.ini" || exit 1
    awk -v data="$app_dir/data" -v plugins="$app_dir/plugins" '
        function options() {
            print "gui_theme=scummremastered";
            print "gui_scale=125";
            print "gui_language=pt_PT";
            print "gfx_mode=default";
            print "themepath=" data;
            print "extrapath=" data;
            print "pluginspath=" plugins;
        }
        /^\[/ {
            section=($0=="[scummvm]"); print;
            if(section) { found=1; options(); }
            next;
        }
        section && /^(gui_theme|gui_scale|gui_language|gfx_mode|themepath|extrapath|pluginspath)=/ { next; }
        { print; }
        END { if(!found) { print "[scummvm]"; options(); } }
    ' "$cfg" > "$cfg.new" || exit 1
    mv "$cfg.new" "$cfg" || exit 1
    printf 'ok\n' > "$app_dir/config/gui-2026-migrated" || exit 1
fi
theme_args=()
if [ -n "${SCUMMVM_RGDS_THEME:-}" ]; then
    theme_args=("--gui-theme=$SCUMMVM_RGDS_THEME")
fi
if ! awk '/^\[/ { section=($0=="[scummvm]") } section && /^audio_buffer_size=/ { found=1 } END { exit !found }' "$cfg"; then
    awk '/^\[scummvm\]$/ { print; print "audio_buffer_size=4096"; next } { print }' "$cfg" > "$cfg.new" && mv "$cfg.new" "$cfg" || exit 1
fi
printf '\n=== Estado do hardware (leitura) ===\n'
for item in /sys/class/anbernic_misc/openbor_volume /sys/class/anbernic_misc/lcdoff /sys/class/anbernic_misc/lcdswap /sys/class/anbernic_misc/brightness /sys/class/backlight/*/brightness /sys/class/backlight/*/max_brightness; do
    if [ -r "$item" ]; then printf '%s: ' "$item"; cat "$item"; fi
done
printf 'Wayland: %s/%s\nEcra pedido: %s\n' "$XDG_RUNTIME_DIR" "$WAYLAND_DISPLAY" "$SCUMMVM_RGDS_DISPLAY"
if [ ! -S "$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY" ]; then
    printf '%s\n' 'O socket Wayland nao esta disponivel neste contexto do menu APPS.'
    exit 1
fi
printf '\n=== Bibliotecas ===\n'
if [ -x /lib/ld-linux-aarch64.so.1 ]; then
    /lib/ld-linux-aarch64.so.1 --list "$app_dir/scummvm" || exit 1
fi
cd "$app_dir" || exit 1
printf '\n=== Versao ===\n'
./scummvm --config="$cfg" --version || exit 1
. "$app_dir/touch-control.sh" || exit 1
. "$app_dir/tools/screen-control.sh" || exit 1
rgds_cleanup() { rgds_screens_restore; rgds_touch_restore; }
trap rgds_cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM HUP
rgds_touch_enable /sys/class/anbernic_misc/tpctrl || exit 1
rgds_screens_save || exit 1
# Verified on RG DS Plus Stock V1.0 (20260915): backlight1 is the upper panel.
# Seed/correct this hardware profile before the chooser; no first-run dim probe.
printf '1\n' > "$app_dir/config/top-backlight.txt.new" || exit 1
mv "$app_dir/config/top-backlight.txt.new" "$app_dir/config/top-backlight.txt" || exit 1
printf 'RGDS screens: verified Stock mapping upper=backlight1 lower=backlight\n'
chmod +x "$app_dir/tools/rgds-screen-menu" || exit 1
"$app_dir/tools/rgds-screen-menu" "$app_dir"
screen_result=$?
[ "$screen_result" = 2 ] && exit 0
[ "$screen_result" = 0 ] || exit 1
rgds_screens_apply || exit 1
rgds_run_scummvm() {
    if [ "$rgds_screen_mode" = mirror ]; then
        env LD_PRELOAD="$app_dir/lib/librgds-screen.so" ./scummvm "$@"
    else
        env LD_PRELOAD="$app_dir/lib/librgds-lower.so" ./scummvm "$@"
    fi
}
# Missing calibration or an explicit GUI request starts a separate raw session.
rgds_calibrate_if_needed() {
    if [ ! -s "$SCUMMVM_RGDS_TOUCH_CALIBRATION" ] || [ -f "$SCUMMVM_RGDS_TOUCH_CALIBRATION.recalibrate" ]; then
        printf '\n=== Calibracao do toque ===\n'
        /bin/bash "$app_dir/tools/calibrate.sh"
        calibration_result=$?
        if [ "$calibration_result" -ne 0 ] || [ ! -s "$SCUMMVM_RGDS_TOUCH_CALIBRATION" ]; then
            printf '%s\n' 'Calibracao nao guardada. Volta a abrir ScummVM para repetir.'
            return 1
        fi
        rm -f "$SCUMMVM_RGDS_TOUCH_CALIBRATION.recalibrate" || return 1
    fi
}
rgds_calibrate_if_needed || exit 1
printf '\n=== Interface ===\n'
if [ "${SCUMMVM_RGDS_TEST:-0}" = 1 ]; then
    # Bounded first test: only this child is signalled, never Weston/the launcher.
    rgds_run_scummvm --config="$cfg" --savepath="$app_dir/saves" --extrapath="$app_dir/data" --fullscreen "${theme_args[@]}" &
    child=$!
    (
        sleep 120 &
        sleeper=$!
        trap 'kill -TERM "$sleeper" 2>/dev/null; exit 0' INT TERM HUP
        wait "$sleeper"
        if kill -0 "$child" 2>/dev/null; then
            printf '\nTeste: limite de 120 segundos atingido.\n'
            kill -TERM "$child" 2>/dev/null
            sleep 5 &
            sleeper=$!
            wait "$sleeper"
            kill -KILL "$child" 2>/dev/null
        fi
    ) &
    watchdog=$!
    trap 'kill -TERM "$child" "$watchdog" 2>/dev/null' INT TERM HUP
    wait "$child"
    result=$?
    kill -TERM "$watchdog" 2>/dev/null
    wait "$watchdog" 2>/dev/null
    trap - INT TERM HUP
else
    while :; do
        rgds_run_scummvm --config="$cfg" --savepath="$app_dir/saves" --extrapath="$app_dir/data" --fullscreen "${theme_args[@]}"
        result=$?
        [ -f "$SCUMMVM_RGDS_TOUCH_CALIBRATION.recalibrate" ] || break
        rgds_calibrate_if_needed || exit 1
    done
fi
printf '\nCodigo de saida: %s\n' "$result"
exit "$result"
