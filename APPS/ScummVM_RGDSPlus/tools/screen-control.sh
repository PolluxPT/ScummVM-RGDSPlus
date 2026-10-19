#!/bin/bash
# Save both known stock backlights before the menu, restore at exit.
rgds_screen_saved0=''
rgds_screen_saved1=''
rgds_screen_guard=''
rgds_screen_active=0
rgds_screen_root=${RGDS_SCREEN_BACKLIGHT_ROOT:-/sys/class/backlight}
rgds_screens_save() {
    local path value index=0
    for path in "$rgds_screen_root/backlight/brightness" "$rgds_screen_root/backlight1/brightness"; do
        [ -r "$path" ] && [ -w "$path" ] || return 1
        value=$(cat "$path") || return 1
        case "$value" in ''|*[!0-9]*) return 1;; esac
        [ "$value" -le 255 ] || return 1
        printf -v "rgds_screen_saved$index" '%s' "$value"
        index=$((index + 1))
    done
    rgds_screen_active=1
    # Survives an abrupt launcher termination; never kills stock processes.
    (
        trap '' HUP
        while kill -0 "$$" 2>/dev/null; do sleep 1; done
        printf '%s\n' "$rgds_screen_saved0" > "$rgds_screen_root/backlight/brightness"
        printf '%s\n' "$rgds_screen_saved1" > "$rgds_screen_root/backlight1/brightness"
    ) &
    rgds_screen_guard=$!
}
rgds_screens_restore() {
    if [ "$rgds_screen_active" = 1 ]; then
        printf '%s\n' "$rgds_screen_saved0" > "$rgds_screen_root/backlight/brightness"
        printf '%s\n' "$rgds_screen_saved1" > "$rgds_screen_root/backlight1/brightness"
        rgds_screen_active=0
        printf 'RGDS screens: restored brightness %s / %s\n' "$rgds_screen_saved0" "$rgds_screen_saved1"
    fi
    if [ -n "$rgds_screen_guard" ]; then kill -TERM "$rgds_screen_guard" 2>/dev/null; wait "$rgds_screen_guard" 2>/dev/null || :; rgds_screen_guard=''; fi
}
rgds_screens_apply() {
    local top mode path brightness
    top=$(cat "$app_dir/config/top-backlight.txt") || return 1
    case "$top" in 0) path="$rgds_screen_root/backlight/brightness"; brightness=$rgds_screen_saved0;; 1) path="$rgds_screen_root/backlight1/brightness"; brightness=$rgds_screen_saved1;; *) return 1;; esac
    mode=$(cat "$app_dir/config/screen-mode.txt") || return 1
    case "$mode" in
        off)
            export SCUMMVM_RGDS_UPPER_BACKLIGHT="$path"
            if [ "$top" = 0 ]; then
                export SCUMMVM_RGDS_LOWER_BACKLIGHT="$rgds_screen_root/backlight1/brightness"
            else
                export SCUMMVM_RGDS_LOWER_BACKLIGHT="$rgds_screen_root/backlight/brightness"
            fi
            printf 'RGDS screens: lower-only lighting deferred until active game surfaces\n';;
        mirror)
            if [ "$brightness" = 0 ]; then
                if [ "$top" = 0 ]; then brightness=$rgds_screen_saved1; else brightness=$rgds_screen_saved0; fi
                [ "$brightness" -gt 0 ] || brightness=70
                printf '%s\n' "$brightness" > "$path" || return 1
            fi;;
        *) return 1;;
    esac
    printf 'RGDS screens: mode=%s upper=%s\n' "$mode" "$path"
    rgds_screen_mode=$mode
}
