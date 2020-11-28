package main

import "fmt"

func highIndex(slice []int) int {
	max := 0
	for i, e := range slice {
		if e > slice[max] {
			max = i
		}
	}
	return max
}

func distribute(mem []int, i int) {
	val := mem[i]
	mem[i] = 0
	for val > 0 {
		i = (i + 1) % len(mem)
		mem[i]++
		val--
	}
}

func findCycle(mem [16]int) (int, [16]int) {
	seen := map[[16]int]bool{}
	steps := 0
	for !seen[mem] {
		seen[mem] = true
		distribute(mem[:], highIndex(mem[:]))
		steps++
	}
	return steps, mem
}

func main() {
	memory := [16]int{}
	for i := range memory {
		_, err := fmt.Scan(&memory[i])
		if err != nil {
			panic(err)
		}
	}
	p1, memory := findCycle(memory)
	fmt.Println(p1)
	p2, _ := findCycle(memory)
	fmt.Println(p2)
}
