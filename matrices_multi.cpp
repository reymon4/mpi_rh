#include <iostream>

#include<mpi.h>
#include<fmt/core.h>
#include<memory>
#define MATRIX_DIM 25

//nprocs=4
//rows_per_rank=25/4=6
//
void printVector(double* vec, int size)
{
    fmt::print("[");
    for(int i=0; i<size; i++)
    {
        fmt::print("{}, ", vec[i]);
    }
    fmt::println("]");
}
int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int rank;
    int nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //id del proceso
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);//# de proceso

    int rows_per_rank; //rows per rank
    int rows_alloc= MATRIX_DIM; //TAMAÑO AJUSTADO DE LA MATRIZ PARA QUE EL NUM DE FILAS SEA DIVISIBLE PARA EL NPROS
    int padding=0; //padding for the last rank (Relleno)

    if(MATRIX_DIM % nprocs != 0) //El número de filas no es divisible para nproces. Entonces, necesitamos relleno en el último rank
    {
        rows_alloc = std::ceil((double)MATRIX_DIM / nprocs) * nprocs;
        padding = rows_alloc - MATRIX_DIM;
    }

    rows_per_rank = rows_alloc / nprocs;
    //Buffers -> smart pointers
    //b=A*x
    std::unique_ptr<double[]> A;
    std::unique_ptr<double[]> B;
    std::unique_ptr<double[]> x = std::make_unique<double[]>(MATRIX_DIM); //Reservamos memoria

    //bi = Ai * x, donde Ai es la matriz de dimension 4*25 -> rows_per_rank*25
    std::unique_ptr<double[]> A_local;
    std::unique_ptr<double[]> b_local;

    //Cada rank va a tener siete filas de la matriz A

    if(rank==0)
    {
        fmt::println("Dimension={}, rows_alloc={}, rows_per_rank={}, padding={}", MATRIX_DIM, rows_alloc, rows_per_rank, padding);
        //std::printf("Dimension=%d, rows_alloc=%d, rows_per_rank=%d, padding=%d\n", MATRIX_DIM, rows_alloc, rows_per_rank, padding);
        //Relleno por matriz
        A = std::make_unique<double[]>(rows_alloc*MATRIX_DIM);
        B = std::make_unique<double[]>(rows_alloc);
        for(int i=0; i<MATRIX_DIM; i++){
            fmt::print("fila={:>2} -> ",i);
            for(int j=0; j<MATRIX_DIM; j++){
               int index = i*MATRIX_DIM + j;
                A[index] = i;
                fmt::print("{:2} ",A[index]);
            }
            fmt::println("");
        }
        for(int i=0; i<MATRIX_DIM; i++){
            x[i] = i;
        }
    }
    A_local = std::make_unique<double[]>(rows_per_rank*MATRIX_DIM);
    b_local = std::make_unique<double[]>(rows_per_rank);

    printVector(x.get(), MATRIX_DIM);


    MPI_Bcast(x.get(), MATRIX_DIM, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}
