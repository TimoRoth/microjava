#!/bin/bash
set -e

CONVT="cat"
if [[ "$1" == "-c" ]]; then
    CONVT="$2"
    shift 2
fi

if [[ "$#" -lt "2" ]]; then
    echo "Usage: $0 [-c /path/to/classCvt] output input.class [input.class ...]"
    echo "If output ends in .c, a C array is generated. Otherwise a raw UJC file is written."
    exit -1
fi

TMPOUT="$(mktemp)"
INTERMED_D="$(mktemp -d)"
INTERMED_F="$INTERMED_D/workfile"
INTERMED_O="$INTERMED_D/tmpfile"
OUT="$1"
shift

function cleanup() {
    rm -rf "$TMPOUT" "$INTERMED"
}
trap cleanup EXIT

while [[ $# -gt 0 ]]; do
    INP="$1"
    shift

    "$CONVT" <"$INP" >"$INTERMED_F"
    FSIZE="$(stat -c%s "$INTERMED_F")"

    if [[ "$FSIZE" -gt 16777215 ]]; then
        echo "$INP is too large."
        exit -1
    fi

    BYTE1="$(( ( "$FSIZE" >> 16 ) & 255 ))"
    BYTE2="$(( ( "$FSIZE" >>  8 ) & 255 ))"
    BYTE3="$(( ( "$FSIZE" >>  0 ) & 255 ))"
    DATASTR="$(printf '\\x%0.2x\\x%0.2x\\x%0.2x' "$BYTE1" "$BYTE2" "$BYTE3")"
    printf "$DATASTR" >> "$INTERMED_O"
    cat "$INTERMED_F" >> "$INTERMED_O"
done

printf '\x00\x00\x00' >> "$INTERMED_O"

if [[ $OUT == *.c ]]; then
    pushd "$INTERMED_D" >/dev/null
    BNAME="$(basename "$OUT")"
    BNAME="ujc_${BNAME%.*}"
    mv "$INTERMED_O" "$BNAME"
    DATA="$(xxd -i "$BNAME")"
    popd >/dev/null
    printf '%s' "$DATA" > "$OUT"
else
    mv "$INTERMED_O" "$OUT"
fi
