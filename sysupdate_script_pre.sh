#!/bin/sh

test -n "$0" || exit 1

WORKDIR="$(dirname $0)/cloner"
FROMWD="$(pwd)"

trap cleanup EXIT
cleanup()
{
    cd "$FROMWD"
    rm -rf "$WORKDIR"

    sync
}

test -d "$WORKDIR" && rm -rf "$WORKDIR"
mkdir -p "$WORKDIR"
cd "$WORKDIR"

