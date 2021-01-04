#!/usr/bin/awk -f

{ gsub(/[.:]/, "") }

/Begin in state/ { start = $NF }
/Perform a diagnostic/ { steps = $(NF-1) }

/In state/ { st = $NF }
/If the current value is/ { val = $NF }
/Write the value/ { state[st,val,"write"] = $NF }
/Move one slot to the/ { state[st,val,"move"] = $NF == "left" ? -1 : 1 }
/Continue with state/ { state[st,val,"continue"] = $NF }

END {
	at = 0
	while (steps--) {
		val = 0+tape[at]
		tape[at] = state[start,val,"write"]
		at += state[start,val,"move"]
		start = state[start,val,"continue"]
	}
	for (k in tape) sum += tape[k]
	print sum
}
