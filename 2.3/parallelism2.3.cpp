//метод простой итерации
#include <omp.h>
#include <iostream>
#include <vector>
#define SIZE 10000
#ifdef multiple
#define funcname solve1
#endif
#ifdef single
#define funcname solve2
#endif
using namespace std;
typedef vector<vector<double>> mat;
typedef vector<double> vec;

double default_time;
vector<double> solve(mat& A,vec& b,double t,double ksi) {
	vec x0(SIZE), x1(SIZE);
	double norm1, norm2;

	for (int i = 0; i < SIZE; i++)
		x0[i] = 0;

	// ||b|| -> norm2
	norm2 = 0;
	for (int i = 0; i < SIZE; i++)
		norm2 += b[i] * b[i];
	int cnt = 0;
	double start = omp_get_wtime();
	while (1) {
		cnt++;
		if (cnt % 1000 == 0)cout << cnt << "\n";
		// Axn -> x1
		for (int i = 0; i < SIZE;i++) {
			x1[i] = 0;
			for (int j = 0; j < SIZE;j++)
				x1[i] += x0[j] * A[i][j];
		}
		// x1 - b -> x1
		for (int i = 0; i < SIZE; i++)
			x1[i] -= b[i];
		// ||Axn - b|| = ||x1|| -> norm1
		norm1 = 0;
		for (int i = 0; i < SIZE; i++)
			norm1 += x1[i] * x1[i];
		// условие остановки
		if (norm1 / norm2 < ksi) {
			double time = omp_get_wtime() - start;
			cout << " finished in " << time << "\n";
			return x0;
		}
		// t * x1 -> x1
		for (int i = 0; i < SIZE; i++)
			x1[i] *= t;
		// x0 - x1 -> x0
		for (int i = 0; i < SIZE; i++)
			x0[i] -= x1[i];
	}
}

vector<double> solve1(mat& A, vec& b, double t, double ksi,int threads) {
	vec x0(SIZE), x1(SIZE);
	double norm1, norm2;

	for (int i = 0; i < SIZE; i++)
		x0[i] = 0;

	// ||b|| -> norm2
	norm2 = 0;
	for (int i = 0; i < SIZE; i++)
		norm2 += b[i] * b[i];
	int cnt = 0;
	double start = omp_get_wtime();
	while (1) {
		cnt++;
		if (cnt % 1000 == 0)cout << cnt << "\n";
		// Axn -> x1
		#pragma omp parallel for num_threads(threads)
		for (int i = 0; i < SIZE;i++) {
			x1[i] = 0;
			for (int j = 0; j < SIZE;j++)
				x1[i] += x0[j] * A[i][j];
		}
		// x1 - b -> x1
		#pragma omp parallel for num_threads(threads)
		for (int i = 0; i < SIZE; i++)
			x1[i] -= b[i];
		// ||Axn - b|| = ||x1|| -> norm1
		norm1 = 0;
		#pragma omp parallel for num_threads(threads)
		for (int i = 0; i < SIZE; i++)
			#pragma omp atomic
			norm1 += x1[i] * x1[i];
		// условие остановки
		if (norm1 / norm2 < ksi) {
			double time = omp_get_wtime() - start;
			if (threads == 1)default_time = time;
			cout << threads << " finished in " << default_time / time << "\n";
			return x0; 
		}
		// t * x1 -> x1
		#pragma omp parallel for num_threads(threads)
		for (int i = 0; i < SIZE; i++)
			x1[i] *= t;
		// x0 - x1 -> x0
		#pragma omp parallel for num_threads(threads)
		for (int i = 0; i < SIZE; i++)
			x0[i] -= x1[i];
	}
}

vector<double> solve2(mat& A, vec& b, double t, double ksi,int threads) {
	vec x0(SIZE), x1(SIZE);
	double norm1, norm2;

	for (int i = 0; i < SIZE; i++)
		x0[i] = 0;

	// ||b|| -> norm2
	norm2 = 0;
	for (int i = 0; i < SIZE; i++)
		norm2 += b[i] * b[i];
	int cnt = 0;
	bool stop = 0;
	double start = omp_get_wtime();
	#pragma omp parallel num_threads(threads)
	{
		while (!stop) {
			if (omp_get_thread_num() == 0) {
				cnt++;
				if (cnt % 1000 == 0)cout << cnt << "\n";
			}
			// Axn -> x1
			#pragma omp for
			for (int i = 0; i < SIZE;i++) {
				x1[i] = 0;
				for (int j = 0; j < SIZE;j++)
					x1[i] += x0[j] * A[i][j];
			}
			// x1 - b -> x1
			#pragma omp for
			for (int i = 0; i < SIZE; i++)
				x1[i] -= b[i];
			// ||Axn - b|| = ||x1|| -> norm1

			if(omp_get_thread_num() == 0)norm1 = 0;
			#pragma omp barrier
			#pragma omp for
			for (int i = 0; i < SIZE; i++)
				#pragma omp atomic
				norm1 += x1[i] * x1[i];
			// условие остановки
			if (norm1 / norm2 < ksi) { stop = 1; continue; }
			// t * x1 -> x1
			#pragma omp for
			for (int i = 0; i < SIZE; i++)
				x1[i] *= t;
			// x0 - x1 -> x0
			#pragma omp for
			for (int i = 0; i < SIZE; i++)
				x0[i] -= x1[i];
		}
	}
	double time = omp_get_wtime() - start;
	if (threads == 1)default_time = time;
	cout << threads << " finished in " << default_time/time << "\n";
	
	return x0;
}

int main() {
	vector<vector<double>> A(SIZE);
	vector<double> b(SIZE);
	vec u(SIZE);
	for (int i = 0; i < SIZE; i++)
		A[i].resize(SIZE);

	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			A[i][j] = 1;
			if (i == j) A[i][j] += 1;
		}
	}

	for (int i = 0; i < SIZE;i++)
		u[i] = 1;

	for (int i = 0; i < SIZE; i++) {
		b[i] = 0;
		for (int j = 0; j < SIZE; j++)
			b[i] += A[i][j] * u[j];
	}

	for (int i = 1; i <= 80; i++)
		vec sol = funcname(A, b, 0.0001, 0.00000001, i);
	return 0; 
}