#include "task2.h"


#define DEBUG 0
#define DEEP_DEBUG 0

/**
 * A global variable to define the number of processors.
 */ 
static unsigned int numberOfProcessors;

static long globalN;
static int globalI;
static int globalJ;

//static double** matrixC;

// TODO: Check if this works.
double inline randfrom(double min, double max) 
{
    double range = (max - min); 
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

/**
 * This method will run on every machine.
 */
void bspEntrance(){

    bsp_begin(numberOfProcessors);

    // Distribution between processors.
    long p= bsp_nprocs();
    long s= bsp_pid();
    long n = globalN;
    int get_i = globalI;
    int get_j = globalJ;
    bsp_push_reg(&n,sizeof(long));
    bsp_push_reg(&get_i,sizeof(int));
    bsp_push_reg(&get_j,sizeof(int));
    bsp_sync();

    bsp_get(0,&n,0,&n,sizeof(long));
    bsp_get(0,&get_i,0,&get_i,sizeof(int));
    bsp_get(0,&get_j,0,&get_j,sizeof(int));
    bsp_sync();
    bsp_pop_reg(&n);
    bsp_pop_reg(&get_i);
    bsp_pop_reg(&get_j);

    int start = n/numberOfProcessors * s;
    int end = n/p * (s+1);
    int k = start;
    int nrows = end - start;

    // Matrix init
    double* pointerA = (double*) malloc(sizeof(double)*n*nrows);
    double* pointerB = (double*) malloc(sizeof(double)*n*nrows);
    double* pointerC = (double*) malloc(sizeof(double)*n*nrows);
    double** matrixA = (double**) malloc(sizeof(double*) * nrows);
    double** matrixB = (double**) malloc(sizeof(double*) * nrows);
    double** localMatrixC = (double**) malloc(sizeof(double*) * nrows);
    double* i_row = (double*) malloc(sizeof(double)*n);
    double* j_colum = (double*) malloc(sizeof(double)*n);

    for(int i = 0; i < nrows; i++){
        matrixA[i] = pointerA + i*n;
        matrixB[i] = pointerB + i*n;
        localMatrixC[i] = pointerC + i*n;
    }

    for(int i = 0; i < nrows; i++){
        for(int y = 0; y < n; y++){
            matrixA[i][y] = randfrom(0,100);
            matrixB[i][y] = randfrom(0,100);
            localMatrixC[i][y] = 0;
        }
    }

    bsp_push_reg(pointerB,n*nrows*sizeof(double));
    bsp_sync();

    if(DEBUG) printf("...Matrix init done for s=%ld\n",s);

    double timeStart= bsp_time();
    do {
        for(int i = 0; i < nrows;i++){
            for(int h = k; h < k + n/p;h++){ // h or k
                for(int j = 0; j < n; j++){
                    if(DEEP_DEBUG){
                        printf("i=%d,j=%d,h=%d,k=%d for s=%ld\n",i,j,h,k,s);
                    }
                    localMatrixC[i][j] += matrixA[i][h] * matrixB[h-k][j];
                }
            }
        }
        k = (k + n / numberOfProcessors) % n;
        if(DEBUG) printf("Start distribution k=%d for s=%ld...\n",k,s);
        bsp_get((s+1)%p,pointerB,0,pointerB,n*nrows*sizeof(double));
        bsp_sync();
        if(DEBUG) printf("...distribution k=%d for s=%ld done...\n",k,s);
    } while(k != start);

    
    double timeEnd= bsp_time();
    if(DEBUG) printf("...calculations done for s=%ld\n",s);



    bsp_sync();
    if(s==0){
        printf("...calculations done in %.6lf seconds\n",timeEnd-timeStart);
    }

    


    // TODO: Make it possible to access the (i,j) cell and matching row
    // and colum.

    free(pointerA);
    free(pointerB);
    free(pointerC);
    free(matrixA);
    free(matrixB);
    free(localMatrixC);
    free(i_row);
    free(j_colum);
    bsp_end();
}

int main(int argc, char **argv){
    bsp_init(bspEntrance, argc, argv);
    numberOfProcessors = 1;
    globalN = 1800;
    globalI = 5;
    globalJ = 5;
    printf("Please enter number of processors:");
    scanf("%d", &numberOfProcessors);
    printf("\nPlease enter size of matrix:");
    scanf("%ld", &globalN);
    printf("\nPlease enter i:");
    scanf("%d", &globalI);
    printf("\nPlease enter j:");
    scanf("%d", &globalJ);
    printf("\n");

    if(numberOfProcessors > bsp_nprocs()){
        numberOfProcessors = bsp_nprocs();
    }
    printf("Start program with n=%ld,p=%d...\n",globalN,numberOfProcessors);
    bspEntrance();

    exit(EXIT_SUCCESS);
}