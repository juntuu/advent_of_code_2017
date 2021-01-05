import sys
from collections import Counter


def apply(v, fs):
    for x in fs:
        v = x(v)
    return v


def step(g):
    size = 2 + len(g) % 2
    group = lambda x: zip(*[iter(x)] * size)
    gs = []
    for x, *xs in group(g):
        xs = [group(r) for r in xs]
        gs.append([["".join(i) for i in ((y, *map(next, xs)))] for y in group(x)])

    for i, row in enumerate(gs):
        for j, o in enumerate(row):
            gs[i][j] = RULES[tuple(o)]

    if len(g) == 6:
        return [tuple(i) for e in gs for i in e]

    lines = []
    for x, *xs in gs:
        xs = [iter(x) for x in xs]
        for s in x:
            lines.append(s + "".join(map(next, xs)))
    return lines


def two_more_step(xs) -> int:
    total = 0
    for s, c in xs.items():
        s = step(step(s))
        total += c * "".join(s).count("#")
    return total


def three_step(xs):
    out = Counter()
    for s, c in xs.items():
        for block in step(step(step(s))):
            out[block] += c
    return out


p = lambda x: list(map("".join, x))
f = lambda x: p(map(reversed, x))
r = lambda x: p(map(reversed, zip(*x)))

OPS = [(), (r,), (r, r), (r, r, r)]
OPS += [i + (f,) for i in OPS]

RULES = {
    tuple(apply(x, o)): v
    for x, v in (
        [i.split("/") for i in line.strip().split(" => ")] for line in sys.stdin
    )
    for o in OPS
}

START = (
    ".#.",
    "..#",
    "###",
)

xs = three_step(Counter([START]))
print(two_more_step(xs))
xs = apply(xs, [three_step] * 5)
print(sum("".join(s).count("#") * c for s, c in xs.items()))
