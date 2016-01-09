#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

/*#define DEBUG*/
#define RS 100 // record size

int nprocs,dim,myid,n,N; /* Cube size, dimension, & my node ID */

/* Sequential mergesort (either ascending or descending) */
void mergesort(int *list,int left,int right,int descending) {
    int i,j,k,t,middle;
    int *temp = list+2*n;
    if (left < right) {
        middle = (left + right)/2;
        mergesort(list,left,middle,descending);
        mergesort(list,middle+1,right,descending);
        k = i = left; j = middle+1;
        if (descending)
            while (i<=middle && j<=right)
                temp[k++] = list[i]>list[j] ? list[i++] : list[j++];
        else
            while (i<=middle && j<=right)
                temp[k++] = list[i]<list[j] ? list[i++] : list[j++];
        t = i>middle ? j : i;
        while (k <= right) temp[k++] = list[t++];
        for (k=left; k<=right; k++) list[k] = temp[k];
    }
}

/* Parallel mergesort */
void parallel_mergesort(int myid,int *list) {
    int listsize, l, m, bitl = 1, bitm, partner, i;
    MPI_Status status;
    listsize = n;
    mergesort(list,0,listsize-1,myid & bitl);
    for (l=1; l<=dim; l++) {
        bitl = 1 << l;
        bitm = 1 << (l-1);
        for (m=l-1; m>=0; m--) {
            partner = myid ^ bitm;
            MPI_Send(list,listsize,MPI_INT,partner,l*dim+m,MPI_COMM_WORLD);
            MPI_Recv(list+listsize,listsize,MPI_INT,partner,l*dim+m,
                    MPI_COMM_WORLD,&status);
            mergesort(list,0,2*listsize-1,myid & bitl);
            if (myid & bitm)
                for (i=0; i<listsize; i++) list[i] = list[i+listsize];
            bitm = bitm >> 1;
        }
    }
}

int main(int argc,char *argv[]) {
    N = atoi(argv[1]);

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    if (argc != 2) {
        if (myid == 0) {
            fprintf(stderr, "Usage: %s <size>\n", argv[0]);
        }
        exit(1);
    }

    n = N/nprocs;
    int *list = malloc(4*n*sizeof(int)); // for the chunk from the partner and temporary mergesort space
    if (list == 0) {
        fprintf(stderr, "Cannot allocate memory for buffer.");
        exit(1);
    }

    dim = (int)(log2(nprocs));
    srand((unsigned) myid+1);
    for (int i=0; i<n; i++) list[i] = rand()%N;

#ifdef DEBUG
    char *tmpl = " %2d";

    if (myid == 0) {
        printf("Before:");
        for (int i=0; i<n; i++) printf(tmpl,list[i]);
        printf(" ");
        for (int id=1; id<nprocs; id++) {
            int recvlist[n];
            MPI_Status status;
            MPI_Recv(recvlist,n,MPI_INT,id,id,MPI_COMM_WORLD,&status);
            for (int i=0; i<n; i++) printf(tmpl, recvlist[i]);
            printf(" ");
        }
        printf("\n");
    } else {
        MPI_Send(list,n,MPI_INT,0,myid,MPI_COMM_WORLD);
    }
#endif

    parallel_mergesort(myid,list);

#ifdef DEBUG
    if (myid == 0) {
        printf("After: ");
        for (int i=0; i<n; i++) printf(tmpl,list[i]);
        printf(" ");
        for (int id=1; id<nprocs; id++) {
            int recvlist[n];
            MPI_Status status;
            MPI_Recv(recvlist,n,MPI_INT,id,id,MPI_COMM_WORLD,&status);
            for (int i=0; i<n; i++) printf(tmpl, recvlist[i]);
            printf(" ");
        }
        printf("\n");
    } else {
        MPI_Send(list,n,MPI_INT,0,myid,MPI_COMM_WORLD);
    }
#endif

    MPI_Finalize();
    return 0;
}
