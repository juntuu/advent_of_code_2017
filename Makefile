done := 25
days := $(shell seq -w 1 $(done))

.PHONY: all $(days) clean

all: $(days)

$(days):
	@ $(MAKE) -C day_$@ a.out
	- cd day_$@ && time ./a.out < input.txt

clean:
	rm -f day_*/a.out
