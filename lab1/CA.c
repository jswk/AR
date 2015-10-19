#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*#define DEBUG*/

static int Q = 10;
static int K1 = 2;
static int K2 = 3;
static int G = 3;

static int world_rank;
static int world_size;

static int *cells;
static int *prev_cells;

static int *cells_left;
static int *cells_right;

static int size;
static int segment_size;

void get_neighbour_counts(int i, int j, int *infected, int *ill, int *sum);

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    srand(1+world_rank);

    if (argc != 3) {
        if (world_rank == 0) {
            fprintf(stderr, "Usage: %s <size> <steps>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    size = atoi(argv[1]);
    int steps = atoi(argv[2]);

    segment_size = size / world_size;
    if (size % world_size != 0) {
        segment_size++;
    }
    int start_offset = world_rank * segment_size;

    if (start_offset + segment_size > size) {
        segment_size -= size - (start_offset + segment_size);
    }

    cells_left  = (int*) malloc(sizeof(int)*size);
    cells_right = (int*) malloc(sizeof(int)*size);
    for (int i = 0; i < size; i++) {
        cells_left[i]  = 0;
        cells_right[i] = 0;
    }

    prev_cells = (int*) malloc(sizeof(int)*size*segment_size);
    cells      = (int*) malloc(sizeof(int)*size*segment_size);

    for (int i = 0; i < size * segment_size; i++) {
        prev_cells[i] = rand() % (Q+1);
        cells[i] = 0;
    }

    double start = MPI_Wtime();

    for (int step = 0; step < steps; step++) {
        MPI_Request reqs[4];
        MPI_Status stat;
        if (world_rank != 0) {
            MPI_Irecv(cells_left, size, MPI_INT, world_rank-1, step, MPI_COMM_WORLD, &reqs[0]);
            MPI_Isend(prev_cells + 0, size, MPI_INT, world_rank-1, step, MPI_COMM_WORLD, &reqs[2]);
        }
        if (world_rank+1 != world_size) {
            MPI_Irecv(cells_right, size, MPI_INT, world_rank+1, step, MPI_COMM_WORLD, &reqs[1]);
            MPI_Isend(prev_cells + (segment_size-1)*size, size, MPI_INT, world_rank+1, step, MPI_COMM_WORLD, &reqs[3]);
        }

        if (world_rank != 0) {
            MPI_Wait(&reqs[0], &stat);
        }
        if (world_rank+1 != world_size) {
            MPI_Wait(&reqs[1], &stat);
        }

        for (int i = 0; i < segment_size; i++) {
            for (int j = 0; j < size; j++) {
                int ij = i * size + j;
                if (prev_cells[ij] == Q) {
                    cells[ij] = 0;
                } else if (prev_cells[ij] == 0) {
                    int infected = 0;
                    int ill = 0;
                    int sum = 0;
                    get_neighbour_counts(i, j, &infected, &ill, &sum);
                    cells[ij] = infected / K1 + ill / K2;
                } else {
                    int infected = 0;
                    int ill = 0;
                    int sum = prev_cells[ij];
                    get_neighbour_counts(i, j, &infected, &ill, &sum);
                    cells[ij] = sum / (infected + ill + 1) + G;
                }
                if (cells[ij] > 10) {
                    cells[ij] = 10;
                }
            }
        }

#ifdef DEBUG
        for (int i = 0; i < segment_size; i++) {
            for (int j = 0; j < size; j++) {
                int ij = i * size + j;
                printf("%d %d %d %d %d %d\n", step, world_rank, start_offset+i, j, prev_cells[ij], cells[ij]);
            }
        }
#endif

        if (world_rank != 0) {
            MPI_Wait(&reqs[2], &stat);
        }
        if (world_rank+1 != world_size) {
            MPI_Wait(&reqs[3], &stat);
        }

        int *tmp_cells = prev_cells;
        prev_cells = cells;
        cells = tmp_cells;
    }

    double end = MPI_Wtime();

    if (world_rank == 0) {
        double elapsed = end - start;
        fprintf(stderr, "elapsed = %lfs\n", elapsed);
    }

    free(cells_left);
    free(cells_right);
    free(cells);
    free(prev_cells);
    MPI_Finalize();
    return 0;
}

inline static void get_neighbour_counts_update(int val, int *infected, int *ill, int *sum) {
    if (val == Q) {
        (*ill)++;
    } else if (val != 0) {
        (*infected)++;
    }
    *sum += val;
}

void get_neighbour_counts(int i, int j, int *infected, int *ill, int *sum) {
    int *col;

    // cells to the left
    if (i != 0) {
        col = prev_cells + (i-1)*size;
    } else {
        col = cells_left;
    }
    if (i != 0 || world_rank != 0) {
        if (j != 0) {
            get_neighbour_counts_update(col[j-1], infected, ill, sum);
        }
        get_neighbour_counts_update(col[j], infected, ill, sum);
        if (j+1 != size) {
            get_neighbour_counts_update(col[j+1], infected, ill, sum);
        }
    }

    // cells in the same column
    col = prev_cells + i*size;
    if (j != 0) {
        get_neighbour_counts_update(col[j-1], infected, ill, sum);
    }
    if (j+1 != size) {
        get_neighbour_counts_update(col[j+1], infected, ill, sum);
    }

    // cells to the right
    if (i+1 != segment_size) {
        col = prev_cells + (i+1)*size;
    } else {
        col = cells_right;
    }
    if (i+1 != segment_size || world_rank+1 != world_size) {
        if (j != 0) {
            get_neighbour_counts_update(col[j-1], infected, ill, sum);
        }
        get_neighbour_counts_update(col[j], infected, ill, sum);
        if (j+1 != size) {
            get_neighbour_counts_update(col[j+1], infected, ill, sum);
        }
    }
}
