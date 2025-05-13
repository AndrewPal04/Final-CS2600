#!/bin/bash

PLAYER="O"
TOPIC="game/move/player2"
BROKER="localhost"

board=$(/usr/bin/mosquitto_sub -h "$BROKER" -t game/board -C 1 -W 2)

if [ -z "$board" ]; then
    exit 1
fi

board=${board:0:9}

empty_positions=()
for i in {0..8}; do
    char="${board:$i:1}"
    if [[ "$char" != "X" && "$char" != "O" ]]; then
        empty_positions+=($((i + 1)))
    fi
done

if [ ${#empty_positions[@]} -eq 0 ]; then
    echo "No moves left."
    exit 0
fi

RANDOM_INDEX=$((RANDOM % ${#empty_positions[@]}))
move=${empty_positions[$RANDOM_INDEX]}

/usr/bin/mosquitto_pub -h "$BROKER" -t "$TOPIC" -m "$move"

