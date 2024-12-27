#! /bin/bash

echo "[$(journalctl --since "$1" \
         | grep \+.*prio \
         | sed -E "s/.*: \+ //" \
         | awk -F "(, | - )" '{printf "{"; for (i=1; NF > i; i += 2) if (NF - 1 == i) printf "\"%s\":\"%s\"", $i, $(i + 1); else printf "\"%s\":\"%s\",", $i, $(i + 1); printf "},"}' \
         | sed 's/\(.*\),/\1/')]"

