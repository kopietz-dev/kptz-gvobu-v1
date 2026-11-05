#!/bin/sh

DEVICE="/dev/sda1"
UF2_FILE="$1"

MOUNT_POINT=$(udisksctl mount -b "$DEVICE" | awk -F' at ' '{print $2}' | tr -d '.')

[ -z "$MOUNT_POINT" ] && exit 1

cp "$UF2_FILE" "$MOUNT_POINT/"
sync
