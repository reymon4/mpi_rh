#include <mpi.h>
#include <memory>
#include <iostream>
#include <cstdlib> // Para rand() y srand()
#include <ctime>   // Para time()
#include <fmt/core.h>

#define MATRIX_DIM 25

void imprimir_vector(const std::string msg, double* v, int size) {
    fmt::print("{} [",msg);
        for (int i=0; i<size;i++) { //imprime los numeros seguidos de una coma
            fmt::print("{},",v[i]);
        }

    fmt::println("]");
}

void imprimir_matriz(const std::string msg,double* A,int rows, int cols) {
    fmt::print("{} [",msg);
    for (int i=0;i<rows;i++) {
        fmt::print("{:>2}:  ",i);
        for (int j=0; j<cols;j++) {
            int index = i*cols+j;
            fmt::print("{:2},",A[index]);
        }
        fmt::println("");
    }
    fmt::println("]");
}

void multiplicar_matriz_vector(double* A, double* x, double* b, int rows, int cols) {
    for (int i=0; i<rows; i++) {
        b[i]=0;
        for (int j=0; j<cols; j++) {
            b[i]+=A[i*cols+j]*x[j];
        }
    }
}

int main(int argc, char* argv[]) {


    MPI_Init(&argc, &argv); // Inicializa MPI
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Obtener el rank del proceso
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs); // Obtener el número total de procesos


    int rows_per_rank; //Numero de filas a enviar por cada RANK
    int rows_alloc=MATRIX_DIM; //tamanio ajustado de la matriz, para que el # de filas sea divisipa para el NPROCS
    int padding=0; //numero de filas que se agregan para ajustar el tamanio de la matriz

    if(MATRIX_DIM%nprocs!=0) {
        //el numero de filas no es divisiple para NPROCS
        rows_alloc=std::ceil((double) MATRIX_DIM/nprocs)*nprocs; //funcion techo de la division y multiplicacion por el numero de procesos
        padding=rows_alloc-MATRIX_DIM; //numero de filas que se agregan para ajustar el tamanio de la matriz
    }

    rows_per_rank=rows_alloc/nprocs; //numero de filas a enviar por cada RANK

    //--buffers
    // b= A*x
    std::unique_ptr<double[]> A; //solo RANK_0
    //esto es un puntero a un array de doubles
    std::unique_ptr<double[]> b; //solo RANK_0
    std::unique_ptr<double[]> x=std::make_unique<double[]>(MATRIX_DIM);
        //permite reservar memoria

    //bi = Ai*x, donde Ai e suna matriz de dimencion 7x25 ==> rows_per_rank x MATRIX_DIM
    /*
    A_0 = [
        f0
        f1
        f2
        f3
        f4
        f5
        f6
    ]

    A_1 = [
        f7
        f8
        f9
        f10
        f11
        f12
        f13
    ]

    A_2 = [
        f14
        f15
        f16
        f17
        f18
        f19
        f20
    ]

    A_3 = [
        f21
        f22
        f23
        f24
        f25 --> padding
        f26 --> padding
        f27 --> padding
    ]

     */

    std::unique_ptr<double[]> A_local;
    std::unique_ptr<double[]> b_local;

    if(rank==0) {
        fmt::print("Dimension={}, rows_alloc={}, rows_per_rank={}, padding={}\n",
            MATRIX_DIM, rows_alloc, rows_per_rank, padding
            );

        A=std::make_unique<double[]>(rows_alloc*MATRIX_DIM); //defino la matriz A con dimencion 28*25
        //al manejar la matriz en una sola linea vectorial me facilita la distribucion en los ranks.
        b=std::make_unique<double[]>(rows_alloc); //defino b con 28 elemenetos

        //--incializar matriz A,, vector x
         //std::printf("         c1 c2 c3 ...\n");
         for(int i=0; i<MATRIX_DIM; i++) {
             //fmt::print("fila{:>3} |",i);
             for(int j=0; j<MATRIX_DIM; j++) {
                 int index = i*MATRIX_DIM + j;
                 A[index]=i;
                 //fmt::print( "{:>2} ", A[index]);
             }
             //std::printf("| \n");
         }// define la matriz A con valores de 0 a 24 en cada fila

        //imprimir_matriz(A,MATRIX_DIM,MATRIX_DIM);

        for (int i=0; i<MATRIX_DIM; i++) {
            x[i]=1;
        }// defino el vector x con valores de 1



        // Rellenar filas adicionales si hay padding /////#############################################
        /*for (int i = MATRIX_DIM; i < rows_alloc; i++) {
            for (int j = 0; j < MATRIX_DIM; j++) {
                A[i * MATRIX_DIM + j] = 0; // Llenar con ceros
            }
        }*/
        /////////////////////////////////////////////////////////////////////////////
    }

    //--incializar matrizes locales
    A_local=std::make_unique<double[]>(rows_per_rank*MATRIX_DIM); //defino la matriz A con dimencion 7*25
    // es decir en este caso ya tomo unicamente las filas de la matriz A que le corresponde a cada rank
    // En resumen: es la submatriz que maneja un rank x
    b_local=std::make_unique<double[]>(rows_per_rank); // como tengo 7 filas a multiplicar, por tanto tengo 7 resultados (rows_per_rank=7)
    // En resumen es el resultado parcial de la multiplicacion.

    //imprimir vector 'x'
    auto txt = fmt::format("RANK_{}",rank);
    imprimir_vector(txt,x.get(),MATRIX_DIM);

    MPI_Bcast(x.get(), MATRIX_DIM, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    //Distribuye el vector x desde el proceso rank 0 a todos los demás procesos. root=0 es decir el rank 0

    auto txt_after = fmt::format("RANK_{} after",rank);
    imprimir_vector(txt_after,x.get(),MATRIX_DIM);





    ////////////////////////////////////////////////////////

    /*// Dividir la matriz A entre los procesos
    if (rank == 0) {
        MPI_Scatter(A.get(), rows_per_rank * MATRIX_DIM, MPI_DOUBLE,
                    MPI_IN_PLACE, rows_per_rank * MATRIX_DIM, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);
    } else {
        MPI_Scatter(nullptr, rows_per_rank * MATRIX_DIM, MPI_DOUBLE,
                    A_local.get(), rows_per_rank * MATRIX_DIM, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);
    }

    // Multiplicación local: b_local = A_local * x
    for (int i = 0; i < rows_per_rank; i++) {
        b_local[i] = 0.0; // Inicializar a 0
        for (int j = 0; j < MATRIX_DIM; j++) {
            b_local[i] += A_local[i * MATRIX_DIM + j] * x[j];
        }
    }

    // Recolectar los resultados locales en el proceso raíz
    if (rank == 0) {
        MPI_Gather(MPI_IN_PLACE, rows_per_rank, MPI_DOUBLE,
                   b.get(), rows_per_rank, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);
    } else {
        MPI_Gather(b_local.get(), rows_per_rank, MPI_DOUBLE,
                   nullptr, rows_per_rank, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);
    }

    // Imprimir el vector resultado b en el proceso raíz
    if (rank == 0) {
        fmt::print("Vector resultado b:\n");
        imprimir_vector("", b.get(), MATRIX_DIM);
    }*/

    ///////////////////////


    //--enviar la matriz A: Scatter
    MPI_Scatter(
        A.get(),
        MATRIX_DIM*rows_per_rank,
        MPI_DOUBLE,
        A_local.get(),
        MATRIX_DIM*rows_per_rank,
        MPI_DOUBLE,
        0,
        MPI_COMM_WORLD);

    int rank_to_print=0;
    if (rank==rank_to_print) {
        auto txtMatriz = fmt::format("RANK_{} Mtz",rank);
        imprimir_matriz(txtMatriz,A_local.get(),rows_per_rank,MATRIX_DIM);
    }

    // -- calcular D_local = A_local * x
    int rows_per_rank_tmp=rows_per_rank;
    if(rank==nprocs-1) {
        rows_per_rank_tmp=rows_per_rank-padding;
    }

    multiplicar_matriz_vector(A_local.get(),x.get(),b_local.get(),rows_per_rank_tmp,MATRIX_DIM);

    if (rank==rank_to_print) {
        auto txtVector = fmt::format("RANK_{} b_local",rank);
        imprimir_vector(txtVector,b_local.get(),rows_per_rank);
    }

    //--enviar los resultados parciales al RANK_0: Gather
    MPI_Gather(
        b_local.get(), //enviar
        rows_per_rank,
        MPI_DOUBLE,
        b.get(), //recibir
        rows_per_rank,
        MPI_DOUBLE,
        0, //al RANK_0
        MPI_COMM_WORLD
    );

    if (rank==0) {
        auto txtResultado = fmt::format("RANK_{} rasultado b: ",rank);
        imprimir_vector(txtResultado,b.get(),MATRIX_DIM);
    }


    MPI_Finalize();
    return 0;

}