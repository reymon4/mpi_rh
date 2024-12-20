#include <iostream>

#include<mpi.h>

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int rank;
    int nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //id del proceso
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);//# de proceso

    //std::printf("Rank %d of %d procs\n", rank, nprocs);

    int valor = 0;
    int tmp = 0;
    if(rank == 0)
    {
        valor = 99;
        // MPI_Send(&valor, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        // MPI_Send(&valor, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        // MPI_Send(&valor, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);

        for(int rankId = 1; rankId<nprocs;rankId++)
        {
            MPI_Send(&valor, 1, MPI_INT, rankId, 0 , MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(&tmp, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //std::printf("Rank_%d, recibido=%d\n", rank, tmp);
    }

    std::printf("RANK_%d valor=%d recibido=%d\n", rank, valor,tmp);

    MPI_Finalize();
    return 0;
}
