#!/bin/bash

set -e

read -r KEY

N=128

knot_hashes() {
	printf '%s\n' "$KEY-"{0..127} |
		LC_ALL=C Rscript ../day_10/solution.r |
		tr 'a-f' 'A-F'
}

grid() {
	{ echo "obase=2; ibase=16;"; knot_hashes; } |
		BC_LINE_LENGTH=130 bc |
		while read -r line
		do printf '%0128s\n' "$line"
		done
}

pop() {
	[[ $1 -lt 0 ]] || [[ $1 -ge $N ]] || [[ $2 -lt 0 ]] || [[ $2 -ge $N ]] && return 1
	key=$(($1 * N + $2))
	[[ ${GRID[key]} ]] || return 1
	unset "GRID[key]"
	pop $(($1 + 1)) "$2"
	pop $(($1 - 1)) "$2"
	pop "$1" $(($2 + 1))
	pop "$1" $(($2 - 1))
	return 0
}

row=0
while read -r line
do
	for col in {0..127}
	do
		[[ ${line:$col:1} = 1 ]] && GRID[$row + $col]=1
	done
	row=$((row + N))
done < <(grid)

echo ${#GRID[@]}

count=0
for key in "${!GRID[@]}"
do
	pop $((key / N)) $((key % N)) && count=$((count + 1))
done

echo $count
