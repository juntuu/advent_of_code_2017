#!/usr/bin/awk -f

function value(v, reg) { return v ~ /^-?[0-9]+$/ ? v : 0+reg[v] }

function q_empty(q) { return q["head"] == q["tail"] }
function q_last(q) { return q[q["tail"]-1] }
function q_push(q, v) { q[q["tail"]++] = v }
function q_pop(q, _v, _i) {
	_v = q[_i = q["head"]++]
	delete q[_i]
	return _v
}

function run(pc, snd, rcv, reg, _i, _x, _y) {
	while (_i = code[++pc]) {
		_x = code[pc,1]
		_y = code[pc,2]
		if (_i == "rcv") {
			if (q_empty(rcv))
				return pc - 1
			reg[_x] = q_pop(rcv)
		}
		if (_i == "snd") q_push(snd, value(_x, reg))
		if (_i == "set") reg[_x] = value(_y, reg)
		if (_i == "add") reg[_x] += value(_y, reg)
		if (_i == "mul") reg[_x] *= value(_y, reg)
		if (_i == "mod") reg[_x] %= value(_y, reg)
		if (_i == "jgz") value(_x, reg) > 0 && pc += value(_y, reg) - 1
	}
}

{
	code[NR] = $1
	code[NR,1] = $2
	code[NR,2] = $3
}

END {
	run(0, sound)
	print q_last(sound)

	rb["p"] = 1
	do {
		a = run(a, qb, qa, ra)
		b = run(b, qa, qb, rb)
	} while (!q_empty(qa))
	print qa["tail"]
}
