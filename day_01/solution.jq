def zip(a; b): [a, b] | transpose;

def sum_eq(a; b):
	[zip(a; b)[] | select(.[0] == .[1])[0]]
	| add
;

def part1: sum_eq(.; .[1:] + .[:1]);

def part2:
	(length / 2) as $half
	| 2 * sum_eq(.[:$half]; .[$half:])
;

split("")
| map(tonumber)
| (part1, part2)
