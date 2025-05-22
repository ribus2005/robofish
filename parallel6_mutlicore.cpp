#include <boost/program_options.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <iostream>

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
    
    double *local_grid = (double*)calloc(ny * nx, sizeof(*local_grid));
    double *local_newgrid = (double*)calloc(ny * nx, sizeof(*local_newgrid));
    double dx = 1.0 / (nx - 1.0);
    char *filename = NULL;

    initgrid(local_grid, nx, ny);
    initgrid(local_newgrid, nx, ny);

    //#pragma acc update device(local_grid[0:ny*nx], local_newgrid[0:ny*nx])

    #pragma acc data copy(local_grid[0:ny*nx], local_newgrid[0:ny*nx])
    {
        double maxdiff;
        auto start = std::chrono::system_clock::now();
        int niters = 0;
        for (;;) {
            niters++;
            #pragma acc parallel loop collapse(2) present(local_grid, local_newgrid)
            for (int i = 1; i < ny - 1; i++) { // Update interior points
                for (int j = 1; j < nx - 1; j++) {
                    local_newgrid[IND(i, j)] =
                    (local_grid[IND(i - 1, j)] + local_grid[IND(i + 1, j)] +
                    local_grid[IND(i, j - 1)] + local_grid[IND(i, j + 1)]) * 0.25;
                }
            }
            // Check termination condition
            if(niters % 1000 == 0){
                maxdiff = 0;
                #pragma acc parallel loop collapse(2) reduction(max:maxdiff) present(local_grid, local_newgrid)
                for (int i = 1; i < ny - 1; i++) {
                    for (int j = 1; j < nx - 1; j++) {
                        int ind = IND(i, j);
                        maxdiff = fmax(maxdiff, fabs(local_grid[ind] - local_newgrid[ind]));
                    }   
                }
                // Swap grids (after termination local_grid will contain result)
                
                //std::cout << maxdiff << " >= " << EPS << "\r";
                //fflush(stdout);
                if (maxdiff < EPS || niters >= maxiter)
                    break;
            }   
            std::swap(local_grid, local_newgrid);


        }
        std::cout << "difference " << maxdiff << "\n";
        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "elapsed time " << elapsed.count() / 1000.0 << " seconds\n";
        std::cout << "made " << niters << " iterations\n";
        printf("# Heat 2D (parallel): grid: rows %d, cols %d\n", rows, cols);
    }
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