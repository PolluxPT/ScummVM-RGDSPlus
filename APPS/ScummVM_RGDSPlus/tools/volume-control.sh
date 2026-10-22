#!/bin/bash
# Preserve the pre-launch Stock volume; in-session buttons still use the core curve.
rgds_volume_node=''
rgds_volume_saved=''
rgds_volume_active=0
rgds_volume_save() {
    local value
    # Never replace the original snapshot during calibration or session restarts.
    [ "$rgds_volume_active" = 0 ] || return 0
    rgds_volume_node="$1"
    if [ ! -r "$rgds_volume_node" ] || [ ! -w "$rgds_volume_node" ]; then
        printf '%s\n' 'RGDS volume: cannot preserve Stock volume; node unavailable.'
        return 1
    fi
    value=$(cat "$rgds_volume_node") || return 1
    case "$value" in
        [0-9]|10) ;;
        *) printf 'RGDS volume: invalid initial level: %s\n' "$value"; return 1 ;;
    esac
    rgds_volume_saved="$value"
    rgds_volume_active=1
    printf 'RGDS volume: saved initial level=%s/10\n' "$rgds_volume_saved"
}
rgds_volume_restore() {
    local current
    [ "$rgds_volume_active" = 1 ] || return 0
    if ! printf '%s\n' "$rgds_volume_saved" > "$rgds_volume_node"; then
        printf '%s\n' 'RGDS volume: could not restore initial level.'
        return 1
    fi
    current=$(cat "$rgds_volume_node") || return 1
    if [ "$current" != "$rgds_volume_saved" ]; then
        printf 'RGDS volume: restore verification failed (expected=%s, read=%s).\n' "$rgds_volume_saved" "$current"
        return 1
    fi
    rgds_volume_active=0
    printf 'RGDS volume: restored initial level=%s/10\n' "$rgds_volume_saved"
}
