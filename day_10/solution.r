si <- file("stdin")
input <- readLines(si, n=1)
close(si)

max <- 256

I <- function(x) {
	x %% max + 1
}

reverse <- function(v, i, n) {
	while (n > 1) {
		x <- c(I(i), I(i+n-1))
		v[x[1:2]] <- v[x[2:1]]
		i <- i+1
		n <- n-2
	}
	v
}

rounds <- function(input, n) {
	v <- 0:(max-1)
	s <- 0
	i <- 0
	while (n > 0) {
		n <- n - 1
		for (x in input) {
			v <- reverse(v, i, x)
			i <- i + x + s
			s <- s + 1
		}
	}
	v
}

xor <- function(a) {
	x = 0
	for (e in a) x = bitwXor(x, e)
	x
}

condence <- function(v) {
	x <- c()
	i <- 1
	while (length(v) > i) {
		x <- c(x, xor(v[i:(i+15)]))
		i <- i + 16
	}
	x
}

numbers <- as.numeric(strsplit(input, ",")[[1]])
v <- rounds(numbers, 1)
write(prod(v[1:2]), "")

bytes <- c(charToRaw(input), 17, 31, 73, 47, 23)
v <- rounds(bytes, 64)
hash <- condence(v)
write(cat(format(as.hexmode(hash), width=2), sep=""), "")
