#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <string.h>

/*#define DEBUG*/
#define RS 100 // record size
#define KS 10 // key size

int nprocs,dim,myid,n,N; /* Cube size, dimension, & my node ID */

/* Sequential mergesort (either ascending or descending) */
void mergesort(char *list,int left,int right,int descending) {
    int i,j,k,t,middle;
    char *temp = list+2*n*RS;
    if (left < right) {
        middle = (left + right)/2;
        mergesort(list,left,middle,descending);
        mergesort(list,middle+1,right,descending);
        k = i = left; j = middle+1;
        if (descending) {
            while (i<=middle && j<=right) {
                int ij = memcmp(list+RS*i, list+RS*j, KS) > 0 ? i++ : j++;
                memmove(temp+(k++)*RS, list+RS*ij, RS);
                /*temp[k++] = list[i]>list[j] ? list[i++] : list[j++];*/
            }
        } else {
            while (i<=middle && j<=right) {
                int ij = memcmp(list+RS*i, list+RS*j, KS) < 0 ? i++ : j++;
                memmove(temp+(k++)*RS, list+RS*ij, RS);
                /*temp[k++] = list[i]<list[j] ? list[i++] : list[j++];*/
            }
        }
        t = i>middle ? j : i;
        /*while (k <= right) temp[k++] = list[t++];*/
        while (k <= right) memmove(temp+RS*(k++), list+RS*(t++), RS);
        /*for (k=left; k<=right; k++) list[k] = temp[k];*/
        memmove(list+left*RS, temp+left*RS, (right-left+1)*RS);
    }
}

/* Parallel mergesort */
void parallel_mergesort(int myid,char *list) {
    int l, m, bitl = 1, bitm, partner;
    MPI_Status status;
    mergesort(list,0,n-1,myid & bitl);
    for (l=1; l<=dim; l++) {
        bitl = 1 << l;
        bitm = 1 << (l-1);
        for (m=l-1; m>=0; m--) {
            partner = myid ^ bitm;
            MPI_Send(list,n*RS,MPI_BYTE,partner,l*dim+m,MPI_COMM_WORLD);
            MPI_Recv(list+n*RS,n*RS,MPI_BYTE,partner,l*dim+m,
                    MPI_COMM_WORLD,&status);
            mergesort(list,0,2*n-1,myid & bitl);
            if (myid & bitm) {
                memcpy(list, list+n*RS, n*RS);
            }
            bitm = bitm >> 1;
        }
    }
}

int main(int argc,char *argv[]) {
    if (argc != 2) {
        if (myid == 0) {
            fprintf(stderr, "Usage: %s <chunk size>\n", argv[0]);
        }
        exit(1);
    }

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    n = atoi(argv[1]);
    N = n*nprocs;
    char *list = malloc(4*RS*n); // for the chunk from the partner and temporary mergesort space
    if (list == 0) {
        fprintf(stderr, "Cannot allocate memory for buffer.");
        exit(1);
    }

    dim = (int)(log2(nprocs));

    {
        char fname[10];
        sprintf(fname, "in.%d", myid);
        FILE *file = fopen(fname, "rb");
        if (file == 0) {
            fprintf(stderr, "Cannot open file %s for reading.\n", fname);
            exit(1);
        }
        fseek(file, 0, SEEK_END);
        if (ftell(file) != RS*n) {
            fprintf(stderr, "File %s has incorrect length.\n", fname);
            exit(1);
        }
        fseek(file, 0, SEEK_SET);
        size_t res = fread(list, RS*n, 1, file);
        if (res != 1) {
            fprintf(stderr, "Failed to read from file %s.\n", fname);
            exit(1);
        }
        fclose(file);
    }

    parallel_mergesort(myid,list);

    {
        char fname[10];
        sprintf(fname, "out.%d", myid);
        FILE *file = fopen(fname, "wb");
        if (file == 0) {
            fprintf(stderr, "Cannot open file %s for writing.\n", fname);
            exit(1);
        }
        fwrite(list, RS*n, 1, file);
        fclose(file);
    }

    free(list);

    MPI_Finalize();
    return 0;
}
