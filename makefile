CFLAGS=-Wall -Werror -std=c18

all: sample inf args ssi

ssi: ssi.c
	gcc $(CFLAGS) -o ssi ssi.c

sample: sample.c
	gcc sample.c -lreadline -lhistory -ltermcap -o sample

inf: inf.c
	gcc inf.c -o inf

args: args.c
	gcc args.c -o args

clean:
	rm -f *.o
	rm -f sample
	rm -f inf
	rm -f args
	rm -f ssi
