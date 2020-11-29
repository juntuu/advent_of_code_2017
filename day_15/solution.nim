import strutils
import sequtils
import bitops

const MOD = 2147483647

proc gen(start, x, f: uint): iterator(): uint =
  if f == 0:
    return iterator(): uint =
      var val = start
      while true:
        val = (val * x) mod MOD
        yield bitand(val, 0xffff)
  return iterator(): uint =
    var val = start
    while true:
      val = (val * x) mod MOD
      if bitand(val, f) == 0:
        yield bitand(val, 0xffff)

proc next[T](it: iterator():T): T =
  for i in it:
    return i

proc zip[T](a, b: iterator():T): iterator(): array[0..1, T] =
  return iterator(): array[0..1, T] =
    while true:
      yield [next(a), next(b)]

iterator take[T](n: int, it: iterator():T): T =
  for _ in 0..n:
    yield next(it)

proc num_from_input(): uint =
  for part in readLine(stdin).rsplit(' '):
    return parseUint(part)

const A = 16807
const B = 48271
let a = num_from_input()
let b = num_from_input()

proc part1(): int =
  let ga = gen(a, A, 0)
  let gb = gen(b, B, 0)
  for x in take(40_000_000, zip(ga, gb)):
    if x[0] == x[1]:
      result += 1

proc part2(): int =
  let ga = gen(a, A, 0b11)
  let gb = gen(b, B, 0b111)
  for x in take(5_000_000, zip(ga, gb)):
    if x[0] == x[1]:
      result += 1

echo part1()
echo part2()
