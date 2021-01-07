import os
import strconv

fn part1(mut program []int) int {
	mut steps := 0
	for pc := 0; 0 <= pc && pc < program.len; steps++ {
		pc += program[pc]++
	}
	return steps
}

fn part2(mut program []int) int {
	mut steps := 0
	for pc := 0; 0 <= pc && pc < program.len; steps++ {
		if program[pc] >= 3 {
			pc += program[pc]--
		} else {
			pc += program[pc]++
		}
	}
	return steps
}

fn main() {
	mut s := os.get_lines().map(int(strconv.parse_int(it, 10, 0)))
	println(part1(mut s.clone()))
	println(part2(mut s))
}
