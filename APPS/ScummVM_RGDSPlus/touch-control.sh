#!/bin/bash
# Mirror the stock NNDDSS runtime touch switch and restore its previous value.
rgds_touch_node=''
rgds_touch_saved=''
rgds_touch_enabled=0
rgds_touch_enable() {
    rgds_touch_node="$1"
    if [ ! -r "$rgds_touch_node" ] || [ ! -w "$rgds_touch_node" ]; then
        printf '%s\n' 'RGDS tpctrl: controlo de touch indisponivel.'
        return 1
    fi
    rgds_touch_saved=$(cat "$rgds_touch_node") || return 1
    case "$rgds_touch_saved" in
        0|1) ;;
        *) printf 'RGDS tpctrl: valor inesperado: %s\n' "$rgds_touch_saved"; rgds_touch_saved=''; return 1 ;;
    esac
    printf 'RGDS tpctrl: valor anterior=%s\n' "$rgds_touch_saved"
    # Mark before writing so the EXIT trap can restore on any later failure.
    rgds_touch_enabled=1
    if ! printf '0\n' > "$rgds_touch_node"; then
        printf '%s\n' 'RGDS tpctrl: falha ao ativar o modo usado pelo NNDDSS.'
        return 1
    fi
    rgds_touch_current=$(cat "$rgds_touch_node") || return 1
    printf 'RGDS tpctrl: valor durante ScummVM=%s\n' "$rgds_touch_current"
    [ "$rgds_touch_current" = 0 ]
}
rgds_touch_restore() {
    if [ "$rgds_touch_enabled" = 1 ] && [ -n "$rgds_touch_saved" ]; then
        if printf '%s\n' "$rgds_touch_saved" > "$rgds_touch_node"; then
            printf 'RGDS tpctrl: valor reposto=%s\n' "$rgds_touch_saved"
        else
            printf '%s\n' 'RGDS tpctrl: nao foi possivel repor o valor anterior.'
        fi
        rgds_touch_enabled=0
    fi
}
