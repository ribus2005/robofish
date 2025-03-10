#include <omp.h>
#include <chrono>
#include <iostream>
#include <vector>
#ifdef BIG
#define msize 40000
#else
#define msize 20000
#endif
using namespace std;
int main()
{
	time_t start, end;
	
	vector<vector<double> > A(msize);
	for (int i = 0; i < msize; i++) {
		A[i].resize(msize);
		for (int j = 0; j < msize; j++) {
			A[i][j] = i+j;
		}
	}
	vector<double> b(msize);
	for (int i = 0; i < msize; i++)
		b[i] = 5 * i;
	vector<double> c(msize);
	int potoks[] = { 3 };

    
	for (int i = 0; i < 1; i++) {
		int thread_amount = potoks[i];
		//на старт, внимание, марш!
		cout << "code started\n";
		auto begin = chrono::steady_clock::now();
		#pragma omp parallel num_threads(thread_amount)
		{
			int threadid = omp_get_thread_num();
			int items_per_thread = msize / thread_amount;
			int left_bound = threadid * items_per_thread;
			int right_bound;
			if (threadid == thread_amount - 1) right_bound = msize - 1;
			else right_bound = left_bound + items_per_thread;
			//cout << threadid << ", " << right_bound - left_bound << "\n";
			for (int i = left_bound; i < right_bound; i++) {
				for (int j = 0; j < msize; j++)
					c[i] += A[i][j] * b[j];
			}
		}
		auto end = chrono::steady_clock::now();
		auto elapsed_ms = chrono::duration_cast<std::chrono::milliseconds>(end - begin);
		cout << potoks[i] << " threads " << elapsed_ms.count()/1000.0 << " seconds\n";
		#pragma omp barrier
		{
			cout << c.at(10000);
		}
	}
	
	return 0;
}