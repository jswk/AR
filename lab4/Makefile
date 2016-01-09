
all: merge_rand merge_file

%: %.c
	mpicc -o $@ $< -Wall -lm

clean:
	rm -f merge_rand merge_file
