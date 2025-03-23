#include <thread>
#include <vector>
#include <mutex>
#include <iostream>
#include <omp.h>

using namespace std;

typedef vector<vector<double>> matrix;
typedef vector<double> vec;

mutex my_mutex;


void compute(const matrix& A,const vec& b, vec& ans, const int start, const int amount) {
	vec result(amount);
	for (int i = start; i < start + amount; i++) {
		for (int j = 0; j < A[i].size(); j++) {
			result[i - start] += A[i][j] * b[j];
		}
	}
	my_mutex.lock();
	for (int i = start; i < start + amount; i++)
		ans[i] = result[i - start];
	my_mutex.unlock();
}


double MxV(const matrix& A, const vec& b, int threads) {
	vector<thread> thread_objects;
	vec result(A.size());
	int start, amount;
	amount = A.size() / threads;
	
	double starttime = omp_get_wtime();

	for (int i = 0; i < threads - 1; i++) {
		start = i * amount;
		thread_objects.push_back(thread(compute, ref(A), ref(b), ref(result), start, amount));
	}

	start = amount * (threads - 1);
	amount = A.size() - start;
	thread_objects.push_back(thread(compute, ref(A), ref(b), ref(result), start, amount));

	for (auto& thr : thread_objects)
		thr.join();

	return omp_get_wtime() - starttime;
}


int main() {
	int size = SIZE;
	matrix A(size);
	for (int i = 0; i < A.size(); i++)
		A[i].resize(A.size());
	vec b(size);

	for (int i = 0; i < A.size();i++) {
		for (int j = 0; j < A[i].size(); j++)
			A[i][j] = i + j;
	}
	for (int i = 0; i < b.size(); i++)
		b[i] = i;


	vector<int> threads{ 1,2,4,7,8,16,20,40 };

	double mytime, mintime, defaulttime;
	

	for (int th : threads) {
		mintime = 1000000;
		for (int i = 0; i < 10; i++) {
			mytime = MxV(A, b, th);
			if (mytime < mintime) mintime = mytime;
		}
		if (th == 1)defaulttime = mintime;
		cout << th << " " << defaulttime / mintime << " " << mytime << "\n";
	}

}