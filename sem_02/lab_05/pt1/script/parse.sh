#! /bin/bash

# res=$(grep prio out.res | awk '{if ("-" != $27) print $9 $27; else print $9 $28}' | sed "s/,/ /g" | awk '{printf "\"%s\": %s, ", $1, $2}')
# res=$(cat out.res | grep prio | sed -E "s/Apr 25 11:30:57 LAPTOP-813EJBL kernel: \+ //" | awk -F "(,| - )" '{printf "\"%s\": %s, ", $2, $14}' | sed 's/\(.*\),/\1/')
# res=$(journalctl --since "$1" | grep \+.*prio | sed -E "s/.*: \+ //" | awk -F "(,| - )" '{printf "\"%s\": %s, ", $2, $14}' | sed 's/\(.*\),/\1/')
echo "{$(journalctl --since "$1" | grep \+.*prio | sed -E "s/.*: \+ //" | awk -F "(,| - )" '{printf "\"%s\": %s, ", $2, $14}' | sed 's/\(.*\),/\1/')}"
# echo "{$res}"

