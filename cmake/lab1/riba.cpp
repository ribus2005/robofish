#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#define SIZE 10000000
#ifdef float_array
    #define TYPE float
#endif
#ifdef double_array
    #define TYPE double
#endif
int main(){
    TYPE period = 2 * M_PI;
    std::vector<TYPE> mass;
    TYPE sum = 0;
    mass.reserve(SIZE);
    for(int i = 0; i < SIZE; i++){
        mass[i] = (TYPE)sin(period / SIZE * (i+1));
        sum += mass[i];
    }
    std::cout << "result sum: " << sum << "\n";
    return 0;
}
