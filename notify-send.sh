#!/bin/sh

while [ "$#" -gt 0 ]; do
    case "$1" in
        --)
            shift
            break
            ;;
        -a)
            export NOTIFY_APPNAME=$2
            ;;
        -c)
            export NOTIFY_CATEGORY=$2
            ;;
        -h)
            export NOTIFY_HINT=$2
            IFS=: read -r _ label value <<EOF
$2
EOF
            if [ "$label" = "x-canonical-private-synchronous" ]; then
                export NOTIFY_MSG_ID="$value"
            fi
            ;;
        -r)
            export NOTIFY_MSG_ID="$2"
            ;;
        -t)
            export NOTIFY_TIMEOUT=$2
            ;;
        -u)
            export NOTIFY_URGENCY=$2
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
    exec notify "$@"
fi
