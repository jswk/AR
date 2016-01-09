
all: merge_rand merge_file

%: %.c
	mpicc -o $@ $< -Wall -lm -std=c99

clean:
	rm -f merge_rand merge_file
