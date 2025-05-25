#include <boost/program_options.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <iostream>
#include <cublas_v2.h>
#include <openacc.h>
#include <cuda_runtime.h>
#include <cub/cub.cuh>

namespace opt = boost::program_options;

#define PI 3.14159265358979323846
#define IND(i, j) ((i) * nx + (j))
#define LIN(x, x0, x1, fx0, fx1) ( fx0 + (x - x0) * (fx1 - fx0) / (x1 - x0)  )

void initgrid(double* grid, int nx, int ny){
    grid[IND(0, 0)] = 10;
    grid[IND(0, nx-1)] = 20;
    grid[IND(ny-1, 0)] = 30;
    grid[IND(ny-1, nx-1)] = 20;

    // linear interpolation upper and lower side
    for(int x = 1; x < nx - 1; x++){
        grid[IND(0, x)] = LIN(x, 0, nx-1, grid[IND(0,0)], grid[IND(0, nx-1)]);
        grid[IND(ny-1, x)] = LIN(x, 0, nx-1, grid[IND(ny-1,0)], grid[IND(ny-1, nx-1)]);
    }
    // linear interpolation for left and right side
    for(int y = 1; y < ny - 1; y++){
        grid[IND(y, 0)] = LIN(y, 0, ny-1, grid[IND(0,0)], grid[IND(ny-1, 0)]);
        grid[IND(y, nx-1)] = LIN(y, 0, ny-1, grid[IND(0,nx-1)], grid[IND(ny-1, nx-1)]);
    }
}

__global__ void max_diff_kernel(const double* grid, const double* newgrid, double* diff, int n) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid < n) {
        diff[tid] = fabs(newgrid[tid] - grid[tid]);
    }
}

__global__ void heat_update_kernel(double* local_grid, double* local_newgrid, int nx, int ny) {
    int i = blockIdx.y * blockDim.y + threadIdx.y + 1;
    int j = blockIdx.x * blockDim.x + threadIdx.x + 1;
    
    if (i < ny-1 && j < nx-1) {
        local_newgrid[i*nx + j] = (local_grid[(i-1)*nx + j] + 
                                  local_grid[(i+1)*nx + j] +
                                  local_grid[i*nx + (j-1)] + 
                                  local_grid[i*nx + (j+1)]) * 0.25;
    }
}


int main(int argc, char *argv[])
{

    opt::options_description desc("All options");

    desc.add_options()
        ("width", opt::value<int>(), "grid width")
        ("height", opt::value<int>(), "grid height")
        ("eps", opt::value<double>(), "min dif")
        ("maxiter", opt::value<int>(), "max iterations")
        ("help", "width height eps maxiter")
    ;

    opt::variables_map vm;

    opt::store(opt::parse_command_line(argc, argv, desc), vm);

    opt::notify(vm);

    int rows = vm["height"].as<int>();
    int cols = vm["width"].as<int>();
    double EPS = vm["eps"].as<double>();
    int maxiter = vm["maxiter"].as<int>();
    int ny = rows;
    int nx = cols;
    
    double *grid = (double*)calloc(ny * nx, sizeof(double));
    double *newgrid = (double*)calloc(ny * nx, sizeof(double));
    double *max = (double*)calloc(ny * nx, sizeof(double));
    double *diff = (double*)calloc(ny * nx, sizeof(double));
    double dx = 1.0 / (nx - 1.0);
    char *filename = NULL;

    initgrid(grid, nx, ny);
    initgrid(newgrid, nx, ny);

    double *local_grid, *local_newgrid, *local_max, *local_diff;
    cudaMalloc(&local_grid, nx*ny*sizeof(double));
    cudaMalloc(&local_newgrid, nx*ny*sizeof(double));
    cudaMalloc(&local_max, nx*ny*sizeof(double));
    cudaMalloc(&local_diff, nx*ny*sizeof(double));

    cudaMemcpy(local_grid, grid, nx*ny * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(local_newgrid, newgrid, nx*ny * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(local_max, max, nx*ny * sizeof(double), cudaMemcpyHostToDevice);

    cublasHandle_t handle;
    cublasCreate(&handle);

    void* d_temp = nullptr;
    size_t temp_bytes = 0;
    cub::DeviceReduce::Max(d_temp, temp_bytes, local_diff, local_max, nx*ny);
    cudaMalloc(&d_temp, temp_bytes);

    cudaMemset(local_diff, 0, nx*ny*sizeof(double));

    dim3 blockSize(32, 32);  
    dim3 gridSize((nx + blockSize.x - 1) / (blockSize.x),
                  (ny + blockSize.y - 1) / (blockSize.y));
    cudaGraph_t graph;
    cudaGraphExec_t graphExec;
    cudaStream_t stream;

    cudaStreamCreate(&stream);
    cudaGraphCreate(&graph, 0);
    cudaStreamBeginCapture(stream, cudaStreamCaptureModeGlobal);

    for(int i = 0; i < 1000; i++){
        if(i % 2 == 0) heat_update_kernel<<<gridSize, blockSize, 0, stream>>>(local_grid, local_newgrid, nx, ny);
        else heat_update_kernel<<<gridSize, blockSize, 0, stream>>>(local_newgrid, local_grid, nx, ny);
    }
    
    cudaStreamEndCapture(stream, &graph);
    cudaGraphInstantiate(&graphExec, graph, NULL, NULL, 0);

        double maxdiff;
        auto start = std::chrono::system_clock::now();
        int niters = 0;
        for (;;) {
            niters += 1000;
            cudaGraphLaunch(graphExec, stream);
            cudaStreamSynchronize(stream);
            

            max_diff_kernel<<<(nx*ny + 255)/256, 256>>>(local_grid, local_newgrid, local_diff, nx*ny);

            cub::DeviceReduce::Max(d_temp, temp_bytes, local_diff, local_max, nx*ny);

            cudaMemcpy(&maxdiff, local_max, sizeof(double), cudaMemcpyDeviceToHost);

            if (maxdiff < EPS || niters >= maxiter)
                break;

        }
        

        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "difference " << maxdiff << "\n";
        
        std::cout << "elapsed time " << elapsed.count() / 1000.0 << " seconds\n";
        std::cout << "made " << niters << " iterations\n";
        printf("# Heat 2D (parallel): grid: rows %d, cols %d\n", rows, cols);
    //printf("# niters %d, total time %.6f\n", niters, ttotal);
    // Save grid
    if (filename) {
        FILE *fout = fopen(filename, "w");
        if (!fout) {
            perror("Can't open file");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < ny; i++) {
            for (int j = 0; j < nx; j++)
                fprintf(fout, "%.4f ", local_grid[IND(i, j)]);
            fprintf(fout, "\n");
        }
        fclose(fout);
    }
    return 0;
}