#!/bin/sh -ex

NOTIFY_ARGS=$DEFAULT_NOTIFY_ARGS
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
NOTIFY_SEND_PATH=${NOTIFY_SEND_PATH:-~/.config/notify}
if [ -d "$NOTIFY_SEND_PATH" ]; then
    cd $NOTIFY_SEND_PATH
    for cmd in "$NOTIFY_SEND_PATH"/*.sh; do
        [ -r $cmd ] && . $cmd
    done
else
    notify $NOTIFY_ARGS "$*"
fi