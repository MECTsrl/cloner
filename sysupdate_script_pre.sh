#!/bin/sh

# Clean up dos2unix temp file.
sync

test -n "$0" || exit 1

FROMWD="$(pwd)"

WORKDIR="$(dirname $0)"
CLONERDIR="$WORKDIR/bin"

trap cleanup EXIT
cleanup()
{
    cd "$FROMWD"
    rm -rf "$CLONERDIR"

    sync
}

cd "$WORKDIR"

