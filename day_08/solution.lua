
local ops = {
	["inc"] = function(a, b) return a + b end,
	["dec"] = function(a, b) return a - b end,
	["<"] = function(a, b) return a < b end,
	[">"] = function(a, b) return a > b end,
	["<="] = function(a, b) return a <= b end,
	[">="] = function(a, b) return a >= b end,
	["=="] = function(a, b) return a == b end,
	["!="] = function(a, b) return a ~= b end,
}

function run(regs, reg, op, val)
	local fn = ops[op]
	local rval = regs[reg]
	if rval == nil then
		rval = 0
	end
	return fn(rval, val)
end

local max_ever = 0
local registers = {}
local pattern = "(%a+) +(%a+) +([-]?%d+) +if +(%a+) +([!><=]+) +([-]?%d+)"
for line in io.lines() do
	local reg, op, x, guard_reg, cmp, y = string.match(line, pattern)
	if run(registers, guard_reg, cmp, tonumber(y)) then
		local val = run(registers, reg, op, tonumber(x))
		registers[reg] = val
		if val > max_ever then
			max_ever = val
		end
	end
end

local max = nil
for key, value in pairs(registers) do
	if not max or max < value then
		max = value
	end
end
print(max)
print(max_ever)
