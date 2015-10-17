#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG

static int Q = 10;
static int K1 = 2;
static int K2 = 3;
static int G = 3;

static int *cells;
static int *prev_cells;

static int segment_size;

static int *get_neighbour_counts_offsets;

void get_neighbour_counts(int ij, int *infected, int *ill, int *sum);

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    srand(world_rank);

    if (argc != 3) {
        if (world_rank == 0) {
            fprintf(stderr, "Usage: %s <size> <steps>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    int size = atoi(argv[1]);
    int steps = atoi(argv[2]);

    segment_size = size / world_size;
    if (size % world_size != 0) {
        segment_size++;
    }
    int start_offset = world_rank * segment_size;

    if (start_offset + segment_size > size) {
        segment_size -= size - (start_offset + segment_size);
    }

    prev_cells = (int*) malloc(sizeof(int)*size*segment_size);
    cells = (int*) malloc(sizeof(int)*size*segment_size);

    {
        int offsets[] = {-1, 1, size-1, size, size+1, -size-1, -size, -size+1};
        get_neighbour_counts_offsets = (int*) malloc(sizeof(offsets));
        memcpy(get_neighbour_counts_offsets, offsets, sizeof(offsets));
    }

    for (int i = 0; i < size * segment_size; i++) {
        prev_cells[i] = rand() % (Q+1);
        cells[i] = 0;
    }

    for (int i = 1; i < segment_size - 1; i++) {
        for (int j = 0; j < size; j++) {
            int ij = i * size + j;
            if (prev_cells[ij] == Q) {
                cells[ij] = 0;
            } else if (prev_cells[ij] == 0) {
                int infected = 0;
                int ill = 0;
                int sum = 0;
                get_neighbour_counts(ij, &infected, &ill, &sum);
                cells[ij] = infected / K1 + ill / K2;
            } else {
                int infected = 0;
                int ill = 0;
                int sum = 0;
                get_neighbour_counts(ij, &infected, &ill, &sum);
                cells[ij] = sum / (infected + ill + 1) + G;
            }
        }
    }

    for (int i = 1; i < segment_size - 1; i++) {
        for (int j = 0; j < size; j++) {
            int ij = i * size + j;
            printf("%d %d %d %d %d\n", world_rank, i, j, prev_cells[ij], cells[ij]);
        }
    }

    free(cells);
    free(prev_cells);
    free(get_neighbour_counts_offsets);
    MPI_Finalize();
    return 0;
}

void get_neighbour_counts(int ij, int *infected, int *ill, int *sum) {
    for (int i = 0; i < 8; i++) {
        int ij_neighbour = ij + get_neighbour_counts_offsets[i];
        if (prev_cells[ij_neighbour] == Q) {
            (*ill)++;
        } else if (prev_cells[ij_neighbour] != 0) {
            (*infected)++;
        }
        *sum += prev_cells[ij_neighbour];
    }
}
