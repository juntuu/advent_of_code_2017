function ant(puzzle::Dict{Complex,Int}, at::Complex, bursts::Int, update::Function)::Int
	d::Complex = -1
	count::Int = 0
	for _ in 1:bursts
		status = get(puzzle, at, 0)
		if status == 0
			d *= im
		elseif status == 2
			d *= -im
		elseif status == 3
			d *= im * im
		end
		puzzle[at] = status = update(status)
		count += status == 2
		at += d
	end
	count
end

part1(status::Int)::Int = status == 0 ? 2 : 0
part2(status::Int)::Int = (status + 1) % 4

function main()
	lines = readlines()
	w = length(lines)
	h = length(lines[1])
	grid = Dict{Complex,Int}()
	for (i, line) in enumerate(lines)
		for (j, c) in enumerate(line)
			c == '#' && setindex!(grid, 2, i + j*im)
		end
	end

	mid = complex(1+div(w, 2), 1+div(h, 2))
	println(ant(copy(grid), mid, 10000, part1))
	println(ant(grid, mid, 10000000, part2))
end

main()
