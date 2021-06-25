#!/bin/sh

while [ "$#" -gt 0 ]; do
    case "$1" in
        -h)
            export HINT=$2
            IFS=: read -r _ label value <<EOF
$2
EOF
            if [ "$label" = "x-canonical-private-synchronous" ]; then
                export NOTIFICATION_ID="$value"
                NOTIFY_ARGS="$NOTIFY_ARGS -r $value"
            fi
            ;;
        -r)
            export NOTIFICATION_ID="$2"
            NOTIFY_ARGS="$NOTIFY_ARGS -r $2"
            ;;
        -a)
            export APPNAME=$2
            ;;
        -t)
            export TIMEOUT=$2
            NOTIFY_ARGS="$NOTIFY_ARGS -t $2"
            ;;
        -u)
            export URGENCY=$2
            ;;
        -c)
            export CATEGORY=$2
            ;;
        *)
            break
            ;;
    esac
    shift 2
done
NOTIFY_SEND_DIR=${NOTIFY_SEND_DIR:-${XDG_CONFIG_DIR:-~/.config}/notify}/notify.d
if [ -d "$NOTIFY_SEND_DIR" ]; then
    for cmd in "$NOTIFY_SEND_DIR"/*.sh; do
        # shellcheck disable=SC2015
        [ -r "$cmd" ] && . "$cmd" || break
    done
else
    # shellcheck disable=SC2086
    notify $NOTIFY_ARGS "$@"
fi
